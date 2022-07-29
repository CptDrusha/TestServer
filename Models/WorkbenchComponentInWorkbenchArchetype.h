//
// Created by dml on 19.07.22.
//
#include "../Json/json11.hpp"

struct WorkbenchComponentInWorkbenchArchetype
{
    WorkbenchComponentInWorkbenchArchetype() : WBAID(-1), WBCID(-1) {}

    WorkbenchComponentInWorkbenchArchetype(int wbaid, int wbcid)
        : WBAID(wbaid), WBCID(wbcid) {}

    WorkbenchComponentInWorkbenchArchetype(json11::Json json)
    {
        WBAID = json.array_items()[0].int_value();
            WBCID = json.array_items()[1].int_value();
    }

    void SetData(int wbaid, int wbcid)
    {
        this->WBAID = wbaid;
        this->WBCID = wbcid;
    }

    json11::Json to_json() const
    {
        return json11::Json::array {
            WBAID,
            WBCID
        };
    }

    int WBAID;
    int WBCID;
};