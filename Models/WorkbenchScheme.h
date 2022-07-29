//
// Created by dml on 16.06.22.
//

#include "../Json/json11.hpp"

enum class AlternativeType
{
    None,
    Conversion
};

struct WorkbenchScheme
{
    WorkbenchScheme() : SchemeID(-1), WBAID(-1) {}

    WorkbenchScheme(int schemeID, int wbaid, std::string altType)
        : SchemeID(schemeID), WBAID(wbaid)
    {
        AltType = GetStringToType(altType);
    }

    WorkbenchScheme(json11::Json json)
    {
        SchemeID = json.array_items()[0].int_value();
        WBAID = json.array_items()[1].int_value();
        AltType = GetStringToType(json.array_items()[2].string_value());
    }

    void SetData(int schemeID, int wbaid, std::string altType)
    {
        this->SchemeID = schemeID;
        this->WBAID = wbaid;
        this->AltType = GetStringToType(altType);
    }

    AlternativeType GetStringToType(std::string type)
    {
        if(type == "None")
            return AlternativeType::None;
        else if(type == "Conversion")
            return AlternativeType::Conversion;

        return AlternativeType::None;
    }

    std::string GetTypeToString() const
    {
        if(AltType == AlternativeType::None)
            return "None";
        else if(AltType == AlternativeType::Conversion)
            return "Conversion";

        return "None";
    }

    json11::Json to_json() const
    {
        return json11::Json::array {
            SchemeID,
            WBAID,
            GetTypeToString()
        };
    }

    int SchemeID;
    int WBAID;
    AlternativeType AltType;
};