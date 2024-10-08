#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <memory>
#include <span>
#include <utility>

using i8  = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using idx         = std::ptrdiff_t;
using usize       = std::size_t;
using isize       = std::ptrdiff_t;
using byte        = std::byte;
using voidp       = void*;
using const_voidp = const void*;

template<typename T>
using rc = std::shared_ptr<T>;

template<typename T>
using weak = std::weak_ptr<T>;

template<typename T, typename D = std::default_delete<T>>
using up = std::unique_ptr<T, D>;

template<typename T, typename... Args>
auto make_up(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}
template<typename T, typename... Args>
auto make_rc(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

namespace std
{
struct monostate;
}

struct NoCopy {
protected:
    NoCopy()  = default;
    ~NoCopy() = default;

    NoCopy(const NoCopy&)            = delete;
    NoCopy& operator=(const NoCopy&) = delete;
};

namespace ycore
{
struct monostate {};

template<typename Wrapper>
static inline typename Wrapper::element_type* GetPtrHelper(const Wrapper& p) {
    return p.get();
}

template<class T>
inline void hash_combine(std::size_t& seed, const T& v) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<typename, template<typename...> class>
constexpr bool is_specialization_of = false;
template<template<typename...> class T, typename... Args>
constexpr bool is_specialization_of<T<Args...>, T> = true;

template<typename T, typename... Ts>
concept convertible_to_any = (std::convertible_to<T, Ts> || ...);

template<typename T, typename F>
concept extra_cvt = (! std::same_as<std::decay_t<T>, std::decay_t<F>>);

} // namespace ycore

template<typename Tout, typename Tin>
struct Convert;

template<typename Tout, typename Fin>
concept convertable = requires(Tout& t, Fin&& f) {
    { Convert<std::decay_t<Tout>, std::decay_t<Fin>>::from(t, std::forward<Fin>(f)) };
};

template<typename Tout, typename Tin>
    requires convertable<Tout, Tin>
void convert(Tout& out, Tin&& in) {
    Convert<std::decay_t<Tout>, std::decay_t<Tin>>::from(out, std::forward<Tin>(in));
}

template<typename Tout, typename Tin>
    requires convertable<Tout, Tin>
Tout convert_from(Tin&& in) {
    Tout out;
    Convert<std::decay_t<Tout>, std::decay_t<Tin>>::from(out, std::forward<Tin>(in));
    return out;
}

template<typename T>
struct Convert<T, T> {
    static void from(T& out, const T& in) { out = in; }
};

template<std::integral T, std::integral F>
    requires(! std::same_as<T, bool> && ycore::extra_cvt<T, F>) // && (sizeof(T) >= sizeof(F))
struct Convert<T, F> {
    static void from(T& out, F in) { out = (T)in; }
};

template<std::integral T>
    requires ycore::extra_cvt<bool, T>
struct Convert<bool, T> {
    static void from(bool& out, T i) { out = (bool)(i); }
};
template<typename T>
    requires ycore::extra_cvt<ycore::monostate, T>
struct Convert<ycore::monostate, T> {
    static void from(ycore::monostate&, const T&) {};
};

template<typename T>
    requires ycore::extra_cvt<std::monostate, T>
struct Convert<std::monostate, T> {
    static void from(std::monostate&, const T&) {};
};

#define DECLARE_CONVERT(Ta, Tb, ...)         \
    template<>                               \
    struct __VA_ARGS__ Convert<Ta, Tb> {     \
        using out_type = Ta;                 \
        using in_type  = Tb;                 \
        static void from(Ta&, const Tb& in); \
    };

#define IMPL_CONVERT(Ta, Tb) void Convert<Ta, Tb>::from(Ta& out, const Tb& in)
#define DEFINE_CONVERT(Ta, Tb) \
    DECLARE_CONVERT(Ta, Tb)    \
    inline IMPL_CONVERT(Ta, Tb)

#define STATIC_CAST_CONVERT(Ta, Tb)                             \
    DEFINE_CONVERT(Ta, Tb) { out = static_cast<out_type>(in); } \
    DEFINE_CONVERT(Tb, Ta) { out = static_cast<out_type>(in); }

template<typename IMPL>
struct CRTP {
protected:
    IMPL&       crtp_impl() { return *static_cast<IMPL*>(this); }
    const IMPL& crtp_impl() const { return *static_cast<const IMPL*>(this); }
};

#include "core/macro.h"