//
// Created by dml on 16.06.22.
//

#include "../Json/json11.hpp"

struct WorkbenchArchetype
{
    WorkbenchArchetype() : WBAID(-1), PAID(-1) {}

    WorkbenchArchetype(int wbaid, int paid)
        : WBAID(wbaid), PAID(paid) {}

    WorkbenchArchetype(json11::Json json)
    {
        WBAID = json.array_items()[0].int_value();
        PAID = json.array_items()[1].int_value();
    }

    void SetData(int wbaid, int paid)
    {
        this->WBAID = wbaid;
        this->PAID = paid;
    }

    json11::Json to_json() const
    {
        return json11::Json::array {
            WBAID,
            PAID
        };
    }

    int WBAID;
    int PAID;
};

#ifndef EDGE_SERVER_WORKBENCHARCHETYPE_H
#define EDGE_SERVER_WORKBENCHARCHETYPE_H

#endif //EDGE_SERVER_WORKBENCHARCHETYPE_H
