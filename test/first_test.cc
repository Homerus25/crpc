#include <array>
#include <functional>
#include <iostream>
#include <limits>
#include <sstream>
#include <tuple>

#include "doctest/doctest.h"

#include "cista/reflection/arity.h"
#include "cista/serialization.h"

namespace std {

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

template <typename ReturnType, typename... Args>
struct fn {};

template <typename Interface, typename ReturnType, typename... Args>
size_t index_of_member(fn<ReturnType, Args...> Interface::*const member_ptr) {
  auto i = 0U, field_index = std::numeric_limits<unsigned>::max();
  Interface interface;
  cista::for_each_field(interface, [&](auto&& m) {
    if constexpr (std::is_same_v<decltype(&m),
                                 decltype(&(interface.*member_ptr))>) {
      if (&m == &(interface.*member_ptr)) {
        field_index = i;
      }
    }
    ++i;
  });
  return field_index;
}

template <typename Interface>
struct server {
  std::vector<unsigned char> call(unsigned fn_idx,
                                  std::vector<unsigned char> const& params) {
    return fn_.at(fn_idx)(params);
  }

  template <typename Fn, typename ReturnType, typename... Args>
  void reg(fn<ReturnType, Args...> Interface::*const member_ptr, Fn&& f) {
    fn_[index_of_member(member_ptr)] =
        [mf = std::forward<Fn>(f)](std::vector<unsigned char> const& in)
        -> std::vector<unsigned char> {
      if constexpr (std::is_same_v<ReturnType, void>) {
        if constexpr (sizeof...(Args) == 0) {
          mf();
        } else {
          std::apply(mf, *cista::deserialize<std::tuple<Args...>>(in));
        }
        return {};
      } else {
        if constexpr (sizeof...(Args) == 0) {
          auto const return_value = mf();
          return cista::serialize(return_value);
        } else {
          auto const return_value =
              std::apply(mf, *cista::deserialize<std::tuple<Args...>>(in));
          return cista::serialize(return_value);
        }
      }
    };
  }

  std::array<std::function<std::vector<unsigned char>(
                 std::vector<unsigned char> const&)>,
             cista::arity<Interface>()>
      fn_;
};

template <typename Transport, typename Interface>
struct client : public Transport {
  template <typename... Args>
  client(Args&&... args) : Transport{std::forward<Args...>(args...)} {}

  template <typename ReturnType, typename... Args>
  ReturnType call(fn<ReturnType, Args...> Interface::*const member_ptr,
                  Args&&... args) {
    std::vector<unsigned char> response;
    if constexpr (sizeof...(Args) == 0U) {
      response = Transport::send(index_of_member(member_ptr), {});
    } else {
      auto const params = std::make_tuple(std::forward<Args>(args)...);
      response = Transport::send(index_of_member(member_ptr),
                                 cista::serialize(params));
    }
    if constexpr (!std::is_same_v<ReturnType, void>) {
      return *cista::deserialize<ReturnType>(response);
    }
  }
};

template <typename Interface>
struct stub_transport {
  stub_transport(server<Interface>& s) : s_{s} {}

  std::vector<unsigned char> send(unsigned fn_idx,
                                  std::vector<unsigned char> const& params) {
    return s_.call(fn_idx, params);
  }

  server<Interface>& s_;
};

template <typename Interface>
stub_transport(server<Interface>& s) -> stub_transport<Interface>;

template <typename Interface>
using stub_client = client<stub_transport<Interface>, Interface>;

TEST_CASE("first_test") {
  struct interface {
    fn<int, int, int> add_;
    fn<void> hello_world_;
    fn<void, int> inc_count_;
    fn<int> get_count_;
  };

  int count = 0;
  auto s = server<interface>{};
  std::stringstream out;
  s.reg(&interface::add_, [](int a, int b) { return a + b; });
  s.reg(&interface::hello_world_, [&]() { out << "hello world"; });
  s.reg(&interface::inc_count_, [&](int i) { return count += i; });
  s.reg(&interface::get_count_, [&]() { return count; });

  auto c = stub_client<interface>{s};
  CHECK(c.call(&interface::add_, 1, 2) == 3);
  CHECK_NOTHROW(c.call(&interface::hello_world_));
  CHECK(out.str() == "hello world");
  CHECK_NOTHROW(c.call(&interface::inc_count_, 5));
  CHECK(c.call(&interface::get_count_) == 5);
}