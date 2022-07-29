#include "../Json/json11.hpp"

enum class SlotInCharacter {
    Hand,
    Body
};

struct ProductInCharacter
{
    ProductInCharacter() : PIID(-1), CharacterID(-1) {}

    ProductInCharacter(int piid, int characterID, std::string slot)
        : PIID(piid), CharacterID(characterID)
    {
        Slot = StringToSlot(slot);
    }

    ProductInCharacter(json11::Json json)
    {
        PIID = json.array_items()[0].int_value();
        CharacterID = json.array_items()[1].int_value();
        Slot = StringToSlot(json.array_items()[2].string_value());
    }

    SlotInCharacter StringToSlot(std::string stringSlot)
    {
        if (stringSlot == "Hand")
            return SlotInCharacter::Hand;
        else if (stringSlot == "Body")
            return SlotInCharacter::Body;

        return SlotInCharacter::Hand;
    }

    void SetData(int piid, int characterID, std::string slot)
    {
        this->PIID = piid;
        this->CharacterID = characterID;
        this->Slot = StringToSlot(slot);
    }

    std::string GetTypeString() const
    {
        if (Slot == SlotInCharacter::Hand)
            return "Hand";
        else if (Slot == SlotInCharacter::Body)
            return "Body";

        return "Hand";
    }

    json11::Json to_json() const
    {
        return json11::Json::array {
            PIID,
            CharacterID,
            GetTypeString()
        };
    }

    int PIID;
    int CharacterID;
    SlotInCharacter Slot;
};