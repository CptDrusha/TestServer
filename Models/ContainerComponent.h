//
// Created by dml on 05.07.22.
//

#include "../Json/json11.hpp"

struct ContainerComponent
{
    ContainerComponent() : CCID(-1), LIID(-1) {}

    ContainerComponent(int ccid, int iocid, int liid)
        : CCID(ccid), IOCID(iocid), LIID(liid) {}

    ContainerComponent(json11::Json json)
    {
        CCID = json.array_items()[0].int_value();
        IOCID = json.array_items()[1].int_value();
        LIID = json.array_items()[2].int_value();
    }

    void SetData(int ccid, int iocid, int liid)
    {
        this->CCID = ccid;
        this->IOCID = iocid;
        this->LIID = liid;
    }

    json11::Json to_json() const
    {
        return json11::Json::array {
            CCID,
            IOCID,
            LIID
        };
    }

    int CCID;
    int IOCID;
    int LIID;
};