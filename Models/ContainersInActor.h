//
// Created by dml on 05.03.2022.
//

#include "../Json/json11.hpp"

struct ContainersInActor{
    ContainersInActor() : CIID(-1), ActorID(-1) {}

    ContainersInActor(int ciid, int aid)
        : CIID(ciid), ActorID(aid) {}

   ContainersInActor(json11::Json json) {
        CIID = json.array_items()[0].int_value();
        ActorID = json.array_items()[1].int_value();
    }

    void SetData(int ciid, int aid)
    {
        this->CIID = ciid;
        this->ActorID = aid;
    }

    json11::Json to_json() const {
        return json11::Json::array {
            CIID,
            ActorID
        };
    }

    int CIID;
    int ActorID;
};