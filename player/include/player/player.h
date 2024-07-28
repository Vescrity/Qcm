#pragma once

#include <asio/experimental/concurrent_channel.hpp>
#include <asio/thread_pool.hpp>
#include <asio/strand.hpp>

#include "core/core.h"
#include "player/notify.h"

namespace player
{

class Player {
public:
    using executor_type = asio::thread_pool::executor_type;

    class Private;
    Player(std::string_view name, Notifier, executor_type exc);
    ~Player();

    void play();
    void pause();
    void stop();
    void seek(i32);

    void set_source(std::string_view);

    auto volume() const -> float;
    void set_volume(float);

    auto fade_time() const -> u32;
    void set_fade_time(u32);

private:
    C_DECLARE_PRIVATE(Player, m_d)
};

} // namespace player
