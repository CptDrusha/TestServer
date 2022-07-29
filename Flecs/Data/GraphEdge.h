#pragma once

// Struct to store information about connected vertices and distances
struct GraphEdge {
    int connectedId;
    float distance;

    GraphEdge() = default;

    GraphEdge(int connectedId, float distance) : connectedId(connectedId), distance(distance) {}
};