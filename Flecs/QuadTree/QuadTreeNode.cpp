#include <ECSComponents.h>
#include "QuadTreeNode.h"


// Helper method for calculating visibility of point
float Distance2D(const float x1, const float y1, const float x2, const float y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}



QuadTreeNode::QuadTreeNode(const float x, const float y, const float size, const int level, const int maxLevel)
        : currentLevel(level), maxLevel(maxLevel), x(x), y(y), size(size) {
    northWest = nullptr;
    northEast = nullptr;
    southWest = nullptr;
    southEast = nullptr;
}

QuadTreeNode::~QuadTreeNode() {
    if (northWest != nullptr) {
        delete northWest;
        northWest = nullptr;
    }
    if (northEast != nullptr) {
        delete northEast;
        northEast = nullptr;
    }
    if (southWest != nullptr) {
        delete southWest;
        southWest = nullptr;
    }
    if (southEast != nullptr) {
        delete southEast;
        southEast = nullptr;
    }
}



// Method for adding client to node's client list
// Pass it deeper, if node is not on max detalization level
void QuadTreeNode::UpdateClientPosition(int clientId, const Vector3D position, const float visibilityRadius) {
    // Check if client can see this node
    // If client changed its position and cannot see this node more, remove it
    if (!IsVisibleFromPoint(position, visibilityRadius)) {
        // if it can't, remove from list
        RemoveClient(clientId);
        return;
    }

    // If we are on max level, we just need to remember this client
    if (currentLevel >= maxLevel) {
        clients.insert(clientId);
        return;
    }

    // if not, pass client deeper!
    Subdivide();

    if (northWest != nullptr) northWest->UpdateClientPosition(clientId, position, visibilityRadius);
    if (northEast != nullptr) northEast->UpdateClientPosition(clientId, position, visibilityRadius);
    if (southWest != nullptr) southWest->UpdateClientPosition(clientId, position, visibilityRadius);
    if (southEast != nullptr) southEast->UpdateClientPosition(clientId, position, visibilityRadius);
}

// Method for removing client from node or its children
void QuadTreeNode::RemoveClient(const int clientId) {
    // If we are on deepest level, try to erase client
    if (currentLevel >= maxLevel) {
        // Erase client from clients list
        const auto it1 = clients.find(clientId);
        if (it1 != clients.end())
            clients.erase(it1);

        // Erase client from all entity-clients lists
        for (auto entityClientPair : entitiesClientsMap) {
            const auto it2 = entityClientPair.second.find(clientId);
            if (it2 != entityClientPair.second.end())
                entityClientPair.second.erase(it2);
        }
    }

    if (northWest != nullptr) northWest->RemoveClient(clientId);
    if (northEast != nullptr) northEast->RemoveClient(clientId);
    if (southWest != nullptr) southWest->RemoveClient(clientId);
    if (southEast != nullptr) southEast->RemoveClient(clientId);

    ClearEmptyNodes();
}

// Method for removing entity from entities visibility map
// We don't have to add entity, because of GetWatchingClients method
// Position only to find node in which entity is contains
void QuadTreeNode::RemoveEntity(int entityId, Vector3D position) {
    // Pass entity to children
    if (northWest != nullptr && northWest->ContainsPoint(position)) {
        northWest->RemoveEntity(entityId, position);
        return;
    }
    if (northEast != nullptr && northEast->ContainsPoint(position)) {
        northEast->RemoveEntity(entityId, position);
        return;
    }
    if (southWest != nullptr && southWest->ContainsPoint(position)) {
        southWest->RemoveEntity(entityId, position);
        return;
    }
    if (southEast != nullptr && southEast->ContainsPoint(position)) {
        southEast->RemoveEntity(entityId, position);
        return;
    }

    // If children are not exists, search in current map
    auto it = entitiesClientsMap.find(entityId);
    if (it != entitiesClientsMap.end())
        entitiesClientsMap.erase(it);
}

// Method for adding entity to visibility map
// Position only to find node in which entity is contains
void QuadTreeNode::EntityWasSent(int entityId, Vector3D position) {
    // If we on max level, add all clients to map
    if (currentLevel >= maxLevel) {
        auto it = entitiesClientsMap.find(entityId);
        if (it != entitiesClientsMap.end()) {
            entitiesClientsMap.erase(it);
        }
        entitiesClientsMap[entityId] = clients;
        return;
    }

    // If not, pass entity deeper
    if (northWest != nullptr && northWest->ContainsPoint(position)) {
        northWest->EntityWasSent(entityId, position);
        return;
    }
    if (northEast != nullptr && northEast->ContainsPoint(position)) {
        northEast->EntityWasSent(entityId, position);
        return;
    }
    if (southWest != nullptr && southWest->ContainsPoint(position)) {
        southWest->EntityWasSent(entityId, position);
        return;
    }
    if (southEast != nullptr && southEast->ContainsPoint(position)) {
        southEast->EntityWasSent(entityId, position);
        return;
    }
}


