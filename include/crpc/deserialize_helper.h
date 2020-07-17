#pragma once

#include "cista/serialization.h"

namespace std {  // NOLINT

    template <typename Ctx, typename... T>
    void serialize(Ctx& c, std::tuple<T...> const* origin,
                   cista::offset_t const offset) {
        std::apply(
                [&](auto&&... args) {
                    (serialize(c, &args,
                               offset + (reinterpret_cast<intptr_t>(&args) -
                                         reinterpret_cast<intptr_t>(origin))),
                            ...);
                },
                *origin);
    }

    template <typename Ctx, typename... T>
    void deserialize(Ctx const& c, std::tuple<T...>* el) {
        std::apply([&](auto&&... args) { (deserialize(c, &args), ...); }, *el);
    }

}  // namespace std

struct Container {
    explicit Container(unsigned char *d, size_t si)
            : data{d}, size_{si} {}

    unsigned char &operator[](size_t pos)
    {
        return data[pos];
    }

    size_t size()
    {
        return this->size_;
    }

private:
    unsigned char* data;
    size_t size_;
};
