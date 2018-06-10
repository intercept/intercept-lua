#include "LuaManager.hpp"
#include "types.hpp"
#include <sstream>

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

game_value executeLua(game_value_parameter leftArg, game_value_parameter rightArg) {
    auto source = sqf::preprocess_file(rightArg);

    lua.state["_this"] = sol::make_object(lua.state, static_cast<game_value>(leftArg));
    try {
        auto result = lua.state.safe_script(source, sol::script_default_on_error);
        //std::string ret = result;
        //return ret;
    } catch (sol::error err) {
        return err.what();
    }
	return {};
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


void intercept::on_frame() {
    auto result = lua.state.safe_script(" \
        for k, v in pairs(SQF.events.PFH) do \
            SQF.systemChat('PFH '..k); \
            v(); \
        end \
        "sv, sol::script_default_on_error);

    //lua.state["SQF"]["events"]["PFH"].get<sol::table>().for_each([](sol::object key, sol::object value) {
    //    std::string name = key.as<std::string>();
    //    intercept::sqf::system_chat("PFH " + name);
    //    auto func = static_cast<sol::protected_function>(value);
    //    sol::protected_function_result result = func();
    //    result.valid();
    //});
}


void intercept::pre_init() {
    intercept::sqf::system_chat("The Intercept LUA plugin is running!");
    auto result = lua.state.safe_script(" \
        for k, v in pairs(SQF.events.preInit) do \
            SQF.systemChat('preInit '..k); \
            v(); \
        end \
        "sv, sol::script_default_on_error);


    //lua.state["SQF"]["events"]["preInit"].get<sol::table>().for_each([](sol::object key, sol::object value) {
    //    intercept::sqf::system_chat("preInit "+key.as<std::string>());
    //    auto func = static_cast<sol::protected_function>(value);
    //    sol::protected_function_result result = func();
    //    auto state = result.status();
    //    result.valid();
    //});
    intercept::sqf::system_chat("LUA preInit done.");
}

void intercept::post_init() {
    intercept::sqf::system_chat("LUA postInit");
    auto result = lua.state.safe_script(" \
        for k, v in pairs(SQF.events.postInit) do \
            SQF.systemChat('preInit '..k); \
            v(); \
        end \
        "sv, sol::script_default_on_error);

    intercept::sqf::system_chat("LUA postInit done.");
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
    static auto _rebuildLibraries = client::host::register_sqf_command("rebuildLuaLibraries"sv, "", [](uintptr_t gs) -> game_value {

        for (auto& it : sqf::config_classes("true"sv, sqf::config_entry() >> "CfgLuaLibraries"sv)) {
            if (sqf::is_text(sqf::config_entry(it) >> "init"sv)) {
                auto fileName = sqf::get_text(sqf::config_entry(it) >> "init"sv);
                auto code = sqf::preprocess_file(fileName);
                auto result = lua.state.safe_script(code, sol::script_default_on_error, sqf::config_name(it).c_str());
            }
        }

        return {};
    }, game_data_type::NOTHING);

    state.open_libraries();

    //state["getVariable"] = &getVariable;
    //state["allPlayers"] = &allPlayers;
    //state["player"] = &player;

    initTypes(state);

    auto events = state["SQF"].get<sol::table>().create("events");

    events.create("PFH");
    events.create("preInit");
    events.create("postInit");


    for (auto& it : sqf::config_classes("true"sv, sqf::config_entry() >> "CfgLuaLibraries"sv)) {
        if (sqf::is_text(sqf::config_entry(it) >> "init"sv)) {
            auto fileName = sqf::get_text(sqf::config_entry(it) >> "init"sv);
            auto code = sqf::preprocess_file(fileName);
            lua.state.safe_script(code, sol::script_default_on_error, sqf::config_name(it).c_str());
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