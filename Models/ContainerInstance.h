//
// Created by dml on 05.03.2022.
//

#include "../Json/json11.hpp"

struct ContainerInstance {
    ContainerInstance() : CIID(-1), CAID(-1), PIID(-1), LIID(-1) {}

    ContainerInstance(int ciid, int caid, int piid, int liid)
        : CIID(ciid), CAID(caid), PIID(piid), LIID(liid) {}

    ContainerInstance(json11::Json json) {
        CIID = json.array_items()[0].int_value();
        CAID = json.array_items()[1].int_value();
        PIID = json.array_items()[2].int_value();
        LIID = json.array_items()[3].int_value();
    }

    void SetData(int ciid, int caid, int piid, int liid)
    {
        this->CIID = ciid;
        this->CAID = caid;
        this->PIID = piid;
        this->LIID = liid;
    }

    json11::Json to_json() const {
        return json11::Json::array {
            CIID,
            CAID,
            PIID,
            LIID
        };
    }

    int CIID;
    int CAID;
    int PIID;
    int LIID;
};