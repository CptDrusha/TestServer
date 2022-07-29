#include "../Json/json11.hpp"

struct ContainersInProductArchetype
{
    ContainersInProductArchetype() : PAID(-1), CAID(-1) {}

    ContainersInProductArchetype(int paid, int caid, int count)
        : PAID(paid), CAID(caid), ContainersCount(count) {}

    ContainersInProductArchetype(json11::Json json)
    {
        PAID = json.array_items()[0].int_value();
        CAID = json.array_items()[1].int_value();
        ContainersCount = json.array_items()[2].int_value();
    }

    void SetData(int paid, int caid, int count)
    {
        this->PAID = paid;
        this->CAID = caid;
        this->ContainersCount = count;
    }

    json11::Json to_json() const
    {
        return json11::Json::array
        {
            PAID,
            CAID,
            ContainersCount
        };
    }

    int PAID;
    int CAID;
    int ContainersCount;
};