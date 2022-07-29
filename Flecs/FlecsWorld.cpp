#include "iostream"
#include "FlecsWorld.h"
#include "Modules/InitializeComponents.h"
#include "Modules/InitializeEntities.h"
#include "Modules/InitializeSystems.h"



// Creates flecs::world instance and graph data object
FlecsWorld::FlecsWorld(const std::map<int, Vector3D> waypoints,
                       const std::map<int, std::vector<GraphEdge>> adjacencyMap) {
    ecsWorld = new flecs::world();
    graphData = new GraphData(waypoints, adjacencyMap);
}

// Initializes flecs world: components, entities and systems
void FlecsWorld::Initialize(AsioServer* server, QuadTreeNode* quadTreeRoot) const {
    InitializeComponents(*ecsWorld);
    InitializeEntities(*ecsWorld, graphData);
    InitializeSystems(*ecsWorld, server, quadTreeRoot);
}

// Calls progress on flecs world
bool FlecsWorld::Progress() const {
    return ecsWorld->progress();
}