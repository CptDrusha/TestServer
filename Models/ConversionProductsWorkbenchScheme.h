//
// Created by dml on 15.07.22.
//

#include "../Json/json11.hpp"
#include "cmath"

struct ConversionProductsWorkbenchScheme
{
    ConversionProductsWorkbenchScheme() : SchemeID(-1), PAID(-1), ProductPAID(-1) {}

    ConversionProductsWorkbenchScheme(int schemeID, int paid, int ppaid, float perfectRation, float qualityMultiplier)
        : SchemeID(schemeID), PAID(paid), ProductPAID(ppaid), PerfectRatio(perfectRation), QualityMultiplier(qualityMultiplier) {}

    ConversionProductsWorkbenchScheme(json11::Json json)
    {
        SchemeID = json.array_items()[0].int_value();
        PAID = json.array_items()[1].int_value();
        ProductPAID = json.array_items()[2].int_value();
        PerfectRatio = json.array_items()[3].int_value() / 1000.f;
        QualityMultiplier = json.array_items()[4].int_value() / 1000.f;
    }

    void SetData(int schemeID, int paid, int ppaid, float perfectRation, float qualityMultiplier)
    {
        this->SchemeID = schemeID;
        this->PAID = paid;
        this->ProductPAID = ppaid;
        this->PerfectRatio = perfectRation;
        this->QualityMultiplier = qualityMultiplier;
    }

    json11::Json to_json() const
    {
        return json11::Json::array {
            SchemeID,
            PAID,
            ProductPAID,
            (int)round(PerfectRatio * 1000),
            (int)round(QualityMultiplier * 1000)
        };
    }

    int SchemeID;
    int PAID;
    int ProductPAID;
    float PerfectRatio;
    float QualityMultiplier;
};