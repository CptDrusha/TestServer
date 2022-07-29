//
// Created by dml on 04.03.2022.
//

#include "../Json/json11.hpp"

struct GraphicsSet
{
    GraphicsSet() : GraphicSetID(-1) {}

    GraphicsSet(json11::Json json)
    {
        GraphicSetID = json.array_items()[0].int_value();
    }

    void SetData(int id)
    {
        this->GraphicSetID = id;
    }

    json11::Json to_json() const
    {
        return json11::Json::array{
            GraphicSetID
        };
    }

    int GraphicSetID;
};