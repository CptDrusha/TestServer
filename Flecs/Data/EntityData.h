#pragma once

#include "Vector3D.h"
#include "../../Json/json11.hpp"

// Struct to send entity info to clients
struct EntityData {
    int id;
    float speed;
    float scale;
    float elapsedTime;
    float remainingTime;
    Vector3D upperColor;
    Vector3D lowerColor;
    Vector3D startPoint;
    Vector3D endPoint;

    EntityData() = default;
    EntityData(const int id,
               const float speed,
               const float scale,
               const float elapsedTime,
               const float remainingTime,
               const Vector3D upperColor,
               const Vector3D lowerColor,
               const Vector3D startPoint,
               const Vector3D endPoint)
            : id(id), speed(speed), scale(scale),
              elapsedTime(elapsedTime), remainingTime(remainingTime),
              upperColor(upperColor), lowerColor(lowerColor),
              startPoint(startPoint), endPoint(endPoint) {}

    explicit EntityData(const json11::Json json) {
        for (auto jsonItem : json.object_items()) {
            if (jsonItem.first == "id")
                id = jsonItem.second.int_value();
            else if (jsonItem.first == "sp")
                speed = jsonItem.second.number_value();
            else if (jsonItem.first == "sc")
                scale = jsonItem.second.number_value();
            else if (jsonItem.first == "et")
                elapsedTime = jsonItem.second.number_value();
            else if (jsonItem.first == "rt")
                remainingTime = jsonItem.second.number_value();
            else if (jsonItem.first == "uc")
                upperColor = Vector3D(jsonItem.second);
            else if (jsonItem.first == "lc")
                lowerColor = Vector3D(jsonItem.second);
            else if (jsonItem.first == "stp")
                startPoint = Vector3D(jsonItem.second);
            else if (jsonItem.first == "edp")
                endPoint = Vector3D(jsonItem.second);
        }
    }

    json11::Json to_json() const {
        return json11::Json::object {
                { "id", id },
                { "sp", speed },
                { "sc", scale },
                { "et", elapsedTime },
                { "rt", remainingTime },
                { "uc", upperColor },
                { "lc", lowerColor },
                { "stp", startPoint },
                { "edp", endPoint }
        };
    }
};
