//
// Created by dml on 17.07.22.
//

#include "../Json/json11.hpp"
#include "cmath"

struct WorkbenchComponent
{
    WorkbenchComponent() : WBCID(-1), SlotCount(1) {}

    WorkbenchComponent(int wbcid, bool useRec, bool start, int slotCount)
        : WBCID(wbcid), UseRecomendations(useRec), AutoStart(start), SlotCount(slotCount) {}

    WorkbenchComponent(json11::Json json)
    {
        WBCID = json.array_items()[0].int_value();
        UseRecomendations = json.array_items()[1].bool_value();
        AutoStart = json.array_items()[2].bool_value();
        SlotCount = json.array_items()[3].int_value();
    }

    void SetData(int wbcid, bool useRec, bool start, int slotCount)
    {
        this->WBCID = wbcid;
        this->UseRecomendations = useRec;
        this->AutoStart = start;
        this->SlotCount = slotCount;
    }

    json11::Json to_json() const
    {
        return json11::Json::array {
            WBCID,
            UseRecomendations,
            AutoStart,
            SlotCount
        };
    }

    int WBCID;
    bool UseRecomendations;
    bool AutoStart;
    int SlotCount;
};
