dedmen_randomOrientation = coroutine.create(
   function ()
     while (true) do
      coroutine.yield(math.random(0,360))
     end
   end);

function dedmen_vehiclePFH()
    SQF.systemChat("dedmenlib setDir");
    _, newOrient = coroutine.resume(dedmen_randomOrientation);
    Dedmen_vehicle:setDir(newOrient);
end

function dedmen_preInit()
    --https://github.com/dedmen/interceptTest/blob/master/interceptTest/main.cpp#L597
    SQF.systemChat("dedmenlib preinit");
    --Dedmen_vehicle = SQF.createVehicle("B_Quadbike_01_F", SQF.player.positionASL);

    --SQF.events.PFH.dedmenlib = dedmen_vehiclePFH;

    if CBA.canUseWeapon(SQF.player) then
        CBA.addWeapon(SQF.player, "Binocular", false)
        CBA.selectWeapon(SQF.player, "Binocular")
    end
end


--SQF.events.preInit.dedmenlib = dedmen_preInit;
SQF.events.postInit.dedmenlib = dedmen_preInit;



--Dedmen_vehicle = SQF.createVehicle(SQF.configFile.CfgVehicles.B_Quadbike_01_F.name , SQF.player.positionASL);

--SQF.systemChat(SQF.configFile.CfgVehicles.B_Quadbike_01_F.name);
--SQF.systemChat(SQF.configFile.CfgVehicles.B_Quadbike_01_F.displayName:asText());
--SQF.systemChat((SQF.configFile >> "CfgVehicles" >> "B_Quadbike_01_F" >> "displayName"):asText());
--SQF.systemChat(SQF.configFile["CfgVehicles"]["B_Quadbike_01_F"]["displayName"]:asText());
