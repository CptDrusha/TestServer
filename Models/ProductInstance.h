#pragma once

#include "../Json/json11.hpp"
#include "cmath"

enum class PresenceType{
    Container,
    Actor,
    Virtual
};

struct ProductInstance {
    ProductInstance() : PIID(-1) {}

    ProductInstance(int id, int paid, float mass, float volume, float quality, int cgsid, std::string meta, PresenceType type)
        : PIID(id), PAID(paid), mass(mass), volume(volume), quality(quality), CurrentGSID(cgsid), meta(meta), type(type) {}

    ProductInstance(int id, int paid, float mass, float volume, float quality, int cgsid, std::string meta, std::string type)
            : PIID(id), PAID(paid), mass(mass), volume(volume), quality(quality), CurrentGSID(cgsid), meta(meta)
    {
        if(type == "Container")
            this->type = PresenceType::Container;
        else if(type == "Actor")
            this->type = PresenceType::Actor;
        else
            this->type = PresenceType::Virtual;
    }

    explicit ProductInstance(json11::Json json) {
        PIID = json.array_items()[0].int_value();
        PAID = json.array_items()[1].int_value();
        mass = json.array_items()[2].int_value() / 1000.f;
        volume = json.array_items()[3].int_value() / 1000.f;
        quality = json.array_items()[4].int_value() / 1000.f;
        CurrentGSID = json.array_items()[5].int_value();
        meta = json.array_items()[6].string_value();
        std::string jsonType = json.array_items()[7].string_value();

        if (jsonType == "Container")
            type = PresenceType::Container;
        else if (jsonType == "Actor")
            type = PresenceType::Actor;
        else
            type = PresenceType::Virtual;
    }

    void SetData(int id, int paid, float mass, float volume, float quality, int cgsid, std::string meta, std::string type) {
        this->PIID = id;
        this->PAID = paid;
        this->mass = mass;
        this->volume = volume;
        this->quality = quality;
        this->CurrentGSID = cgsid;
        this->meta = meta;

        if(type == "Container")
            this->type = PresenceType::Container;
        else if(type == "Actor")
            this->type = PresenceType::Actor;
        else
            this->type = PresenceType::Virtual;
    }

    std::string GetTypeString() const
    {
        if(type == PresenceType::Container)
            return "Container";
        else if(type == PresenceType::Actor)
            return "Actor";
        else
            return "Virtual";
    }

    json11::Json to_json() const {
        return json11::Json::array {
                PIID,
                PAID,
                (int)round(mass * 1000),
                (int)round(volume * 1000),
                (int)round(quality * 1000),
                CurrentGSID,
                meta,
                GetTypeString()
        };
    }

    int PIID;
    int PAID;
    float mass;
    float volume;
    float quality;
    int CurrentGSID;
    std::string meta;
    PresenceType type;
};