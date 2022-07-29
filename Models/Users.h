//
// Created by dml on 14.04.2022.
//

#include "../Json/json11.hpp"

struct User{

    User() : UserID(-1) {}

    User(int uid, std::string name, int cid)
        : UserID(uid), UserName(name), CharacterID(cid) {}

    User(json11::Json json)
    {
        UserID = json.array_items()[0].int_value();
        UserName = json.array_items()[1].string_value();
        CharacterID = json.array_items()[2].int_value();
    }

    void SetData(int uid, std::string name, int cid)
    {
        this->UserID = uid;
        this->UserName = name;
        this->CharacterID = cid;
    }

    json11::Json to_json() const {
        return json11::Json::array {
            UserID,
            UserName,
            CharacterID
        };
    }

    int UserID;
    std::string UserName;
    int CharacterID;
};