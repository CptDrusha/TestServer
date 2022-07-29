//
// Created by dml on 20.07.22.
//

#include "../Json/json11.hpp"
#include "cmath"

struct ConversionInfo
{
    ConversionInfo() : WBID(-1), SchemeID(-1), WBSID(-1) {}

    ConversionInfo(int wbid, int schemeID, int wbsid)
        : WBID(wbid), SchemeID(schemeID), WBSID(wbsid) {}

    ConversionInfo(json11::Json json)
    {
        WBID = json.array_items()[0].int_value();
        SchemeID = json.array_items()[1].int_value();
        WBSID = json.array_items()[2].int_value();
    }

    void SetData(int wbid, int schemeID, int wbsid)
    {
        this->WBID = wbid;
        this->SchemeID = schemeID;
        this->WBSID = wbsid;
    }

    json11::Json to_json() const
    {
        return json11::Json::array {
            WBID,
            SchemeID,
            WBSID
        };
    }

    bool operator==(ConversionInfo info)
    {
        return (WBID == info.WBID && SchemeID == info.SchemeID && WBSID == info.WBSID);
    }

    int WBID;
    int SchemeID;
    int WBSID;
};