//
// Created by dml on 16.06.22.
//

#include "../Json/json11.hpp"
#include "cmath"

struct ConversionWorkbenchScheme
{
    ConversionWorkbenchScheme() : SchemeID(-1), Time(0) {}

    ConversionWorkbenchScheme(int schemeID, float time)
        : SchemeID(schemeID), Time(time) {}

    ConversionWorkbenchScheme(json11::Json json)
    {
        SchemeID = json.array_items()[0].int_value();
        Time = json.array_items()[1].int_value() / 1000.f;
    }

    void SetData(int schemeID, float time)
    {
        this->SchemeID = schemeID;
        this->Time = time;
    }

    json11::Json to_json() const
    {
        return json11::Json::array {
            SchemeID,
            (int)round(Time * 1000)
        };
    }

    int SchemeID;
    float Time;
};

#ifndef EDGE_SERVER_CONVERSIONWORKBENCHSCHEME_H
#define EDGE_SERVER_CONVERSIONWORKBENCHSCHEME_H

#endif //EDGE_SERVER_CONVERSIONWORKBENCHSCHEME_H
