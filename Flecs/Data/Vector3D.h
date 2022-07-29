#pragma once

#include "cmath"
#include "../../Json/json11.hpp"


// Struct to work with 3d vectors
struct Vector3D {
    float x;
    float y;
    float z;

    json11::Json to_json() const {
        return json11::Json::object {
                { "x", static_cast<int>(floorf(x * 1000)) },
                { "y", static_cast<int>(floorf(y * 1000)) },
                { "z", static_cast<int>(floorf(z * 1000)) }
        };
    }

    Vector3D() = default;
    Vector3D(const float x, const float y, const float z) : x(x), y(y), z(z) {}

    explicit Vector3D(const json11::Json json) {
        GetDataFromJson(json);
    }

    explicit Vector3D(const std::string jsonString) {
        std::string parseError = "";
        const json11::Json json = json11::Json::parse(jsonString, parseError);

        GetDataFromJson(json);
    }

private:
    void GetDataFromJson(json11::Json json) {
        for (auto jsonItem : json.object_items()) {
            if (jsonItem.first == "x")
                x = jsonItem.second.number_value() / 1000;
            if (jsonItem.first == "y")
                y = jsonItem.second.number_value() / 1000;
            if (jsonItem.first == "z")
                z = jsonItem.second.number_value() / 1000;
        }
    }



public:
    static float Distance(const Vector3D first, const Vector3D second) {
        return sqrt((second.x - first.x) * (second.x - first.x) +
                    (second.y - first.y) * (second.y - first.y) +
                    (second.z - first.z) * (second.z - first.z));
    }

    // Linear interpolation to float values
    static float FLerp(const float a, const float b, const float t) {
        return a * (1.0f - t) + b * t;
    }

    // Linear interpolation to vector values
    static Vector3D VLerp(const Vector3D start, const Vector3D end, const float t) {
        return Vector3D(FLerp(start.x, end.x, t),
                        FLerp(start.y, end.y, t),
                        FLerp(start.z, end.z, t));
    }
};
