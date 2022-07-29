#pragma once
#include <map>
#include <vector>

#include "GraphEdge.h"
#include "Vector3D.h"

// Struct to hold data that used in pathfinding
// Needed to pass by pointer in pathfinding functions, so vectors will not be copied every time
struct GraphData {
    std::map<int, Vector3D> waypoints;
    std::map<int, std::vector<GraphEdge>> adjacencyMap;

    GraphData(const std::map<int, Vector3D>& waypoints, const std::map<int, std::vector<GraphEdge>>& adjacencyMap)
            : waypoints(waypoints),
              adjacencyMap(adjacencyMap) {}
};