// Method for removing children which contains no clients
void QuadTreeNode::ClearEmptyNodes() {
    // If we are on max level, we don't need to delete anything
    if (currentLevel >= maxLevel)
        return;

    // If any child exists and cannot be deleted, call clear method on all children
    // Delete children only if all can be deleted, so there always can be only all children or none of them
    if (northWest != nullptr && !northWest->CanBeDeleted()
        || northEast != nullptr && !northEast->CanBeDeleted()
        || southWest != nullptr && !southWest->CanBeDeleted()
        || southEast != nullptr && !southEast->CanBeDeleted()) {
        if (northWest != nullptr)
            northWest->ClearEmptyNodes();
        if (northEast != nullptr)
            northEast->ClearEmptyNodes();
        if (southWest != nullptr)
            southWest->ClearEmptyNodes();
        if (southEast != nullptr)
            southEast->ClearEmptyNodes();
    } else {
        if (northWest != nullptr) {
            delete northWest;
            northWest = nullptr;
        }
        if (northEast != nullptr) {
            delete northEast;
            northEast = nullptr;
        }
        if (southWest != nullptr) {
            delete southWest;
            southWest = nullptr;
        }
        if (southEast != nullptr) {
            delete southEast;
            southEast = nullptr;
        }
    }
}

// Method that returns all clients, who didn't receive info about this entity since its last update
// Position only to find node in which entity is contains
// So we don't have to hold and update all entities in this node
// Only entities on max detalization level will be shown
std::unordered_set<int> QuadTreeNode::GetWatchingClients(int entityId, const Vector3D position) {
    // Not checking if node contains point because we are starting from root at any cases,
    // and checking containing before going deeper

    // Empty vector which will be returned if no clients can watch this node
    std::unordered_set<int> result;

    // If we are on max level, return clients that can see this node
    if (currentLevel >= maxLevel) {
        // If no such entity in map, it means than no one client knows about it
        auto it1 = entitiesClientsMap.find(entityId);
        if (it1 == entitiesClientsMap.end())
            return clients;

        // Iterate over clients and add that clients,
        // who don't know about this entity
        for (int client : clients) {
            auto it2 = entitiesClientsMap[entityId].find(client);
            if (it2 == entitiesClientsMap[entityId].end())
                result.insert(client);
        }

        return result;
    }

    // If any child contains point, return its clients
    if (northWest != nullptr && northWest->ContainsPoint(position))
        return northWest->GetWatchingClients(entityId, position);
    if (northEast != nullptr && northEast->ContainsPoint(position))
        return northEast->GetWatchingClients(entityId, position);
    if (southWest != nullptr && southWest->ContainsPoint(position))
        return southWest->GetWatchingClients(entityId, position);
    if (southEast != nullptr && southEast->ContainsPoint(position))
        return southEast->GetWatchingClients(entityId, position);

    // If there is no children, or they are not contain point, return empty vector
    // It means that we aren't on deepest level, so no one client has to see this node
    return result;
}



// Method for subdividing node to four child nodes
void QuadTreeNode::Subdivide() {
    const float halfSize = size / 2;

    if (northWest == nullptr)
        northWest = new QuadTreeNode(x + halfSize, y + halfSize, halfSize, currentLevel + 1, maxLevel);
    if (northEast == nullptr)
        northEast = new QuadTreeNode(x - halfSize, y + halfSize, halfSize, currentLevel + 1, maxLevel);
    if (southWest == nullptr)
        southWest = new QuadTreeNode(x + halfSize, y - halfSize, halfSize, currentLevel + 1, maxLevel);
    if (southEast == nullptr)
        southEast = new QuadTreeNode(x - halfSize, y - halfSize, halfSize, currentLevel + 1, maxLevel);
}

// Checking that point is in this node bounds
bool QuadTreeNode::ContainsPoint(const Vector3D position) const {
    return position.x > x - size
           && position.x < x + size
           && position.y > y - size
           && position.y < y + size;
}

// Checking if this node is visible from given position
bool QuadTreeNode::IsVisibleFromPoint(const Vector3D position, const float visibilityRadius) const {
    // Check if client is in this cell or in neighbour cells
    const float xDist = abs(x - position.x);
    const float yDist = abs(y - position.y);

    if (xDist < 2 * size && yDist < 2 * size)
        return true;

    // If not, check distance to client
    float distance;

    if (position.x > x) {
        if (position.y > y)
            distance = Distance2D(x + size, y + size, position.x, position.y);
        else
            distance = Distance2D(x + size, y - size, position.x, position.y);
    } else {
        if (position.y > y)
            distance = Distance2D(x - size, y + size, position.x, position.y);
        else
            distance = Distance2D(x - size, y - size, position.x, position.y);
    }

    return distance < visibilityRadius;
}

// Shows that node can be safely deleted without losing information of clients
bool QuadTreeNode::CanBeDeleted() {
    if (currentLevel >= maxLevel)
        return clients.size() == 0;

    bool flag = true;

    // Node can be deleted only if all children can be deleted
    if (northWest != nullptr && !northWest->CanBeDeleted())
        flag = false;
    if (northEast != nullptr && !northEast->CanBeDeleted())
        flag = false;
    if (southWest != nullptr && !southWest->CanBeDeleted())
        flag = false;
    if (southEast != nullptr && !southEast->CanBeDeleted())
        flag = false;

    return flag;
}

// Debug method for collecting quad tree info and sending it to QuadTreeDebug request
void QuadTreeNode::AddSelfToDebugList(std::vector<QuadTreeNode*>& vector) {
    bool bHasChildren = false;

    if (northWest != nullptr) {
        northWest->AddSelfToDebugList(vector);
        bHasChildren = true;
    }
    if (northEast != nullptr) {
        northEast->AddSelfToDebugList(vector);
        bHasChildren = true;
    }
    if (southWest != nullptr) {
        southWest->AddSelfToDebugList(vector);
        bHasChildren = true;
    }
    if (southEast != nullptr) {
        southEast->AddSelfToDebugList(vector);
        bHasChildren = true;
    }

    if (bHasChildren)
        return;

    vector.push_back(this);
}