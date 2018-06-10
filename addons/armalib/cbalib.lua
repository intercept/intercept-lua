CBA = {}
CBA.__index = CBA


function CBA.currentUnit()
    return SQF.missionNamespace.CBA_fnc_currentUnit();
end

function CBA.addWeapon(unit, item, verify)
    return SQF.missionNamespace.CBA_fnc_addWeapon(unit, item, verify or false);
end

function CBA.canUseWeapon(unit)
    return SQF.missionNamespace.CBA_fnc_canUseWeapon(unit);
end

function CBA.selectWeapon(unit, weapon, mode)
    return SQF.missionNamespace.CBA_fnc_selectWeapon(unit, weapon, mode or "");
end

return CBA;