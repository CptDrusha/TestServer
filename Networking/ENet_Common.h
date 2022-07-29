#pragma once

#include <enet/enet.h>

#include "chrono"
#include "../Serial/Serialization.h"
#include "../Models/MessageTypes.h"

#include <iostream>
#include <string.h>
#include <vector>
#include <cstdint>
#include <map>

template<typename T>
struct PacketHeader {
    T packetType{};
    uint32_t size = 0;
};

struct PingShare
{
    std::chrono::time_point<std::chrono::high_resolution_clock> time;
};

struct gamepackage
{
public:
    explicit gamepackage() = default;
    ~gamepackage() = default;

    /*gamepackage(size_t size = 1024) : read_pos(0)
    {
        body.reserve(size);
    }*/

    gamepackage(const enet_uint8* data, size_t size) : read_pos(0), write_pos(0)
    {
        body.insert(body.end(), data, data + size);
    }

    gamepackage(const char* data, size_t size) : read_pos(0), write_pos(0)
    {
        body.reserve(size);
        body.assign(data, data + size);
    }

    const size_t size() const
    {
        return body.size();
    }

    std::vector<char>& buffer()
    {
        return body;
    }

    char* data()
    {
        return body.data();
    }

    unsigned long long int length()
    {
        return body.size();
    }

    template<typename DataType>
    void write(const DataType& data)
    {
        auto size = sizeof(data);

        auto as_char = reinterpret_cast<const char*>(&data);

        body.insert(body.end(), as_char, as_char + size);

        write_pos += size;
    }

    void write(const std::string& str)
    {
        auto data = str.data();

        auto length = str.length() + 1;

        body.insert(body.end(), data, data + length);
    }

    template<typename DataType>
    gamepackage& operator << (const DataType& data)
    {
        write(data);

        return *this;
    }

    template<typename DataType>
    bool read(DataType& data)
    {
        auto size = sizeof(DataType);

        auto as_type = reinterpret_cast<DataType*>(&body[read_pos]);

        read_pos += size;

        data = *as_type;

        return true;
    }

    bool read(std::string& str)
    {
        size_t length;

        length = strlen(&body[read_pos]);

        str = std::string(&body[read_pos], length);

        read_pos += str.length() + 1;

        return true;
    }

    template<typename DataType>
    gamepackage& operator >> (DataType& data)
    {
        read(data);

        return *this;
    }

    std::vector<char> body;
    size_t write_pos = 0;
    size_t read_pos = 0;
};

template <typename T>
struct Msg
{
    Msg()
    {
        GenerateID();
    }

    Msg(T type, gamepackage package) : type(type), package(package)
    {
        GenerateID();
    }

    void GenerateID()
    {
        id = GetID();
    }

    void clearData()
    {
        package.buffer().clear();
    }

    std::vector<char> serialize()
    {
        gamepackage header;

        header << id;
        header << type;

        return merge(header, package).buffer();
    }

    std::string GetString()
    {
        std::string data(package.buffer().begin(), package.buffer().end());
        return data;
    }

    void deserialize(gamepackage stream)
    {
        stream >> id;
        stream >> type;
        //_package = stream;
        package.buffer().insert(package.buffer().end(), stream.buffer().begin() + stream.read_pos, stream.buffer().end());
    }

    gamepackage merge(gamepackage a, gamepackage b)
    {
        std::vector<char> abuff;
        std::vector<char> bbuff;
        uint32_t size = 0;
        abuff = a.buffer();
        size += abuff.size();
        bbuff = b.buffer();
        size += bbuff.size();

        auto bytes = std::vector<char>();
        bytes.reserve(size);

        bytes.insert(bytes.end(), abuff.begin(), abuff.end());
        bytes.insert(bytes.end(), bbuff.begin(), bbuff.end());

        return *new gamepackage(&bytes[0], bytes.size());
    }

    uint32_t id;
    T type;

    gamepackage package;

private:
    static uint32_t GetID()
    {
        static uint32_t Counter = 0;
        return ++Counter;
    }
};

class server_client
{
public:
    server_client(unsigned int id) : client_index(id) {}

    unsigned int get_id() const { return _uid; }

    std::string get_username() const { return _username; }

    int get_client_index() const { return client_index; }

    void set_username(std::string username) { _username = username; }

    void set_uid(unsigned int uid) { _uid = uid; }

private:
    unsigned int _uid;
    std::string _username;
    int client_index = -1;
};

class server_interface
{
public:
    server_interface(uint16_t _port) : port(_port) {}

    virtual ~server_interface()
    {
        Stop();
    }

    bool Start()
    {
        auto now = std::chrono::system_clock::now();
        time_t time = std::chrono::system_clock::to_time_t(now);
        auto localtime = std::localtime(&time);

        if(enet_initialize() != 0)
        {
            std::cerr << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                << ": [SERVER] An Error occurred while initializing ENet.\n";

            Stop();
        }

        address.host = ENET_HOST_ANY;
        address.port = port;

        server = enet_host_create(
                &address,
                32,
                1,
                0,
                0);

        if(server == NULL)
        {
            std::cerr << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                << ": [SERVER] Exception: An Error occurred while trying to create an ENet server host.\n";
            Stop();
            return false;
        }

        std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
            << ": [SERVER] started!" << std::endl;

        return true;
    }

    void Stop()
    {
        auto now = std::chrono::system_clock::now();
        time_t time = std::chrono::system_clock::to_time_t(now);
        auto localtime = std::localtime(&time);

        enet_deinitialize();

        enet_host_destroy(server);

        std::cerr << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
            << ": [SERVER] Stopped!" << std::endl;
    }

    void Update() {
        if (enet_host_service(server, &event, 1000) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    OnClientConnect(event.peer);
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT: {
                    OnClientDisconnect(event.peer);
                    break;
                }
                case ENET_EVENT_TYPE_RECEIVE:
                {
                    OnMessage(event);

                    enet_packet_destroy(event.packet);
                    break;
                }
            }
        }
    }

protected:

    virtual bool OnClientConnect(ENetPeer* peer)
    {
        return false;
    }

    virtual void OnClientDisconnect(ENetPeer* peer)
    {

    }

    virtual void OnMessage(ENetEvent event)
    {

    }

    virtual void SendMessage(ENetPeer* peer, Msg<MessageTypes>& msg)
    {

    }

    virtual void SendBroadcast(ENetHost* host, Msg<MessageTypes>& package)
    {

    }

public:

    virtual void OnClientValidated(ENetPeer* peer)
    {

    }

protected:

    uint16_t port;

    ENetAddress address;
    ENetHost* server;
    ENetEvent event;

    std::map<uint32_t, ENetPeer*> clients_;

    uint32_t nIDCounter = 0;
};