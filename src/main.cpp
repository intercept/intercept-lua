#include <intercept.hpp>
#include <sol.hpp>


int intercept::api_version() { //This is required for the plugin to work.
    return 1;
}

void intercept::pre_start() {
    
}

void intercept::pre_init() {
    intercept::sqf::system_chat("The Intercept template plugin is running!");
}