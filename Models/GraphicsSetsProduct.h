//
// Created by dml on 04.03.2022.
//

#include "../Json/json11.hpp"

struct GraphicsSetsProduct {
    GraphicsSetsProduct() : GraphicSetID(-1) {}

    GraphicsSetsProduct(int gsid, int paid, int gscid)
            : GraphicSetID(gsid), PAID(paid), GraphicSetClientID(gscid) {}

    GraphicsSetsProduct(json11::Json json)
    {
        GraphicSetID = json.array_items()[0].int_value();
        PAID = json.array_items()[1].int_value();
        GraphicSetClientID = json.array_items()[2].int_value();
    }

    void SetData(int gsid, int paid, int gscid)
    {
        this->GraphicSetID = gsid;
        this->PAID = paid;
        this->GraphicSetClientID = gscid;
    }

    json11::Json to_json() const
    {
        return json11::Json::array
                {
                        GraphicSetID,
                        PAID,
                        GraphicSetClientID
                };
    }

    int GraphicSetID;
    int PAID;
    int GraphicSetClientID;
};