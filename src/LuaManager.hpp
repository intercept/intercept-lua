#pragma once


#include <containers.hpp>
#include "sol/state.hpp"

class LuaManager {
public:
    LuaManager();
    ~LuaManager();
    void preStart();

    sol::state state;
};
extern LuaManager lua;

struct lua_iface {
    intercept::types::r_string(*blubTest)();
};