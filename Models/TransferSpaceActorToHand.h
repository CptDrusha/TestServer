
#include "../Json/json11.hpp"

struct TransferSpaceActorToHand
{
    TransferSpaceActorToHand() : ActorID(-1), CharacterID(-1) {}

    TransferSpaceActorToHand(int aid, int charId)
        : ActorID(aid), CharacterID(charId) {}

    TransferSpaceActorToHand(json11::Json json)
    {
        ActorID = json.array_items()[0].int_value();
        CharacterID = json.array_items()[1].int_value();
    }

    void SetData(int aid, int charId)
    {
        this->ActorID = aid;
        this->CharacterID = charId;
    }

    json11::Json to_json() const
    {
        return json11::Json::array {
            ActorID,
            CharacterID
        };
    }

    int ActorID;
    int CharacterID;
};