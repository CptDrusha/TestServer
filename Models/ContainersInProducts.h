//
// Created by dml on 14.04.2022.
//

#include "../Json/json11.hpp"

struct ContainerInProduct {
    ContainerInProduct() : PIID(-1), CIID(-1) {}

    ContainerInProduct(int piid, int ciid)
        : PIID(piid), CIID(ciid) {}

    ContainerInProduct(json11::Json json)
    {
        PIID = json.array_items()[0].int_value();
        CIID = json.array_items()[1].int_value();
    }

    void SetData(int piid, int ciid)
    {
        this->PIID = piid;
        this->CIID = ciid;
    }

    json11::Json to_json() const {
        return json11::Json::array {
            PIID,
            CIID
        };
    }

    int PIID;
    int CIID;
};