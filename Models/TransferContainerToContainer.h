#pragma once

#include "../Json/json11.hpp"
#include "cmath"

struct TransferContainerToContainer
{
    TransferContainerToContainer() : PIID(-1), CIID(-1) {}

    TransferContainerToContainer(int piid, int ciid, float dm)
        : PIID(piid), CIID(ciid), deltaMass(dm) {}

    TransferContainerToContainer(json11::Json json)
    {
        PIID = json.array_items()[0].int_value();
        CIID = json.array_items()[1].int_value();
        deltaMass = json.array_items()[2].int_value() / 1000.f;
    }

    void SetData(int piid, int ciid, float dm)
    {
        this->PIID = piid;
        this->CIID = ciid;
        this->deltaMass = dm;
    }

    json11::Json to_json() const
    {
        return json11::Json::array{
            PIID,
            CIID,
            (int)round(deltaMass * 1000)
        };
    }

    int PIID;
    int CIID;
    float deltaMass;
};