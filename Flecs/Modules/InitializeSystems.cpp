#include "InitializeSystems.h"

#include <map>
#include <queue>
#include <string>
#include <vector>

#include "../Data/ECSComponents.h"
#include "../Data/EntityData.h"


// Global variables
// static makes them only file-visible
static ENetServer* ENET_SERVER;
static QuadTreeNode* QUAD_TREE_ROOT;
static int MAX_BATCH_SEND_COUNT = 10;



// Pathfinding functions
bool FindPath(int startId, const int endId,
              GraphData* graphData,
              std::map<int, int>& cameFrom, std::map<int, float>& costSoFar) {
    // Typedef to shorten declarations
    typedef std::pair<float, int> f_element;
    std::priority_queue<f_element, std::vector<f_element>, std::greater<f_element>> queue;
    queue.emplace(0, startId);

    const Vector3D targetLocation = graphData->waypoints[endId];

    cameFrom[startId] = startId;
    costSoFar[startId] = 0;

    while (!queue.empty()) {
        const int currentId = queue.top().second;
        queue.pop();

        if (currentId == endId)
            return true;

        for (const GraphEdge edge : graphData->adjacencyMap[currentId])	{
            int nextId = edge.connectedId;
            const Vector3D nextLocation = graphData->waypoints[nextId];

            const float newCost = costSoFar[currentId] + edge.distance;

            if (costSoFar.find(nextId) == costSoFar.end() || newCost < costSoFar[nextId]) {
                costSoFar[nextId] = newCost;
                float priority = newCost + Vector3D::Distance(nextLocation, targetLocation);;
                queue.emplace(priority, nextId);
                cameFrom[nextId] = currentId;
            }
        }
    }

    return false;
}

std::vector<int> ReconstructPath(
        const int startId,
        const int endId,
        std::map<int, int> cameFrom) {
    std::vector<int> path;
    int currentId = endId;
    path.emplace(path.begin(), currentId);

    while (currentId != startId) {
        currentId = cameFrom[currentId];
        path.emplace(path.begin(), currentId);
    }

    return path;
}

std::vector<int> FindNeighbour(GraphData* graphData, const int currentId) {
    std::vector<GraphEdge> row = graphData->adjacencyMap[currentId];
    const int next = row[rand() % row.size()].connectedId;

    return std::vector<int> {currentId, next};
}

std::vector<int> FindNewPath(GraphData* graphData, const int currentId) {
    if (graphData->adjacencyMap.find(currentId) == graphData->adjacencyMap.end())
        return std::vector<int> {currentId};

    const int nextId = rand() % graphData->waypoints.size();

    if (nextId == currentId)
        return FindNeighbour(graphData, currentId);

    std::map<int, int> cameFrom;
    std::map<int, float> costSoFar;

    if (FindPath(currentId, nextId, graphData, cameFrom, costSoFar)) {
        return ReconstructPath(currentId, nextId, cameFrom);
    }

    return FindNeighbour(graphData, currentId);
}




// System that processes time queue and sets calculate flags to entities that reached their endpoints
void SystemSetCalculateFlags(flecs::iter& it, TimeQueueRef* timeQueue, GlobalEntityTag* tag) {
    const float currentTime = it.world_time();

    // Process all time batches that are completed at this time
    while (timeQueue->value->batches.size() > 0 && timeQueue->value->batches[0].time < currentTime)	{
        const TimeBatch batch = timeQueue->value->batches.front();

        // Set a last processed batch time in time queue, to calculate entity from it
        timeQueue->value->startTime = batch.time;
        timeQueue->value->batches.erase(timeQueue->value->batches.begin());

        // Update entities tags
        for (auto entity : batch.entitiesViews)	{
            entity.mut(it.world()).add<CalculateEntityTag>();
        }
    }
}


// Batches interval in time queue
float CALCULATE_BATCH_INTERVAL = .1f;

// System that calculates new end time of entity and puts it in time queue
void SystemCalculatePoints(flecs::iter& it, CalculateEntityTag* tag, Path* path, Speed* speed,
                           ETAToNextPoint* etaToNext, TimeQueueRef* timeQueue, GraphDataRef* graphData) {
    const float currentEndTime = timeQueue->value->startTime;

    for (auto i : it) {
        auto entity = it.entity(i);

        entity.remove<CalculateEntityTag>();

        // Remove entity from all quad tree lists, because of its update, so no one client know about it yet
        QUAD_TREE_ROOT->RemoveEntity(entity.id(), graphData->value->waypoints[path[i].value[0]]);

        // Skip all reached waypoints
        while (etaToNext[i].end <= currentEndTime && path[i].value.size() > 1) {
            path[i].value.erase(path[i].value.begin());
            etaToNext[i].start = etaToNext[i].end;

            if (path[i].value.size() == 1)
                break;

            // Calculate new end time to next waypoint
            // If it's still less than current time, it will be recalculated on next loop cycle
            etaToNext[i].end += Vector3D::Distance(
                    graphData->value->waypoints[path[i].value[0]],
                    graphData->value->waypoints[path[i].value[1]]) / speed[i].value;
        }

        // If entity reached all its waypoints, set a flag to generate a new path
        if (path[i].value.size() <= 1) {
            entity.add<UpdateEntityPathTag>();
            continue;
        }

        // Calculate new position in time queue
        const float timeOffset = etaToNext[i].end - currentEndTime;
        const int queuePosition = static_cast<int>(timeOffset / CALCULATE_BATCH_INTERVAL);

        if (timeQueue[i].value->batches.size() <= queuePosition) {
            float lastBatchTime = timeQueue[i].value->batches.size() > 0
                                  ? timeQueue[i].value->batches.back().time
                                  : currentEndTime;

            for (int j = timeQueue[i].value->batches.size(); j <= queuePosition; j++) {
                lastBatchTime += CALCULATE_BATCH_INTERVAL;
                timeQueue[i].value->batches.push_back(TimeBatch{ lastBatchTime, std::vector<flecs::entity_view>() });
            }
        }

        timeQueue[i].value->batches[queuePosition].entitiesViews.push_back(entity);
    }
}

