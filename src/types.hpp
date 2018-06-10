#pragma once
#include <intercept.hpp>
#include <sol.hpp>
using namespace intercept;
using namespace intercept::types;


template <>
struct sol::lua_size<r_string> : std::integral_constant<int, 1> {};
template <>
struct sol::lua_type_of<r_string> : std::integral_constant<sol::type, sol::type::string> {};


template <>
struct sol::lua_size<vector3> : std::integral_constant<int, 3> {};
template <>
struct sol::lua_type_of<vector3> : std::integral_constant<sol::type, sol::type::table> {};

namespace sol::stack {

    template <>
    struct checker<r_string> {
        template <typename Handler>
        static bool check(lua_State* L, int index, Handler&& handler, record &tracking)
        {
            int absolute_index = lua_absindex(L, index);
            bool success = stack::check<const char*>(L, absolute_index, handler);
            tracking.use(1);

            return success;
        }
    };


    template <>
    struct getter<r_string> {
        static r_string get(lua_State* L, int index, record &tracking) {
            tracking.use(1);
            std::size_t len;
            auto str = lua_tolstring(L, index, &len);
            return r_string(std::string_view(str, len));
        }
    };

    template<>
    struct pusher<r_string> {
        static int push(lua_State* L, const r_string &str) {
            lua_pushlstring(L, str.data(), str.size());
            return 1;
        }

        static int push(lua_State* L, const r_string &str, std::size_t sz) {
            lua_pushlstring(L, str.data(), sz);
            return 1;
        }
    };

    template <>
    struct userdata_checker<r_string> {
        template <typename Handler>
        static bool check_usertype(lua_State* L, int index, Handler&& handler)
        {
            int absolute_index = lua_absindex(L, index);
            bool success = stack::check<const char*>(L, absolute_index, handler);

            return success;
        }
    };

    template <>
    struct check_getter<r_string> {
        template <typename Handler>
        static std::optional<r_string> get(lua_State* L, int index, Handler&& handler, record& tracking)
        {
            tracking.use(1);
            std::size_t len;
            auto str = lua_tolstring(L, index, &len);
            return r_string(std::string_view(str, len));
        }
    };

} // namespace sol::stack

struct SQF_glob {

};



extern void initTypes(sol::state& state);