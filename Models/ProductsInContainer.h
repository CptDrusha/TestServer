//
// Created by dml on 05.03.2022.
//

#include "../Json/json11.hpp"

struct ProductsInContainer {
    ProductsInContainer() : PIID(-1), CIID(-1) {}

    ProductsInContainer(int piid, int ciid)
        : PIID(piid), CIID(ciid) {}

    ProductsInContainer(json11::Json json) {
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