#include "LuaManager.hpp"
#include <intercept.hpp>
#include <sstream>
using namespace intercept;

types::registered_sqf_function _execLua;
types::registered_sqf_function _compileLua;
types::registered_sqf_function _callLuaString;
types::registered_sqf_function _callLuaCodeArgs;
types::registered_sqf_function _callLuaCode;

//LuaEdit debugger
struct lua_State;


// LuaEdit API
void StartLuaEditRemoteDebugger(int aPort = 32201, lua_State* L = NULL);
void StopLuaEditRemoteDebugger();
const char* GetCurrentLuaVersion();
bool CheckLuaScriptSyntax(const char* aScript, const char* aScriptName, char* aErrBuf, int aErrBufLen);

static sqf_script_type GameDataLuaCode_type;
LuaManager lua;


class GameDataLuaCode : public game_data {

public:
    explicit GameDataLuaCode(sol::protected_function f) : code(f) {}
    GameDataLuaCode(sol::protected_function f, r_string codeString) : code(f), code_string(codeString) {}
    GameDataLuaCode() {}
    void lastRefDeleted() const override { delete this; }
    const sqf_script_type& type() const override { return GameDataLuaCode_type; }
    ~GameDataLuaCode() override {};

    bool get_as_bool() const override { return true; }
    float get_as_number() const override { return 0.f; }
    const r_string& get_as_string() const override { return code_string; }
    game_data* copy() const override { return new GameDataLuaCode(*this); }
    r_string to_string() const override { return code_string; }
    //virtual bool equals(const game_data*) const override;
    const char* type_as_string() const override { return "luaCode"; }
    bool is_nil() const override { return false; }
    bool can_serialize() override { return true; }

    serialization_return serialize(param_archive& ar) override {
        game_data::serialize(ar);
        ar.serialize("value"sv, code_string, 1);
        if (!ar._isExporting) {
            std::string c(code_string);
            code = lua.state.load(c).get<sol::protected_function>();
        }
        return serialization_return::no_error;
    }
    sol::protected_function code;
    r_string code_string;
};

game_data* createGameDataLuaCode(param_archive* ar) {
    //#TODO use armaAlloc
    auto x = new GameDataLuaCode();
    if (ar)
        x->serialize(*ar);
    return x;
}



struct RV_BIND_Object {
    RV_BIND_Object(object o) : obj(o) {}
    object obj;
    auto getName() const {
        return sqf::name(obj);
    }
};

sol::object system_chat(sol::object a, sol::this_state s) {
    sol::state_view lua(s);
    if (a.is<std::string>()) {
        sqf::system_chat(a.as<std::string>());
    }
    return sol::make_object(lua, sol::nil);
}
sol::object player(sol::this_state s) {
    sol::state_view lua(s);
    return sol::make_object(lua, RV_BIND_Object(sqf::player()));
}
std::vector<sol::object> allPlayers(sol::this_state s) {
    sol::state_view lua(s);
    std::vector<sol::object> ret;
    for (auto& it : sqf::all_players()) {
        ret.emplace_back(sol::make_object(lua, it));
    }
    return ret;
}

sol::object getVariable(sol::object a, sol::this_state s) {
    sol::state_view lua(s);
    std::vector<sol::object> ret;
    if (a.is<std::string>()) {
        //#TODO return sol::nil if var is nil
        return sol::make_object(lua, sqf::get_variable(sqf::mission_namespace(), a.as<std::string>()));
    } else if (a.is<r_string>()) {
        return sol::make_object(lua, sqf::get_variable(sqf::mission_namespace(), a.as<r_string>()));
    }
    return  sol::make_object(lua, sol::nil);
}

game_value executeLua(game_value_parameter leftArg, game_value_parameter rightArg) {
    //auto source = sqf::load_file(rightArg);

    lua.state["_this"] = sol::make_object(lua.state, leftArg);
    try {
        //auto result = lua.state.do_file(rightArg);
        //std::string ret = result;
        //return ret;
    }
    catch (sol::error err) {
        return err.what();
    }
}

game_value compileLua(game_value_parameter rightArg) {
    auto result = lua.state.load(static_cast<r_string>(rightArg));
    if (result.valid()) {
        return game_value(new GameDataLuaCode(result.get<sol::protected_function>(), rightArg));
    }
    return game_value();
}

