#pragma once

#include "../Flecs/flecs.h"
#include "../Data/ECSComponents.h"


// Initialization of ECS components
inline void InitializeComponents(flecs::world& ecsWorld) {
    ecsWorld.component<Speed>();
    ecsWorld.component<ETAToNextPoint>();
    ecsWorld.component<Path>();
    ecsWorld.component<GraphDataRef>();
    ecsWorld.component<Offset>();
    ecsWorld.component<Scale>();
    ecsWorld.component<Color>();
    ecsWorld.component<TimeQueueRef>();
    ecsWorld.component<ListeningClients>();

    ecsWorld.component<CalculateEntityTag>();
    ecsWorld.component<UpdateEntityPathTag>();
    ecsWorld.component<GlobalEntityTag>();
    ecsWorld.component<EntityToUpdateTag>();
}