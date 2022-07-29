#include "../Json/json11.hpp"
#include "cmath"

struct CharacterBodyType
{
    CharacterBodyType() : CharacterID(-1) {}

    CharacterBodyType(int charID, bool bmale, float height, float shoulders, float alength, float llength, float muscle, float belly, float fat, float breast, float hips)
            : CharacterID(charID), bMale(bmale), Height(height), Shoulders(shoulders), ArmLength(alength), LegLength(llength), Muscle(muscle), Belly(belly), Fat(fat), Breast(breast), Hips(hips) {}

    CharacterBodyType(json11::Json json)
    {
        CharacterID = json.array_items()[0].int_value();

        bMale = json.array_items()[1].bool_value();

        Height = json.array_items()[2].int_value() / 1000.f;
        Shoulders = json.array_items()[3].int_value() / 1000.f;
        ArmLength = json.array_items()[4].int_value() / 1000.f;
        LegLength = json.array_items()[5].int_value() / 1000.f;
        Muscle = json.array_items()[6].int_value() / 1000.f;
        Belly = json.array_items()[7].int_value() / 1000.f;
        Fat = json.array_items()[8].int_value() / 1000.f;
        Breast = json.array_items()[9].int_value() / 1000.f;
        Hips = json.array_items()[10].int_value() / 1000.f;
    }

    void SetData(int charID, bool bmale, float height, float shoulders, float alength, float llength, float muscle, float belly, float fat, float breast, float hips)
    {
        this->CharacterID = charID;
        this->bMale = bmale;
        this->Height = height;
        this->Shoulders = shoulders;
        this->ArmLength = alength;
        this->LegLength = llength;
        this->Muscle = muscle;
        this->Belly = belly;
        this->Fat = fat;
        this->Breast = breast;
        this->Hips = hips;
    }

    json11::Json to_json() const
    {
        return json11::Json::array
                {
                        CharacterID,
                        bMale,
                        (int)round(Height * 1000),
                        (int)round(Shoulders * 1000),
                        (int)round(ArmLength * 1000),
                        (int)round(LegLength * 1000),
                        (int)round(Muscle * 1000),
                        (int)round(Belly * 1000),
                        (int)round(Fat * 1000),
                        (int)round(Breast * 1000),
                        (int)round(Hips * 1000)
                };
    }

    int CharacterID;
    bool bMale;

    float Height;
    float Shoulders;
    float ArmLength;
    float LegLength;
    float Muscle;
    float Belly;
    float Fat;
    float Breast;
    float Hips;
};

struct CharacterBodyCloth
{
    CharacterBodyCloth() : CharacterID(-1) {}

    CharacterBodyCloth(int charID, std::string name, std::string color)
            : CharacterID(charID), ClothName(name), ClothColor(color) {}

    CharacterBodyCloth(json11::Json json)
    {
        CharacterID = json.array_items()[0].int_value();
        ClothName = json.array_items()[1].string_value();
        ClothColor = json.array_items()[2].string_value();
    }

    void SetData(int charID, std::string name, std::string color)
    {
        CharacterID = charID;
        ClothName = name;
        ClothColor = color;
    }

    json11::Json to_json() const
    {
        return json11::Json::array
                {
                        CharacterID,
                        ClothName,
                        ClothColor
                };
    }

    int CharacterID;
    std::string ClothName;
    std::string ClothColor;
};