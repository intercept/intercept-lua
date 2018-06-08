class CfgPatches {
	class intercept_lua_armalib { //Change this
		name = "Intercept LUA Default libraries"; //Change this
		units[] = {};
		weapons[] = {};
		requiredVersion = 1.82;
		requiredAddons[] = {"intercept_core"};
		author = "Dedmen"; //Change this
		authors[] = {"Dedmen"}; //Change this
		url = "https://github.com/intercept/intercept-lua"; //Change this
		version = "1.0";
		versionStr = "1.0";
		versionAr[] = {1,0};
	};
};

class CfgLuaLibraries {
	class CBA {
		init = "z\intercept-lua\addons\armalib\cbalib.lua";
	};
	class dedmen {
		init = "z\intercept-lua\addons\armalib\dedmenlib.lua";
	};
};


