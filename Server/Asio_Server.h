#pragma once

#include "../Models/MessageTypes.h"
#include "../Networking/Asio_Network.h"

#include <chrono>

using std::cout;
using std::endl;

// Struct to send quad tree data to show tree on client
//struct QuadTreeDebugData {
//    float x;
//    float y;
//    float size;
//
//    json11::Json to_json() const {
//        return json11::Json::object {
//                { "x", x },
//                { "y", y },
//                { "size", size },
//        };
//    }
//};

class Asio_Server : public server_interface<MessageTypes>
{
public:
    Asio_Server(uint16_t nPort)
            : server_interface<MessageTypes>(nPort)
    {
//        nonAuthorizedAvailableMessageTypes.emplace(MessageTypes::ClientAuthorize);
//        nonAuthorizedAvailableMessageTypes.emplace(MessageTypes::ClientDisconnected);
//        nonAuthorizedAvailableMessageTypes.emplace(MessageTypes::Ping);
//        nonAuthorizedAvailableMessageTypes.emplace(MessageTypes::QuadTreeDebug);
    }

    std::unordered_map<uint32_t, sPlayerDescription> clientsMap;
    std::vector<uint32_t> m_vGarbageIDs;

    // Returns count of connected and validated clients
    int ClientsCount() const;

protected:
    bool OnClientConnect(std::shared_ptr<connection<MessageTypes>> client) override
    {
        // For now we will allow all
        return true;
    }

    void OnClientValidated(std::shared_ptr<connection<MessageTypes>> client) override
    {
        std::cout << "Client " << client->GetID() << " validated!" << std::endl;

        // Client passed validation check, so send them a message informing
        // them they can continue to communicate
        message<MessageTypes> msg;
        msg.header.messageType = MessageTypes::ClientValidated;
        msg << client->GetID();

        // Send calidatesd response to client
        client->Send(msg);
    }

    void OnClientDisconnect(std::shared_ptr<connection<MessageTypes>> client) override
    {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        auto localtime = std::localtime(&time);

        std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                  << ": Client " << client->GetID() << " disconnected" << std::endl;

        // Remove client from quad tree
//        quadTreeRoot->RemoveClient(client->GetID());

        // Remove client from connected clients list
        auto it = clientsMap.find(client->GetID());
        if(it != clientsMap.end())
        {
            auto& pd = clientsMap[client->GetID()];
            std::cout << "[PLAYER]: " + std::to_string(pd.nUniqueID) + "\n";
            clientsMap.erase(it);
            m_vGarbageIDs.push_back(client->GetID());
        }
    }

    void OnMessage(std::shared_ptr<connection<MessageTypes>> client, message<MessageTypes>& msg) override
    {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        auto localtime = std::localtime(&time);

        switch (msg.header.messageType) {
            case MessageTypes::Ping:
            {
                client->Send(msg);
            }
                break;

            default:
            {
                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": Message received" << std::endl;
            }
                break;
        }
    }

private:

    // Pointer to root node of quad tree
//    QuadTreeNode* quadTreeRoot = nullptr;

    // Pointer to class that encapsulates all database connections
//    DbConnector* dbConnector = nullptr;

    // Set of message types that can be processed before client's authorization
    // For example: Ping, Debug messages
    std::unordered_set<MessageTypes> nonAuthorizedAvailableMessageTypes;
};