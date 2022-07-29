//
// Created by dml on 14.04.2022.
//

#include "../Json/json11.hpp"

struct Character {

    Character() : CharacterID(-1) {}

    Character(int cid, std::string name, int iid)
        : CharacterID(cid), Name(name), InventoryID(iid) {}

    Character(json11::Json json)
    {
        CharacterID = json.array_items()[0].int_value();
        Name = json.array_items()[1].string_value();
        InventoryID = json.array_items()[2].int_value();
    }

    void SetData(int cid, std::string name, int iid)
    {
        this->CharacterID = cid;
        this->Name = name;
        this->InventoryID = iid;
    }

    json11::Json to_json() const {
        return json11::Json::array {
            CharacterID,
            Name,
            InventoryID
        };
    }

    int CharacterID;
    std::string Name;
    int InventoryID;
};