game_value compileLuaFromFile(game_value_parameter rightArg) {
    std::string code(sqf::preprocess_file(rightArg));
    auto result = lua.state.load_buffer(code.c_str(), code.length(), (std::string("@") + static_cast<std::string>(rightArg)).c_str());
    if (result.valid()) {
        return game_value(new GameDataLuaCode(result.get<sol::protected_function>(), rightArg));
    }
    return game_value();
}

game_value callLua_String(game_value_parameter leftArg, game_value_parameter rightArg) {
    std::string ret = lua.state[static_cast<std::string>(rightArg)](sol::object(lua.state, sol::in_place, static_cast<std::string>(leftArg)));
    return ret;
}

game_value callLua_Code(game_value_parameter leftArg, game_value_parameter rightArg) {
    auto code = static_cast<GameDataLuaCode*>(rightArg.data.get());
    std::string ret = code->code(sol::object(lua.state, sol::in_place, static_cast<std::string>(leftArg)));
    return ret;
}

game_value callLua_Code(game_value_parameter rightArg) {
    auto code = static_cast<GameDataLuaCode*>(rightArg.data.get());
    std::string ret = code->code();
    return ret;
}

LuaManager::LuaManager() : state{} {

}


LuaManager::~LuaManager() {}


types::registered_sqf_function _compileLuaFromFile;

class ClassEntryx : public param_archive_entry {
public:
    //! virtual destructor
    virtual ~ClassEntryx() {}

    // generic entry
    virtual int GetEntryCount() const { __debugbreak(); return 0; }
    virtual param_archive_entry *GetEntry(int i) const { __debugbreak(); return NULL; }
    virtual r_string _x() { return r_string(); }
    virtual param_archive_entry *FindEntry(const r_string &name) const { __debugbreak(); return NULL; }
    virtual operator float() const { __debugbreak(); return 0; }
    virtual operator int() const { __debugbreak(); return 0; }

    virtual operator int64_t() const { __debugbreak(); return 0; }

    virtual operator const r_string() const { __debugbreak(); return r_string(); }
    virtual operator r_string() const { __debugbreak(); return r_string(); }
    virtual operator bool() const { __debugbreak(); return false; }
    virtual r_string GetContext(const char *member = NULL) const { __debugbreak(); return r_string(); }

    // array

    virtual void AddValue(float val) {
        std::stringstream str;
        str << this << " AddValue " << val << "\n";
        //OutputDebugStringA(str.str().c_str());
    }
    virtual void AddValue(int val) {
        std::stringstream str;
        str << this << " AddValue " << val << "\n";
        //OutputDebugStringA(str.str().c_str());
    }

    virtual void AddValue(int64_t val) {
        std::stringstream str;
        str << this << " AddValue " << val << "\n";
        //OutputDebugStringA(str.str().c_str());
    }

    virtual void AddValue(bool val) {
        std::stringstream str;
        str << this << " AddValue " << val << "\n";
        //OutputDebugStringA(str.str().c_str());
    }
    virtual void AddValue(const r_string &val) {
        std::stringstream str;
        str << this << " AddValue " << val/*.data()*/ << "\n";
        //OutputDebugStringA(str.str().c_str());
    }
    virtual void ReserveArrayElements(int count) {
        std::stringstream str;
        str << this << " ReserveArrayElements " << count << "\n";
        //OutputDebugStringA(str.str().c_str());
    }
    virtual int GetSize() const { __debugbreak(); return 0; }
    virtual param_archive_array_entry *operator [] (int i) const { __debugbreak(); return new param_archive_array_entry(); }

    // class
    virtual param_archive_entry *AddArray(const r_string &name) {
        std::stringstream str;
        auto newArr = new ClassEntryx;
        str << this << " AddArray " << name.data() << " " << newArr << "\n";
        //OutputDebugStringA(str.str().c_str());
        return newArr;
    }
    virtual param_archive_entry *AddClass(const r_string &name, bool guaranteedUnique = false) {
        std::stringstream str;
        auto newClass = new ClassEntryx;
        str << this << " AddClass " << name.data() << " " << guaranteedUnique << " " << newClass << "\n";
        //OutputDebugStringA(str.str().c_str());
        return newClass;
    }
    virtual void Add(const r_string &name, const r_string &val) {
        std::stringstream str;
        str << this << " Add " << name.data() << " " << val.data() << "\n";
        //OutputDebugStringA(str.str().c_str());
    }
    virtual void Add(const r_string &name, float val) {
        std::stringstream str;
        str << this << " Add " << name.data() << " " << val << "\n";
        //OutputDebugStringA(str.str().c_str());
    }
    virtual void Add(const r_string &name, int val) {
        std::stringstream str;
        str << this << " Add " << name.data() << " " << val << "\n";
        //OutputDebugStringA(str.str().c_str());
    }
    virtual void Add(const r_string &name, int64_t val) {
        std::stringstream str;
        str << this << " Add " << name.data() << " " << val << "\n";
        //OutputDebugStringA(str.str().c_str());
    }
    virtual void Compact() {
        std::stringstream str;
        str << this << " Compact " << "\n";
        //OutputDebugStringA(str.str().c_str());
    }

