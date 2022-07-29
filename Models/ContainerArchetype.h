#pragma once

#include "../Json/json11.hpp"
#include "cmath"

enum ContainerAvailableType {
    Solid = 1,
    Grainy = 2,
    Liquid = 4,
    Gaseous = 8
};

struct ContainerArchetype{
    ContainerArchetype() : CAID(-1) {}

    ContainerArchetype(int caid, std::string name, float maxMass, float maxVolume, uint8_t ATypes, std::string AArchetypes)
            : CAID(caid), name(name), maxMass(maxMass), maxVolume(maxVolume), availableTypes(ATypes), availableArchetypes(AArchetypes) {}

    ContainerArchetype(int caid, std::string name, float maxMass, float maxVolume, std::string ATypes, std::string AArchetypes)
            : CAID(caid), name(name), maxMass(maxMass), maxVolume(maxVolume), availableArchetypes(AArchetypes) {
        std::string delimiter = ",";

        size_t pos = 0;
        std::string token;
        std::string s = ATypes;
        while ((pos = ATypes.find(delimiter)) != std::string::npos)
        {
            token = s.substr(0, pos);
            s.erase(0, pos + delimiter.length());
            if (token == "Gaseous")
                this->availableTypes += ContainerAvailableType::Gaseous;
            else if (token == "Grainy")
                this->availableTypes += ContainerAvailableType::Grainy;
            else if (token == "Liquid")
                this->availableTypes += ContainerAvailableType::Liquid;
            else
                this->availableTypes += ContainerAvailableType::Solid;
        }
    }

    ContainerArchetype(json11::Json json)
    {
        CAID = json.array_items()[0].int_value();
        name = json.array_items()[1].string_value();
        maxMass = json.array_items()[2].int_value() / 1000.f;
        maxVolume = json.array_items()[3].int_value() / 1000.f;
        availableTypes = json.array_items()[4].int_value();
        availableArchetypes = json.array_items()[5].string_value();
    }

    void SetData(int caid, std::string name, float maxMass, float maxVolume, uint8_t aTypes, std::string aArchetypes)
    {
        this->CAID = caid;
        this->name = name;
        this->maxMass = maxMass;
        this->maxVolume = maxVolume;
        this->availableTypes = aTypes;
        this->availableArchetypes = aArchetypes;
    }

    json11::Json to_json() const {
        return json11::Json::array {
                CAID,
                name,
                (int) round(maxMass * 1000),
                (int) round(maxVolume * 1000),
                availableTypes,
                availableArchetypes
        };
    }

    int CAID;
    std::string name;
    float maxMass;
    float maxVolume;
    uint8_t availableTypes;
    std::string availableArchetypes;
};
