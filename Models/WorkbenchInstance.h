//
// Created by dml on 16.06.22.
//

#include "../Json/json11.hpp"

struct WorkbenchInstance
{
    WorkbenchInstance() : WBID(-1), WBAID(-1), PIID(-1) {}

    WorkbenchInstance(int wbid, int wbaid, int piid)
        : WBID(wbid), WBAID(wbaid), PIID(piid) {}

    WorkbenchInstance(json11::Json json)
    {
        WBID = json.array_items()[0].int_value();
        WBAID = json.array_items()[1].int_value();
        PIID = json.array_items()[2].int_value();
    }

    void SetData(int wbid, int wbaid, int piid)
    {
        this->WBID = wbid;
        this->WBAID = wbaid;
        this->PIID = piid;
    }

    json11::Json to_json() const
    {
        return json11::Json::array {
            WBID,
            WBAID,
            PIID
        };
    }

    int WBID;
    int WBAID;
    int PIID;
};

#ifndef EDGE_SERVER_WORKBENCHINSTANCE_H
#define EDGE_SERVER_WORKBENCHINSTANCE_H

#endif //EDGE_SERVER_WORKBENCHINSTANCE_H
