#pragma once

#include "../Json/json11.hpp"


struct ProductsGraphicsSet {
    ProductsGraphicsSet() : PGSID(-1) {}

    ProductsGraphicsSet(int id, int paid, std::string icon, std::string model)
            : PGSID(id), PAID(paid), icon(icon), model(model) {}

    ProductsGraphicsSet(json11::Json json) {
        PGSID = json.array_items()[0].int_value();
        PAID = json.array_items()[1].int_value();
        icon = json.array_items()[2].string_value();
        model = json.array_items()[3].string_value();
    }

    void SetData(int id, int paid, std::string icon, std::string model) {
        this->PGSID = id;
        this->PAID = paid;
        this->icon = icon;
        this->model = model;
    }

    json11::Json to_json() const {
        return json11::Json::array {
                PGSID,
                PAID,
                icon,
                model
        };
    }

    int PGSID;
    int PAID;
    std::string icon;
    std::string model;
};