// System to generate new path to entity
void SystemUpdatePath(flecs::iter& it, UpdateEntityPathTag* tag, Path* path, GraphDataRef* graphData) {
    for (auto i : it) {
        path[i].value = FindNewPath(graphData->value, path[i].value[0]);

        it.entity(i).remove<UpdateEntityPathTag>();
        it.entity(i).add<CalculateEntityTag>();
    }
}


// System to set tag to send entities to clients
void SystemCheckEntitiesToSend(flecs::iter& it, Path* path, GraphDataRef* graphData, ListeningClients* listeningClients) {
    for (auto i : it) {
        if (path[i].value.size() <= 1)
            continue;

        auto entity = it.entity(i);

        // Are we need to delete old set because of memory issues??
        // listeningClients[i].clients.clear();

        listeningClients[i].clients = QUAD_TREE_ROOT->GetWatchingClients(entity.id(),
                                                                         graphData->value->waypoints[path[i].value[0]]);

        if (listeningClients[i].clients.size() > 0)
            entity.add<EntityToUpdateTag>();
    }
}



// System to send entities to connected clients that can see them
void SystemSendEntitiesToClients(flecs::iter& it, Path* path, ETAToNextPoint* etaToNext,
                                 Color* color, Speed* speed, Scale* scale,
                                 GraphDataRef* graphData, ListeningClients* listeningClients, EntityToUpdateTag* tag) {
    // Map, where key is client id, and value is all entities that he can see
    std::map<int, std::vector<EntityData>> clientsData;
    const float currentTime = it.world_time();

    for (auto i : it) {
        if (listeningClients[i].clients.size() == 0)
            continue;

        // Calculate values to send entity
        auto entity = it.entity(i);

        const float elapsedTime = currentTime - etaToNext[i].start;
        const float remainingTime = etaToNext[i].end - currentTime;

        Vector3D startPoint = graphData->value->waypoints[path[i].value[0]];
        const Vector3D endPoint = path[i].value.size() > 1
                                  ? graphData->value->waypoints[path[i].value[1]]
                                  : startPoint;

        EntityData data(entity.id(), speed[i].value, scale[i].value,
                        elapsedTime, remainingTime,
                        color[i].upperColor, color[i].lowerColor,
                        startPoint, endPoint);

        // Put entity data to all client's vectors
        for (int client : listeningClients[i].clients) {
            std::vector<EntityData> entitiesData;
            if (clientsData.find(client) != clientsData.end())
                entitiesData = clientsData[client];

            entitiesData.push_back(data);
            clientsData[client] = entitiesData;
        }

        entity.remove<EntityToUpdateTag>();
        QUAD_TREE_ROOT->EntityWasSent(entity.id(), startPoint);
    }

    // Finally, send collected data to clients
    for (auto clientData : clientsData)	{
        for (size_t i = 0; i < clientData.second.size(); i += MAX_BATCH_SEND_COUNT) {
            auto last = std::min(clientData.second.size(), i + MAX_BATCH_SEND_COUNT);

            std::vector<EntityData> temp;
            for (int j = i; j < last; j++) {
                temp.push_back(clientData.second[j]);
            }

            const std::string jsonString = json11::Json(temp).dump();
            ENET_SERVER->SendMessageToClientById(clientData.first, jsonString);
        }
    }
}

// System to call update on asio server to process new messages
// Maybe it has to be in another thread
void SystemReadServerMessages(flecs::iter& it, GlobalEntityTag* tag) {
    ENET_SERVER->Update();
}




// Register systems in flecs world
void InitializeSystems(flecs::world& ecsWorld, EnetServer* server, QuadTreeNode* quadTreeRoot)
{
    ENET_SERVER = server;
    QUAD_TREE_ROOT = quadTreeRoot;

    ecsWorld.system<TimeQueueRef, GlobalEntityTag>("SetCalculateFlags")
            .iter(SystemSetCalculateFlags);

    ecsWorld.system<CalculateEntityTag, Path, Speed, ETAToNextPoint,
                    TimeQueueRef, GraphDataRef>("CalculatePoints")
            .iter(SystemCalculatePoints);

    ecsWorld.system<UpdateEntityPathTag, Path, GraphDataRef>("UpdatePath")
            .iter(SystemUpdatePath);

    ecsWorld.system<Path, GraphDataRef, ListeningClients>("CheckToSend")
            .iter(SystemCheckEntitiesToSend);

    ecsWorld.system<Path, ETAToNextPoint, Color, Speed, Scale,
                    GraphDataRef, ListeningClients, EntityToUpdateTag>("SendInfo")
            .iter(SystemSendEntitiesToClients);

    ecsWorld.system<GlobalEntityTag>("ReadServerMessages")
            .iter(SystemReadServerMessages);
}