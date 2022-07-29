//
// Created by dml on 03.06.2022.
//

#include "../Json/json11.hpp"

using namespace std::chrono;

struct PlayerMovement
{
    PlayerMovement() : CharacterID(-1) {}

    PlayerMovement(int charID, std::string movedir, std::string lookdir, std::string pos, float speed, std::string timestamp)
            : CharacterID(charID), MoveDirection(movedir), LookDirection(lookdir), OldPosition(pos), Speed(speed), Timestamp(timestamp) {}

    PlayerMovement(json11::Json json)
    {
        this->CharacterID = json.array_items()[0].int_value();
        this->MoveDirection = json.array_items()[1].string_value();
        this->LookDirection = json.array_items()[2].string_value();
        this->OldPosition = json.array_items()[3].string_value();
        this->Speed = json.array_items()[4].int_value() / 1000.f;
        this->Timestamp = json.array_items()[5].string_value();
    }

    void SetData(int charID, std::string movedir, std::string lookdir, std::string pos, float speed, std::string timestamp)
    {
        this->CharacterID = charID;
        this->MoveDirection = movedir;
        this->LookDirection = lookdir;
        this->OldPosition = pos;
        this->Speed = speed;
        this->Timestamp = timestamp;
    }

    json11::Json to_json() const
    {
        return json11::Json::array
                {
                        CharacterID,
                        MoveDirection,
                        LookDirection,
                        OldPosition,
                        (int)(round(Speed * 1000)),
                        Timestamp
                };
    }

    int CharacterID;
    std::string MoveDirection;
    std::string LookDirection;
    std::string OldPosition;
    float Speed;
    std::string Timestamp;
};
