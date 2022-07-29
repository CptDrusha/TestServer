//
// Created by dml on 17.07.22.
//

#include "../Json/json11.hpp"
#include "cmath"

struct WorkbenchComponentSlot {
    WorkbenchComponentSlot() : WBSID(-1), WBCID(-1) {}

    WorkbenchComponentSlot(int wbsid, int wbcid)
            : WBSID(wbsid), WBCID(wbcid) {}

    WorkbenchComponentSlot(json11::Json json) {
        WBSID = json.array_items()[0].int_value();
        WBCID = json.array_items()[1].int_value();
    }

    void SetData(int wbsid, int wbcid) {
        this->WBSID = wbsid;
        this->WBCID = wbcid;
    }

    json11::Json to_json() const
    {
        return json11::Json::array {
            WBSID,
            WBCID
        };
    }

    int WBSID;
    int WBCID;
};
