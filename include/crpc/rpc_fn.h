#pragma once

#include <cista/reflection/for_each_field.h>

template <typename ReturnType, typename... Args>
struct fn {};

template <typename Interface, typename ReturnType, typename... Args>
size_t index_of_member(fn<ReturnType, Args...> Interface::*const member_ptr) {
    auto i = 0U, field_index = std::numeric_limits<unsigned>::max();
    Interface interface{};
    cista::for_each_field(interface, [&](auto&& m) {
        if constexpr (
                std::is_same_v<
                        decltype(&m),
                        decltype(
                        &(interface.*
                          member_ptr))>) {  // NOLINT(bugprone-suspicious-semicolon)
            if (&m == &(interface.*member_ptr)) {
                field_index = i;
            }
        }
        ++i;
    });
    return field_index;
}
