#include <intercept.hpp>
#include <sol.hpp>
#include "LuaManager.hpp"


int intercept::api_version() { //This is required for the plugin to work.
    return 1;
}

void intercept::pre_start() {
    lua.preStart();
}
