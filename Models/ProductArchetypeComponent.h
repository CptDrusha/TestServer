//
// Created by dml on 05.07.22.
//

#include "../Json/json11.hpp"

struct ProductArchetypeComponent
{
    ProductArchetypeComponent() : PAID(-1), CCID(-1) {}

    ProductArchetypeComponent(int paid, int ccid)
        : PAID(paid), CCID(ccid) {}

    ProductArchetypeComponent(json11::Json json)
    {
        PAID = json.array_items()[0].int_value();
        CCID = json.array_items()[1].int_value();
    }

    void SetData(int paid, int ccid)
    {
        this->PAID = paid;
        this->CCID = ccid;
    }

    json11::Json to_json() const
    {
        return json11::Json::array {
            PAID,
            CCID
        };
    }

    int PAID;
    int CCID;
};