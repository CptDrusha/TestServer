#pragma once

#include <map>
#include <vector>

#include "Data/GraphData.h"
#include "Flecs/flecs.h"
#include "Data/GraphEdge.h"
#include "Data/Vector3D.h"
#include "../Server/AsioServer.h"


// Class-container of flecs world
class FlecsWorld {
public:
    // Creates flecs::world instance and graph data object
    FlecsWorld(std::map<int, Vector3D> waypoints, std::map<int, std::vector<GraphEdge>> adjacencyMap);

    // Initializes flecs world: components, entities and systems
    void Initialize(AsioServer* server, QuadTreeNode* quadTreeRoot) const;

    // Calls progress on flecs world
    bool Progress() const;

private:
    GraphData* graphData;
    flecs::world* ecsWorld;
};