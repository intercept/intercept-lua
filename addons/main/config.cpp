class CfgPatches {
	class intercept_lua { //Change this
		name = "Intercept LUA Plugin"; //Change this
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
class Intercept {
    class intercept_lua { //Change this. It's either the name of your project if you have more than one plugin. Or just your name.
        class main { //Change this.
            pluginName = "intercept-lua"; //Change this.
        };
    };
};