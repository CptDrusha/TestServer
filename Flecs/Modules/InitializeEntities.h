#pragma once

#include <iostream>

#include "../Flecs/flecs.h"
#include "../Data/ECSComponents.h"



int COUNT = 1000;

int MIN_SPEED = 120;
int MAX_SPEED = 200;
float SCALE = 1.0f;
float COLOR_MIN_SATURATION = 0.f;
float COLOR_MAX_SATURATION = 0.7f;
float COLOR_MIN_VALUE = 0.2f;
float COLOR_MAX_VALUE = 0.7f;


// Helper method to convert HSV colors to RGB
inline Vector3D HSVToRGB(const int hue, const float saturation, const float value) {
    const float c = value * saturation;
    const float x = c * (1 - abs(hue / 60 % 2 - 1));
    const float m = value - c;

    float r = m;
    float g = m;
    float b = m;

    if (hue < 60) {
        r += c;
        g += x;
    }
    else if (hue < 120) {
        r += x;
        g += c;
    }
    else if (hue < 180) {
        g += c;
        b += x;
    }
    else if (hue < 240) {
        g += x;
        b += c;
    }
    else if (hue < 300) {
        r += x;
        b += c;
    }
    else {
        r += c;
        b += x;
    }

    return Vector3D(r, g, b);
}


// Initialize all entities
inline void InitializeEntities(flecs::world& ecsWorld, GraphData* graphData)
{
    srand(time(nullptr));

    std::cout << "Creating " << COUNT << " entities" << std::endl;

    // Construct common time queue
    TimeQueue* timeQueue = new TimeQueue();

    for (int i = 0; i < COUNT; i++)
    {
        // Create random start point in path, randomize scale, speed and colors
        std::vector<int> path;
        path.push_back(rand() % graphData->waypoints.size());

        const float scale = SCALE * (rand() % 3 / 10.f + 0.9f);

        const float speed = scale * (rand() % (MAX_SPEED - MIN_SPEED + 1) + MIN_SPEED);

        const int upperColorHue = (rand() * 1.f / RAND_MAX) * 300 + 30;
        const float upperColorSaturation = (rand() % static_cast<int>(COLOR_MAX_SATURATION * 100 - COLOR_MIN_SATURATION * 100 + 1)) / 100.f;
        const float upperColorValue = (rand() % static_cast<int>(COLOR_MAX_VALUE * 100 - COLOR_MIN_VALUE * 100 + 1)) / 100.f;
        const Vector3D upperColor = HSVToRGB(upperColorHue, upperColorSaturation, upperColorValue);

        const int lowerColorHue = (rand() * 1.f / RAND_MAX) * 300 + 30;
        const float lowerColorSaturation = (rand() % static_cast<int>(COLOR_MAX_SATURATION * 100 - COLOR_MIN_SATURATION * 100 + 1)) / 100.f;
        const float lowerColorValue = (rand() % static_cast<int>(COLOR_MAX_VALUE * 100 - COLOR_MIN_VALUE * 100 + 1)) / 100.f;
        const Vector3D lowerColor = HSVToRGB(lowerColorHue, lowerColorSaturation, lowerColorValue);

        flecs::entity entity = ecsWorld.entity();
        entity.set<Scale>({ scale });
        entity.set<Speed>({ speed });
        entity.set<Color>({ upperColor, lowerColor });
        entity.set<Path>({ path });
        entity.set<ETAToNextPoint>({ 0, 0 });
        entity.set<TimeQueueRef>({ timeQueue });
        entity.set<GraphDataRef>({ graphData });
        entity.set<ListeningClients>({});
        entity.add<UpdateEntityPathTag>();
    }

    // Create single global entity, that will process entities updates in time queue
    ecsWorld.entity("Global")
            .set<TimeQueueRef>({ timeQueue })
            .add<GlobalEntityTag>();
}