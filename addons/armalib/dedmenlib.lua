function dedmen_vehiclePFH()
    SQF.systemChat("dedmenlib setDir");
    Dedmen_vehicle.setDir(math.random(0,360));
end

function dedmen_preInit()
    --https://github.com/dedmen/interceptTest/blob/master/interceptTest/main.cpp#L597
    SQF.systemChat("dedmenlib preinit");
    Dedmen_vehicle = SQF.createVehicle("B_Quadbike_01_F", SQF.player.positionASL);

    SQF.events.PFH["dedmenlib"] = dedmen_vehiclePFH;
end


SQF.events.preInit["dedmenlib"] = dedmen_preInit;