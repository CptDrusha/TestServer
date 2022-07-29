#pragma once

#include <unordered_set>
#include <vector>
#include <unordered_map>
#include <flecs.h>

#include "../Data/Vector3D.h"


// Quad tree node, which can contain another nodes
class QuadTreeNode {
public:
    QuadTreeNode(float x, float y, float size, int level, int maxLevel);
    ~QuadTreeNode();

    // Method for adding client to node's client list
    void UpdateClientPosition(int clientId, const Vector3D position, const float visibilityRadius = 10000);

    // Method for removing client from node or its children
    void RemoveClient(int clientId);

    // Method for removing entity from entities visibility map
    // We don't have to add entity, because of GetWatchingClients method
    // Position only to find node in which entity is contains
    void RemoveEntity(int entityId, Vector3D position);

    // Method for adding entity to visibility map
    // Position only to find node in which entity is contains
    void EntityWasSent(int entityId, Vector3D position);

    // Debug method for collecting quad tree info and sending it to QuadTreeDebug request
    void AddSelfToDebugList(std::vector<QuadTreeNode*>& vector);

    // Method for removing children which contains no clients
    void ClearEmptyNodes();

    // Method that returns all clients, who didn't receive info about this entity since its last update
    // Position only to find node in which entity is contains
    // So we don't have to hold and update all entities in this node
    std::unordered_set<int> GetWatchingClients(int entityId, Vector3D position);

    // Node size and position parameters
    float x;
    float y;
    float size;

private:
    // Method for subdividing node to four child nodes
    void Subdivide();

    // Shows that node can be safely deleted without losing information of clients
    bool CanBeDeleted();

    // Checking that point is in this node bounds
    bool ContainsPoint(Vector3D position) const;

    // Checking that at least one node corner is visible from this position
    bool IsVisibleFromPoint(Vector3D position, float visibilityRadius) const;


    // Current node detalization level
    int currentLevel;

    // Max tree detalization level, used to checks
    int maxLevel;

    // Child nodes
    QuadTreeNode* northWest = nullptr;
    QuadTreeNode* northEast = nullptr;
    QuadTreeNode* southWest = nullptr;
    QuadTreeNode* southEast = nullptr;

    // Set of all clients that can see this node
    std::unordered_set<int> clients;

    // Map that holds which clients know about this entity
    // Key is entity id
    // Value is set of clients that already received information of this entity
    std::unordered_map<int, std::unordered_set<int>> entitiesClientsMap;
};