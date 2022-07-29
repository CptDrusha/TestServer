#pragma once

#include "Vector3D.h"
#include "GraphData.h"
#include "unordered_set"
#include "flecs.h"


// File that just holds all ECS component structs

struct Speed {
    float value;
};

struct ETAToNextPoint {
    float start;
    float end;
};

struct Path {
    std::vector<int> value;
};

struct Offset {
    float value;
};

struct Scale {
    float value;
};

struct Color {
    Vector3D upperColor;
    Vector3D lowerColor;
};

struct GraphDataRef {
    GraphData* value;
};

struct ListeningClients {
    std::unordered_set<int> clients;
};



// Time Queue

// NOT A COMPONENT!!
struct TimeBatch {
    float time;
    std::vector<flecs::entity_view> entitiesViews;
};

// NOT A COMPONENT!!
struct TimeQueue {
    float startTime;
    std::vector<TimeBatch> batches;
};

struct TimeQueueRef {
    TimeQueue* value;
};



// Tags

struct CalculateEntityTag {};
struct UpdateEntityPathTag {};
struct GlobalEntityTag {};
struct EntityToUpdateTag {};