//
// Created by dml on 05.03.2022.
//

#include "../Json/json11.hpp"

struct ProductsInActor {
    ProductsInActor() : PIID(-1), ActorID(-1) {}

    ProductsInActor(int piid, int aid)
        : PIID(piid), ActorID(aid) {}

    ProductsInActor(json11::Json json) {
        PIID = json.array_items()[0].int_value();
        ActorID = json.array_items()[1].int_value();
    }

    void SetData(int piid, int aid)
    {
        this->PIID = piid;
        this->ActorID = aid;
    }

    json11::Json to_json() const {
        return json11::Json::array {
            PIID,
            ActorID
        };
    }

    int PIID;
    int ActorID;
};