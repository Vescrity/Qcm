#pragma once

#include <QObject>
#include <QQmlParserStatus>
#include <QAbstractItemModel>

#include <asio/cancellation_signal.hpp>
#include <asio/bind_cancellation_slot.hpp>
#include <asio/use_awaitable.hpp>
#include <asio/recycling_allocator.hpp>
#include <asio/bind_allocator.hpp>

#include "core/qstr_helper.h"
#include "asio_helper/watch_dog.h"
#include "qcm_interface/enum.h"
#include "qcm_interface/macro.h"
#include "qcm_interface/export.h"
#include "asio_qt/qt_executor.h"

namespace qcm
{

namespace detail
{

template<typename M, typename A>
concept modelable =
    requires(M t, typename A::out_type out, typename A::in_type in) { t.handle_output(out, in); };
} // namespace detail

template<typename M, typename A>
concept modelable =
    detail::modelable<M, A> && (! std::derived_from<M, QAbstractItemModel> ||
                                requires(M t, qint32 offset) { t.fetchMoreReq(offset); });

class QCM_INTERFACE_API ApiQuerierBase : public QObject, public QQmlParserStatus {
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QString error READ error NOTIFY errorChanged FINAL)
    Q_PROPERTY(Status status READ status WRITE set_status NOTIFY statusChanged FINAL)
    Q_PROPERTY(bool autoReload READ autoReload WRITE set_autoReload NOTIFY autoReloadChanged FINAL)
    Q_PROPERTY(QObject* data READ data NOTIFY dataChanged FINAL)
    Q_PROPERTY(QObject* parent READ parent CONSTANT FINAL)
    Q_PROPERTY(
        bool forwardError READ forwardError WRITE set_forwardError NOTIFY forwardErrorChanged FINAL)
public:
    ApiQuerierBase(QObject* parent = nullptr);
    virtual ~ApiQuerierBase();
    using Status = enums::ApiStatus;

    Q_INVOKABLE void query();
    auto             status() const -> Status;

    auto error() const -> const QString&;

    bool autoReload() const;
    bool forwardError() const;

    bool dirty() const;
    bool is_qml_parsing() const;
    void classBegin() override;
    void componentComplete() override;

    virtual auto data() const -> QObject* = 0;
    virtual void reload()                 = 0;
    // virtual bool can_relaod() const = 0;

public Q_SLOTS:
    void         set_status(Status);
    void         set_autoReload(bool);
    void         set_forwardError(bool);
    void         reload_if_needed();
    void         mark_dirty(bool = true);
    virtual void fetch_more(qint32);

Q_SIGNALS:
    void forwardErrorChanged();
    void autoReloadChanged();
    void dataChanged();

    void statusChanged();
    void errorChanged();

protected:
    void set_error(QString);

    template<typename Ex, typename Fn>
    void spawn(Ex&& ex, Fn&& f) {
        QPointer<ApiQuerierBase> self { this };
        auto                     main_ex { get_executor() };
        auto                     alloc = asio::recycling_allocator<void>();
        asio::co_spawn(ex,
                       watch_dog().watch(ex, std::forward<Fn>(f), alloc),
                       asio::bind_allocator(alloc, [self, main_ex](std::exception_ptr p) {
                           if (! p) return;
                           try {
                               std::rethrow_exception(p);
                           } catch (const std::exception& e) {
                               std::string e_str = e.what();
                               asio::post(main_ex, [self, e_str]() {
                                   if (self) {
                                       self->set_error(convert_from<QString>(e_str));
                                       self->set_status(Status::Error);
                                   }
                               });
                               ERROR_LOG("{}", e_str);
                           }
                       }));
    }

    void cancel();
    auto get_executor() -> QtExecutor&;

private:
    auto watch_dog() -> helper::WatchDog&;

    class Private;
    C_DECLARE_PRIVATE(ApiQuerierBase, d_ptr);
};

} // namespace qcm