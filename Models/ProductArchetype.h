#pragma once

#include "../Json/json11.hpp"
#include "cmath"


enum class ProductArchetypeType : uint8_t {
    Solid,
    Grainy,
    Liquid,
    Gaseous
};

struct ProductArchetype {
    ProductArchetype() : PAID(-1) {}

    ProductArchetype(int paid, std::string name, std::string desc, std::string actor, float density, ProductArchetypeType type)
            : PAID(paid), name(name), description(desc), actorClass(actor), density(density), type(type) {}

    ProductArchetype(int paid, std::string name, std::string desc, std::string actor, float density, std::string type)
            : PAID(paid), name(name), description(desc), actorClass(actor), density(density) {
        if (type == "Gaseous")
            this->type = ProductArchetypeType::Gaseous;
        else if (type == "Grainy")
            this->type = ProductArchetypeType::Grainy;
        else if (type == "Liquid")
            this->type = ProductArchetypeType::Liquid;
        else
            this->type = ProductArchetypeType::Solid;
    }

    ProductArchetype(json11::Json json) {
        PAID = json.array_items()[0].int_value();
        name = json.array_items()[1].string_value();
        description = json.array_items()[2].string_value();
        actorClass = json.array_items()[3].string_value();
        density = json.array_items()[4].int_value() / 1000.f;
        std::string jsonType = json.array_items()[5].string_value();

        if (jsonType == "Gaseous")
            type = ProductArchetypeType::Gaseous;
        else if (jsonType == "Grainy")
            type = ProductArchetypeType::Grainy;
        else if (jsonType == "Liquid")
            type = ProductArchetypeType::Liquid;
        else
            type = ProductArchetypeType::Solid;
    }

    void SetData(int paid, std::string name, std::string desc, std::string actor, float density, std::string type) {
        this->PAID = paid;
        this->name = name;
        this->description = desc;
        this->actorClass = actor;
        this->density = density;

        if (type == "Gaseous")
            this->type = ProductArchetypeType::Gaseous;
        else if (type == "Grainy")
            this->type = ProductArchetypeType::Grainy;
        else if (type == "Liquid")
            this->type = ProductArchetypeType::Liquid;
        else
            this->type = ProductArchetypeType::Solid;
    }

    std::string GetTypeString() const {
        if (type == ProductArchetypeType::Gaseous)
            return "Gaseous";
        else if (type == ProductArchetypeType::Grainy)
            return "Grainy";
        else if (type == ProductArchetypeType::Liquid)
            return "Liquid";
        else
            return "Solid";
    }

    json11::Json to_json() const {
        return json11::Json::array {
                PAID,
                name,
                description,
                actorClass,
                (int)round(density * 1000),
                GetTypeString()
        };
    }

    int PAID;
    std::string name;
    std::string description;
    std::string actorClass;
    float density;
    ProductArchetypeType type;
};