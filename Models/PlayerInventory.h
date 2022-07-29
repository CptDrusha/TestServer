//
// Created by dml on 31.03.2022.
//

#include "../Json/json11.hpp"

struct PlayerInventory {
    PlayerInventory() : UserID(-1), CIID(-1) {}

    PlayerInventory(int userID, int ciid)
        : UserID(userID), CIID(ciid) {}

    PlayerInventory(json11::Json json)
    {
        UserID = json.array_items()[0].int_value();
        CIID = json.array_items()[1].int_value();
    }

    void SetData(int userID, int ciid)
    {
        this->UserID = userID;
        this->CIID = ciid;
    }

    json11::Json to_json() const {
        return json11::Json::array {
            UserID,
            CIID
        };
    }

    int UserID;
    int CIID;
};