#include "types.hpp"
#include "LuaManager.hpp"


void initTypes(sol::state& state) {
    

    //https://github.com/actboy168/vscode-lua-debug
    //https://github.com/ThePhD/sol2/issues/327
    state.new_usertype<r_string>("string",
        sol::constructors<sol::types<>, sol::types<const char*>, sol::types<std::string>, sol::types<std::string_view>>(),
        "size", sol::readonly_property(&r_string::size),
        sol::meta_function::to_string, [](const r_string &s) { return s.data(); },
        sol::meta_function::index, [](const r_string &s, size_t i) { return s.data()[i - 1]; },//#TODO add at operator
        "begin", [](const r_string &s) { return s.begin(); },
        "append", sol::overload(
            [](r_string &self, const r_string &other) { self += other; },
            [](r_string &self, const std::string &other) { self += other; },
            [](r_string &self, const char *suffix) { self += suffix; }
        )
        );


    state.new_usertype<object>("object",
        //sol::constructors<sol::types<>, sol::types<const char*>, sol::types<std::string>, sol::types<std::string_view>>(),
        //"size", &r_string::size,
        sol::meta_function::to_string, [](const object &s) { return static_cast<r_string>(s); },
        //sol::meta_function::index, [](const r_string &s, size_t i) { return s.data()[i - 1]; },//#TODO add at operator
        "getPosASL", [](const object &s) { return sqf::get_pos_asl(s); },
        "positionASL", sol::property(
            [](const object &s) { return sqf::get_pos_asl(s); },
            [](const object &s, vector3 pos) { return sqf::set_pos_asl(s, pos); }),
        "setDir", [](const object &s, double dir) { return sqf::set_dir(s, dir); },
        "name", [](const object &s) { return sqf::name(s); },
        "getVariable", [](const object &s, const r_string& other) { return sqf::get_variable(s, other); }
    );

    state.new_usertype<rv_namespace>("namespace",
        "name", [](const rv_namespace &s) { return sqf::str(s); },
        sol::meta_function::index, [](const rv_namespace& e, std::string key) {
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            auto var = e.get_as<game_data_namespace>()->_variables.get(key);
            if (map_string_to_class<game_variable, auto_array<game_variable>>::is_null(var))
                return sol::make_object(lua.state, sol::nil);

            if (var.value.type_enum() == game_data_type::CODE)
                return sol::make_object(lua.state, static_cast<code>(var.value));

            return sol::make_object(lua.state, var.value);
        },
        sol::meta_function::new_index, [](const rv_namespace& e, std::string_view key, game_value value) {
            e.get_as<game_data_namespace>()->_variables.insert({ key , value });
        },
        "getVariable", [](const rv_namespace &s, std::string other) {
            std::transform(other.begin(), other.end(), other.begin(), ::tolower);
            auto var = s.get_as<game_data_namespace>()->_variables.get(other);
            if (map_string_to_class<game_variable, auto_array<game_variable>>::is_null(var))
                return sol::make_object(lua.state, sol::nil);

            if (var.value.type_enum() == game_data_type::CODE)
                return sol::make_object(lua.state, static_cast<code>(var.value));

            return sol::make_object(lua.state, var.value);
        }
    );

    //Generic game_value type. Could be complex. This type should be returned as little as possible
    state.new_usertype<game_value>("game_value",
        sol::meta_function::to_string, [](const game_value &s) { return static_cast<r_string>(s); },
        "type", [](const game_value &s) {
            return s.type_enum();
        },
        sol::meta_function::call, [](const game_value &s) {
            if (s.is_nil()) return sol::make_object(lua.state, sol::nil);
            return sol::make_object(lua.state, sqf::call(s));
        },
        sol::meta_function::call, [](const game_value &s, sol::variadic_args va) {
            if (s.is_nil()) return sol::make_object(lua.state, sol::nil);
            auto_array<game_value> args;
            for (auto& it : va)
                args.emplace_back(it);
            return sol::make_object(lua.state, sqf::call(s, args));
        }

    );

    state.new_enum("VariableType"sv,
        "SCALAR", types::game_data_type::SCALAR,
        "BOOL", types::game_data_type::BOOL,
        "ARRAY", types::game_data_type::ARRAY,
        "STRING", types::game_data_type::STRING,
        "NOTHING", types::game_data_type::NOTHING,
        "ANY", types::game_data_type::ANY,
        "NAMESPACE", types::game_data_type::NAMESPACE,
        "NaN", types::game_data_type::NaN,
        "CODE", types::game_data_type::CODE,
        "OBJECT", types::game_data_type::OBJECT,
        "SIDE", types::game_data_type::SIDE,
        "GROUP", types::game_data_type::GROUP,
        "TEXT", types::game_data_type::TEXT,
        "SCRIPT", types::game_data_type::SCRIPT,
        "TARGET", types::game_data_type::TARGET,
        "CONFIG", types::game_data_type::CONFIG,
        "DISPLAY", types::game_data_type::DISPLAY,
        "CONTROL", types::game_data_type::CONTROL,
        "NetObject", types::game_data_type::NetObject,
        "SUBGROUP", types::game_data_type::SUBGROUP,
        "TEAM_MEMBER", types::game_data_type::TEAM_MEMBER,
        "TASK", types::game_data_type::TASK,
        "DIARY_RECORD", types::game_data_type::DIARY_RECORD,
        "LOCATION", types::game_data_type::LOCATION
    );


    state.new_usertype<code>("code",
        sol::meta_function::call, [](const code &s) { return sqf::call(s); },
        sol::meta_function::call, [](const code &s, sol::variadic_args va) {
        auto_array<game_value> args;

        for (auto it : va) {
            switch (it.get_type()) {
            case sol::type::none:
            case sol::type::nil:
                args.emplace_back(game_value());
                break;
            case sol::type::string:
                args.emplace_back(it.get<r_string>()); break;
            case sol::type::number:
                args.emplace_back(it.get<float>()); break;
            case sol::type::thread: __debugbreak(); break;
            case sol::type::boolean:
                args.emplace_back(it.get<bool>());
            case sol::type::function: {
                std::string str = it.get<std::string>();

                //__debugbreak();
                break;

            }

            case sol::type::userdata:
                args.emplace_back(it);
                break;
            case sol::type::lightuserdata: __debugbreak(); break;
            case sol::type::table: __debugbreak(); break;
            case sol::type::poly: __debugbreak(); break;
            default: break;


            }
        }

        auto retVal = args.empty() ? sqf::call(s) : sqf::call(s, args);
        if (retVal.type_enum() == game_data_type::OBJECT)
            return sol::make_object(lua.state, (object)retVal);

        return  sol::make_object(lua.state, retVal);
    }
    );

    state.new_usertype<vector3>("vector3"

        //#TODO fill?
        );

    state.new_usertype<sqf::config_entry>("config",
		sol::constructors<>(),
        sol::meta_function::index, [](const sqf::config_entry& e, std::string_view key) {
            return e >> key;
        },
        sol::meta_function::to_string, [](const sqf::config_entry& e) {
            return sqf::str(static_cast<game_value>(e));
        },
        sol::meta_function::length, [](const sqf::config_entry& e) {
            return e.count();
        },
        sol::meta_function::bitwise_right_shift, [](const sqf::config_entry& e, std::string_view key) {
            return e >> key;
        },
        "asText", [](const sqf::config_entry& e) {
            return sqf::get_text(e);
        },
        "asNumber", [](const sqf::config_entry& e) {
            return sqf::get_number(e);
        },
        "name", sol::readonly_property([](const sqf::config_entry& e) {
            return sqf::config_name(e);
        })
    );



    //auto events = sol::table(state);
    //
    //events.create("PFH");
    //events.create("preInit");

    //https://github.com/ThePhD/sol2/issues/332
    //https://github.com/ThePhD/sol2/issues/90
    auto SQF_global = state.new_usertype<SQF_glob>("SQF",
        "missionNamespace", sol::var(sqf::mission_namespace()),
        //"events", sol::var(events),
        "player", sol::readonly_property([]() {
            return sqf::player();
        }),
        "systemChat", [](r_string message) {
            sqf::system_chat(message);
        },
        "createVehicle", [](r_string classname, vector3 pos) {
            return sqf::create_vehicle(classname, pos);
        },
        "configFile", sol::readonly_property([]() {
            return sqf::config_entry(sqf::config_file());
        })
    );



    state.new_usertype<vector3>("vector3",
        sol::meta_function::construct, sol::initializers(
            [](vector3* vec, const sol::table &other) {
                new (vec) vector3(other[0].get<double>(), other[1].get<double>(), other[2].get<double>());
            }
        )
    );




}
