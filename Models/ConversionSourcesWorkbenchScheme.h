//
// Created by dml on 16.06.22.
//

#include "../Json/json11.hpp"
#include "cmath"

struct ConversionSourcesWorkbenchScheme
{
    ConversionSourcesWorkbenchScheme() : SchemeID(-1), PAID(-1), ProductPAID(-1) {}

    ConversionSourcesWorkbenchScheme(int schemeID, int paid, int ppaid, float massFraction)
        : SchemeID(schemeID), PAID(paid), MassFraction(massFraction), ProductPAID(ppaid) {}

    ConversionSourcesWorkbenchScheme(json11::Json json)
    {
        SchemeID = json.array_items()[0].int_value();
        PAID = json.array_items()[1].int_value();
        ProductPAID = json.array_items()[2].int_value();
        MassFraction = json.array_items()[3].int_value() / 1000.f;
    }

    void SetData(int schemeID, int paid, int ppaid, float massFraction)
    {
        this->SchemeID = schemeID;
        this->PAID = paid;
        this->ProductPAID = ppaid;
        this->MassFraction = massFraction;
    }

    json11::Json to_json() const
    {
        return json11::Json::array {
            SchemeID,
            PAID,
            ProductPAID,
            (int)round(MassFraction * 1000)
        };
    }

    int SchemeID;
    int PAID;
    int ProductPAID;
    float MassFraction;
};

#ifndef EDGE_SERVER_CONVERSIONSOURCESWORKBENCHSCHEME_H
#define EDGE_SERVER_CONVERSIONSOURCESWORKBENCHSCHEME_H

#endif //EDGE_SERVER_CONVERSIONSOURCESWORKBENCHSCHEME_H