    //! Delete the entry. Note: it could be used in rare cases only!
    virtual void Delete(const r_string &name) { __debugbreak(); }
};


r_string blubTest() {
    return ""sv;
}

static lua_iface iface_decl{ blubTest };

void intercept::register_interfaces() {
    client::host::register_plugin_interface("lua_iface"sv, 1, &iface_decl);
}


struct lua_iterator_state {
    typedef std::map<std::string, int>::iterator it_t;
    compact_array<char>::const_iterator it;
    compact_array<char>::const_iterator last;

    lua_iterator_state(const r_string& mt) : it(mt.begin()), last(mt.end()) {}
};

void intercept::on_frame() {
    lua.state["SQF"]["events"]["preInit"].tbl.for_each([](sol::object key, sol::object value) {
       static_cast<sol::function>(value)();
    });
}


void intercept::pre_init() {
    intercept::sqf::system_chat("The Intercept LUA plugin is running!");
    lua.state["SQF"]["events"]["preInit"].tbl.for_each([](sol::object key, sol::object value) {
        intercept::sqf::system_chat("preInit "+key.as<std::string>());
        static_cast<sol::function>(value)();
    });
    intercept::sqf::system_chat("LUA preInit done.");
}

void LuaManager::preStart() {
    auto codeType = client::host::register_sqf_type(r_string("LUACODE"), r_string("luaCode"), r_string("Dis is LUA!"), r_string("luaCode"), createGameDataLuaCode);
    GameDataLuaCode_type = codeType.second;
    _execLua = client::host::register_sqf_command("execLUA"sv, "Loads, compiles and executes given Lua file"sv, userFunctionWrapper<executeLua>, game_data_type::ANY, game_data_type::ANY, game_data_type::STRING);
    _compileLua = client::host::register_sqf_command("compileLUA"sv, "Compiles Lua string"sv, userFunctionWrapper<compileLua>, codeType.first, game_data_type::STRING);
    _compileLuaFromFile = client::host::register_sqf_command("compileLUAFromFile"sv, "Preprocesses and compiles LUA from file. Setting source information in case of errors."sv, userFunctionWrapper<compileLuaFromFile>, codeType.first, game_data_type::STRING);
    _callLuaString = client::host::register_sqf_command("callLUA"sv, "Call Named lua function in global Namespace"sv, userFunctionWrapper<callLua_String>, game_data_type::ANY, game_data_type::ANY, game_data_type::STRING);
    _callLuaCodeArgs = client::host::register_sqf_command("call"sv, "Call compiled lua code"sv, userFunctionWrapper<callLua_Code>, game_data_type::ANY, game_data_type::ANY, codeType.first);
    _callLuaCode = client::host::register_sqf_command("call"sv, "Call compiled lua codesv", userFunctionWrapper<callLua_Code>, game_data_type::ANY, codeType.first);

    //state.open_libraries();

    //https://github.com/ThePhD/sol2/issues/332
    //https://github.com/ThePhD/sol2/issues/90
    auto SQF = sol::table();

    SQF["missionNamespace"] = sqf::mission_namespace();
    SQF["createVehicle"] = [](r_string classname, sol::table pos) {
        return RV_BIND_Object(sqf::create_vehicle(classname, {pos[0], pos[1], pos[2] }));
    };
    SQF["player"] = sol::property([]() {
        return RV_BIND_Object(sqf::player());
    });

    SQF["systemChat"] = [](r_string message) {
        sqf::system_chat(message);
    };

    state["SQF"] = SQF;
    state.create_table("SQF_PFH");

    lua.state.new_usertype<r_string>("string",
        sol::constructors<sol::types<>, sol::types<const char*>, sol::types<std::string>, sol::types<std::string_view>>(),
        "size", sol::property(&r_string::size),
        sol::meta_function::to_string, [](const r_string &s) { return s.data(); },
        sol::meta_function::index, [](const r_string &s, size_t i) { return s.data()[i - 1]; },//#TODO add at operator
        "begin", [](const r_string &s) { return s.begin(); },
        "append", sol::overload(
            [](r_string &self, const r_string &other) { self += other; },
            [](r_string &self, const std::string &other) { self += other; },
           [](r_string &self, const char *suffix) { self += suffix; }
        )
        );



    //state["getVariable"] = &getVariable;
    //state["allPlayers"] = &allPlayers;
    //state["player"] = &player;
    //https://github.com/actboy168/vscode-lua-debug
    //https://github.com/ThePhD/sol2/issues/327
    lua.state.new_usertype<r_string>("string",
        sol::constructors<sol::types<>, sol::types<const char*>, sol::types<std::string>, sol::types<std::string_view>>(),
        "size", &r_string::size,
        sol::meta_function::to_string, [](const r_string &s) { return s.data(); },
        sol::meta_function::index, [](const r_string &s, size_t i) { return s.data()[i - 1]; },//#TODO add at operator
        "begin", [](const r_string &s) { return s.begin(); },
        "append", sol::overload(
                [](r_string &self, const r_string &other) { self += other; },
                [](r_string &self, const std::string &other) { self += other; },
                [](r_string &self, const char *suffix) { self += suffix; }
            )
        );


    lua.state.new_usertype<RV_BIND_Object>("object",
        //sol::constructors<sol::types<>, sol::types<const char*>, sol::types<std::string>, sol::types<std::string_view>>(),
        //"size", &r_string::size,
        sol::meta_function::to_string, [](const RV_BIND_Object &s) { return static_cast<r_string>(s.obj); },
        //sol::meta_function::index, [](const r_string &s, size_t i) { return s.data()[i - 1]; },//#TODO add at operator
        "getPosASL", [](const RV_BIND_Object &s) { return sqf::get_pos_asl(s.obj); },
        "positionASL", sol::property(
            [](const RV_BIND_Object &s) { return sqf::get_pos_asl(s.obj); }, 
            [](const RV_BIND_Object &s, vector3 pos) { return sqf::set_pos_asl(s.obj, pos); }),
        "setDir", [](const RV_BIND_Object &s, double dir) { return sqf::set_dir(s.obj, dir); },
        "name", [](const RV_BIND_Object &s){ return sqf::name(s.obj); },
        "getVariable", [](const RV_BIND_Object &s, const r_string& other) { return sqf::get_variable(s.obj, other); }
        );

    lua.state.new_usertype<rv_namespace>("namespace",
        "name", [](const rv_namespace &s) { return sqf::str(s); },
        "getVariable", [](const rv_namespace &s, const r_string& other) {
                auto var = sqf::get_variable(s, other);
                if (var.type_enum() == game_data_type::CODE)
                    return sol::make_object(lua.state,static_cast<code>(var));
        
                return sol::make_object(lua.state, var);
    }
    );

    //Generic game_value type. Could be complex. This type should be returned as little as possible
    lua.state.new_usertype<game_value>("game_value",
        sol::meta_function::to_string, [](const game_value &s) { return static_cast<r_string>(s); },
        "type", [](const game_value &s) {
            return s.type_enum();
        }
    );

    lua.state.new_enum("VariableType"sv,
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


    lua.state.new_usertype<code>("code",
        sol::meta_function::call, [](const code &s) { return sqf::call(s); },
        sol::meta_function::call, [](const code &s, sol::variadic_args va) {
            auto_array<game_value> args;
            for (auto& it : va)
                args.emplace_back(it);
            return sqf::call(s, args);
        }
        );

    lua.state.new_usertype<vector3>("vector3"
    //#TODO fill?
    );




        for (auto& it : sqf::config_classes("true"sv, sqf::config_entry() >> "CfgLuaLibraries"sv)) {
            if (sqf::is_text(sqf::config_entry(it) >> "init"sv)) {
                auto fileName = sqf::get_text(sqf::config_entry(it) >> "init"sv);
                std::string code(sqf::preprocess_file(fileName));
                auto result = lua.state.load_buffer(code.c_str(), code.length(), sqf::config_name(it).c_str());
                if (result.valid())
                    result.get<sol::protected_function>()();
            }
        }

    //lua.state.do_file("P:\\test.luac");







    //param_archive ar;
    //delete ar._p1;
    //ar._p1 = new ClassEntryx();
    //auto gv = compileLua("return \"TEST\";");
    //game_value x = sqf::player();
    //auto err = x.serialize(ar);
    //r_string x2 = ar._p1->_x();
    //StartLuaEditRemoteDebugger(32201, state);
}