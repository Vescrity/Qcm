#pragma once

#include <asio/recycling_allocator.hpp>
#include <asio/bind_executor.hpp>
#include <asio/strand.hpp>

#include "Qcm/util/async.hpp"
#include "core/asio/watch_dog.h"
#include "core/asio/error.h"
#include "core/qasio/qt_watcher.h"

namespace qcm
{
template<typename Fn>
void QAsyncResult::spawn(Fn&& f, const std::source_location loc) {
    helper::QWatcher<QAsyncResult> self { this };
    auto                           main_ex { get_executor() };
    auto                           ex    = asio::make_strand(pool_executor());
    auto                           alloc = asio::recycling_allocator<void>();
    if (use_queue()) {
        push(f, loc);
    } else {
        asio::co_spawn(ex,
                       watch_dog().watch(ex, std::forward<Fn>(f), asio::chrono::minutes(3), alloc),
                       asio::bind_allocator(alloc, [self, main_ex, loc](std::exception_ptr p) {
                           helper::handle_asio_exception(
                               p,
                               [main_ex, self](std::string_view error) {
                                   auto e_str = std::string(error);
                                   asio::post(main_ex, [self, e_str]() {
                                       if (self) {
                                           self->setError(QString::fromStdString(e_str));
                                           self->setStatus(Status::Error);
                                       }
                                   });
                               },
                               loc);
                       }));
    }
}
} // namespace qcm