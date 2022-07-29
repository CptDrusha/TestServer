//
// Created by dml on 18.04.2022.
//

#include <iostream>
#include <algorithm>
#include "../Json/json11.hpp"

struct Space
{
    Space() : SID(-1) {}

    Space(int sid, std::string name, std::string pos, std::string rot, float height, float width, float length)
        : SID(sid), Name(name), Pos(pos), Rot(rot), Height(height), Width(width), Length(length) {}

    Space(json11::Json json)
    {
        SID = json.array_items()[0].int_value();
        Name = json.array_items()[1].string_value();
        Pos = json.array_items()[2].string_value();
        Rot = json.array_items()[3].string_value();
        Height = json.array_items()[4].int_value() / 1000.f;
        Width = json.array_items()[5].int_value() / 1000.f;
        Length = json.array_items()[6].int_value() / 1000.f;
    }

    void SetData(int sid, std::string name, std::string pos, std::string rot,
                 float height, float width, float length)
    {
        this->SID = sid;
        this->Name = name;
        this->Pos = pos;
        this->Rot = rot;
        this->Height = height;
        this->Width = width;
        this->Length = length;
    }

    json11::Json to_json() const {
        return json11::Json::array {
            SID,
            Name,
            Pos,
            Rot,
            (int)(round(Height * 1000)),
            (int)(round(Width * 1000)),
            (int)(round(Length * 1000))
        };
    }

    int SID;
    std::string Name;
    std::string Pos;
    std::string Rot;
    float Height;
    float Width;
    float Length;
};