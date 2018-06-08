CBA = {}
CBA.__index = CBA


function CBA.currentUnit()
    return SQF.missionNamespace.getVariable("CBA_fnc_currentUnit")();
end

function CBA.addWeapon(unit, item, verify)
    return SQF.missionNamespace.getVariable("CBA_fnc_addWeapon")(unit, item, verify or false);
end

function CBA.canUseWeapon(unit, item, verify)
    return SQF.missionNamespace.getVariable("CBA_fnc_canUseWeapon")();
end

function CBA.selectWeapon(unit, weapon, mode)
    return SQF.missionNamespace.getVariable("CBA_fnc_selectWeapon")(unit, weapon, mode or "");
end

if CBA.canUseWeapon(SQF.player) then
    CBA.addWeapon(SQF.player, "Binocular", false)
    CBA.selectWeapon(SQF.player, "Binocular")
end




