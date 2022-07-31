//
// Created by dml on 01.02.2022.
//
#pragma once
#include <unordered_map>

#include "../Networking/ENet_Common.h"
#include "../Models/MessageTypes.h"
#include "../Models/ModelsCommon.h"
#include "../Db/DbConnector.h"

class ENetServer : public server_interface {
public:
    ENetServer(uint16_t _port, DbConnector* connector)
        : server_interface(_port), dbConnector(connector)
    {
        nonAuthorizedAvailableMessageTypes.emplace(MessageTypes::ClientAuthorize);
        nonAuthorizedAvailableMessageTypes.emplace(MessageTypes::ClientDisconnected);
        nonAuthorizedAvailableMessageTypes.emplace(MessageTypes::QuadTreeDebug);
        nonAuthorizedAvailableMessageTypes.emplace(MessageTypes::Ping);
        nonAuthorizedAvailableMessageTypes.emplace(MessageTypes::ClientRegistration);
    }

    bool Start();

    void Update();

protected:

    bool OnClientConnect(ENetPeer* peer) override;

    void OnClientValidated(ENetPeer* peer) override;

    void OnClientDisconnect(ENetPeer* peer) override;

    void OnMessage(ENetEvent event) override;

    void SendMessage(ENetPeer* peer, Msg<MessageTypes>& msg) override;

    void SendBroadcast(ENetHost* peer, Msg<MessageTypes>& msg) override;

    // Reusable methods

    // Used for both breaking and resetting status
    void StopConversion(ConversionInfo Info, bool bSkipSearch = false);

private:

    std::unordered_set<MessageTypes> nonAuthorizedAvailableMessageTypes;
    std::unordered_map<int, server_client*> AuthorizedClientMap;

    std::vector<long int> TimeList;
    std::vector<ConversionInfo> ConversionInfoList;

    DbConnector* dbConnector = nullptr;

    Msg<MessageTypes> MsgPosition;
    std::unordered_map<int, PlayerMovement> Movements;

    Msg<MessageTypes> MsgBroadcast;

    float curr_time = 0.f;
    float old_time = 0.f;
    float send_time = 100.f;
};
