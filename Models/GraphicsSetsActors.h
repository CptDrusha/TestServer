//
// Created by dml on 04.03.2022.
//

#include "../Json/json11.hpp"

struct GraphicsSetsActor {
    GraphicsSetsActor() : GraphicSetID(-1), ActorID(-1) {}

    GraphicsSetsActor(int gid, int aid) : GraphicSetID(gid), ActorID(aid) {}

    GraphicsSetsActor(json11::Json json) {
        GraphicSetID = json.array_items()[0].int_value();
        ActorID = json.array_items()[1].int_value();
    }

    void SetData(int gid, int aid)
    {
        this->GraphicSetID = gid;
        this->ActorID = aid;
    }

    json11::Json to_json() const {
        return json11::Json::array {
            GraphicSetID,
            ActorID
        };
    }

    int GraphicSetID;
    int ActorID;
};