#pragma once
#include <exception>
#include <source_location>
#include "core/asio/export.h"

namespace helper
{
class  asio_detached_log_t {
public:
    asio_detached_log_t(const std::source_location = std::source_location::current());
    void operator()(std::exception_ptr);

    std::source_location loc;
};

} // namespace helper