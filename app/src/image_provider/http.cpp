#include "Qcm/image_provider/http.hpp"

#include <filesystem>
#include <cstdio>
#include <fstream>

#include <asio/bind_allocator.hpp>
#include <asio/recycling_allocator.hpp>
#include <asio/detached.hpp>

#include <ctre.hpp>
#include <QtCore/QPointer>

#include "Qcm/util/path.hpp"
#include "Qcm/global.hpp"

#include "core/asio/sync_file.h"
#include "crypto/crypto.h"
#include "Qcm/app.hpp"
#include "Qcm/backend.hpp"

import platform;

using namespace qcm;

namespace
{

struct ImageParam {
    QString item_type;
    QString item_id;
    QString image_type;
};

auto parse_image_url(QUrl url) -> ImageParam {
    constexpr auto ImageProviderRe = ctll::fixed_string { "image://qcm/([^/]+?)/([^/]+?)/(.*)" };

    auto input = url.toString(QUrl::FullyEncoded).toStdString();
    if (auto match = ctre::match<ImageProviderRe>(input)) {
        return { rstd::into(match.get<1>().to_string()),
                 rstd::into(match.get<2>().to_string()),
                 rstd::into(match.get<3>().to_string()) };
    } else {
        return {};
    }
}

} // namespace

namespace qcm
{

QcmAsyncImageResponse::QcmAsyncImageResponse() {}
QcmAsyncImageResponse::~QcmAsyncImageResponse() { plt::malloc_trim_count(0, 10); }

QQuickTextureFactory* QcmAsyncImageResponse::textureFactory() const {
    return QQuickTextureFactory::textureFactoryForImage(image);
}

class QcmImageProviderInner : std::enable_shared_from_this<QcmImageProviderInner>, NoCopy {
public:
    using executor_type = asio::thread_pool::executor_type;

    executor_type& get_executor() { return m_ex; }

    QcmImageProviderInner()
        : m_thread(make_box<QThread>(new QThread())),
          m_context(make_box<QtExecutionContext>(m_thread.get(),
                                                 (QEvent::Type)QEvent::registerEventType())),
          m_ex(Global::instance()->pool_executor()),
          m_session(Global::instance()->session()) {
        m_thread->start();
    }
    ~QcmImageProviderInner() {
        m_thread->quit();
        m_thread->wait();
    }

    task<ncrequest::HttpHeader> dl_image(const ncrequest::Request& req, std::filesystem::path p) {
        helper::SyncFile file { std::fstream(p, std::ios::out | std::ios::binary) };
        file.handle().exceptions(std::ios_base::failbit | std::ios_base::badbit);

        auto rsp_http = (co_await m_session->get(req)).unwrap();

        co_await rsp_http->read_to_stream(file);

        file.handle().close();
        co_return rsp_http->header().clone();
    }

    task<QImage> request_image(const ncrequest::Request& req, QSize req_size) {
        auto rsp_http = (co_await m_session->get(req)).unwrap();
        auto bytes    = co_await rsp_http->bytes();

        co_await asio::post(asio::bind_executor(m_context->get_executor(), asio::use_awaitable));

        QImage img;
        if (bytes) {
            img.loadFromData((uchar*)bytes->data(), (int)bytes->size());
            if (req_size.isValid() && ! img.isNull()) {
                img = img.scaled(req_size,
                                 Qt::AspectRatioMode::KeepAspectRatioByExpanding,
                                 Qt::TransformationMode::SmoothTransformation);
            }
        }
        co_return img;
    }

    task<void> handle_request(rc<QcmAsyncImageResponse> rsp, const ncrequest::Request& req,
                              QSize req_size) {
        auto img   = co_await request_image(req, req_size);
        rsp->image = img;
        co_return;
    }

private:
    Box<QThread>            m_thread;
    Box<QtExecutionContext> m_context;
    executor_type           m_ex;
    rc<ncrequest::Session>  m_session;
};
} // namespace qcm

QcmImageProvider::QcmImageProvider()
    : QQuickAsyncImageProvider(), m_inner(std::make_shared<QcmImageProviderInner>()) {}
QcmImageProvider::~QcmImageProvider() {}

QQuickImageResponse* QcmImageProvider::requestImageResponse(const QString& id,
                                                            const QSize&   requestedSize) {
    auto rsp = QcmAsyncImageResponse::make_rc<QcmAsyncImageResponse>();

    do {
        if (id.isEmpty()) break;

        auto req = rstd::None<ncrequest::Request>();

        if (id.startsWith("http")) {
            req = rstd::Some(ncrequest::Request { id.toStdString() });
        } else {
            ImageParam p = parse_image_url(rstd::into(std::format("image://qcm/{}", id)));
            auto       b = App::instance()->backend();
            req          = rstd::Some(b->image(p.item_type, p.item_id, p.image_type));
        }

        auto alloc = asio::recycling_allocator<void>();
        auto ex    = asio::make_strand(m_inner->get_executor());
        rsp->wdog().spawn(
            ex,
            [rsp, requestedSize, req = std::move(req), inner = m_inner]() -> task<void> {
                co_await inner->handle_request(rsp, *req, requestedSize);
                co_return;
            },
            asio::bind_allocator(alloc,
                                 [rsp, id](std::exception_ptr p) {
                                     if (p) {
                                         try {
                                             std::rethrow_exception(p);
                                         } catch (const std::exception& e) {
                                             rsp->setError(std::format(R"(
QcmImageProvider
    id: {}
    error: {})",
                                                                        id,
                                                                        e.what()));
                                         }
                                     }
                                 }),
            asio::chrono::minutes(2),
            alloc);
        return rsp.get();
    } while (false);

    return rsp.get();
}

#include <Qcm/image_provider/moc_http.cpp>
