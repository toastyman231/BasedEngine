#pragma once

namespace based
{
    // A resource can explicitly instantiate this to override the defaults
    template <typename T>
    struct ResourceTraits
    {
        static constexpr size_t kInitialSlots = 64;
        static_assert(kInitialSlots > 0, "You need to start with at least 1 slot!");
    };

    // Resources must declare that they are one
    // Resources cannot have public destructors since the ResourceManager needs to clean up their handles
    // Technically they're not supposed to have any public constructors either but that's harder to enforce,
    // so we just enforce that they aren't default constructible
    // Resources also need a UUID so we can track them easily
    template <typename T>
    concept Resource = requires(T t)
    {
        { T::IsResource } -> std::convertible_to<bool>;
        { t.GetUUID() } -> std::same_as<UUID>;
    } && T::IsResource && !std::is_constructible_v<T> && !std::is_destructible_v<T>;
}
