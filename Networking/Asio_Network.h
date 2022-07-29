#pragma once

#include "memory"
#include "thread"
#include "mutex"
#include "deque"
#include "optional"
#include "vector"
#include "iostream"
#include "algorithm"
#include "chrono"
#include "cstdint"
#include "unordered_map"

#include "asio-1.18.2/include/asio.hpp"
#include "asio-1.18.2/include/asio/ts/buffer.hpp"
#include "asio-1.18.2/include/asio/ts/internet.hpp"

struct sPlayerDescription
{
    uint32_t nUniqueID = 0;
    uint32_t nAvatarID = 0;
};

// Message header, which contains message type and size
template <typename T>
struct Message_Header {
    T messageType{};
    uint32_t size = 0;
};

// Struct for holding messages data
template <typename T>
struct message {
    // Order of declaration is important:
    // header first, because message starts writing from it by pointer
    // body last, because it's size can change
    Message_Header<T> header{};
    std::vector<uint8_t> body;

    size_t size() const
    {
        return body.size();
    }

    friend std::ostream& operator << (std::ostream& os, const message<T>& msg)
    {
        os << "ID: " << int(msg.header.id) << " Size: " << msg.header.size;
        return os;
    }

    // Push data into message buffer like writing to std::cout
    template<typename DataType>
    friend message<T>& operator << (message<T>& msg, const DataType& data)
    {
        // Check is data can be serialized
        static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");

        // Cache current data length, it will be an offset to new data
        size_t i = msg.body.size();

        // Resize vector to hold new data
        msg.body.resize(msg.body.size() + sizeof(DataType));

        // Copy data into new allocated memory with cached offset
        std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

        // Write new message size to it's header
        msg.header.size = msg.size();

        // Return new message so we can chain operators
        return msg;
    }

    // Read data from message buffer like reading from std::cout
    template<typename DataType>
    friend message<T>& operator >> (message<T>& msg, DataType& data)
    {
        // Check is data can be serialized
        static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pulled from vector");

        // Cache size of vector without extracting data
        size_t i = msg.body.size() - sizeof(DataType);

        // Copy data from buffer to data memory
        std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

        // Reduce vector length
        msg.body.resize(i);

        // Recalculate the message size
        msg.header.size = msg.size();

        // Return new message so we can chain operators
        return msg;
    }
};

template <typename T>
class connection;

template <typename T>
struct OwnedMessage
{
    std::shared_ptr<connection<T>> remote = nullptr;
    message<T> msg;

    // Again, a friendly string maker
    friend std::ostream& operator << (std::ostream& os, const OwnedMessage<T>& msg)
    {
        os << msg.msg;
        return os;
    }
};

// Simple wrapper to std::deque, with mutex, so it becomes thread-safe
template <typename T>
class tsqueue
{
public:
    tsqueue() = default;
    tsqueue(const tsqueue<T>&) = delete;
    virtual ~tsqueue() { clear(); }

public:
    const T& front()
    {
        std::lock_guard<std::mutex> lock(muxQueue);
        return deqQueue.front();
    }

    const T& back()
    {
        std::lock_guard<std::mutex> lock(muxQueue);
        return deqQueue.back();
    }

    T pop_front()
    {
        std::lock_guard<std::mutex> lock(muxQueue);

        // Cache front element, because pop_front returns no data
        auto t = std::move(deqQueue.front());
        deqQueue.pop_front();
        return t;
    }

    T pop_back()
    {
        std::lock_guard<std::mutex> lock(muxQueue);

        // Cache last element, because pop_back returns no data
        auto t = std::move(deqQueue.back());
        deqQueue.pop_back();
        return t;
    }

    void push_back(const T& item)
    {
        std::lock_guard<std::mutex> lock(muxQueue);
        deqQueue.template emplace_back(std::move(item));

        std::unique_lock<std::mutex> ul(muxBlocking);
        cvBlocking.notify_one();
    }

    void push_front(const T& item)
    {
        std::lock_guard<std::mutex> lock(muxQueue);
        deqQueue.emplace_front(std::move(item));

        std::unique_lock<std::mutex> ul(muxBlocking);
        cvBlocking.notify_one();
    }

    bool empty()
    {
        std::lock_guard<std::mutex> lock(muxQueue);
        return deqQueue.empty();
    }

    size_t count()
    {
        std::lock_guard<std::mutex> lock(muxQueue);
        return deqQueue.size();
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(muxQueue);
        deqQueue.clear();
    }

    void wait()
    {
        while(empty())
        {
            std::unique_lock<std::mutex> ul(muxBlocking);
            cvBlocking.wait(ul);
        }
    }

protected:
    std::mutex muxQueue;
    std::deque<T> deqQueue;
    std::condition_variable cvBlocking;
    std::mutex muxBlocking;
};

// Forward declaration of server interface
template<typename T>
class server_interface;

// Struct that matches specific client with its socket and messages queue
// Handles all networking async stuff
template<typename T>
class connection : public std::enable_shared_from_this<connection<T>>
{
public:
    connection(asio::io_context& asioContext, asio::ip::tcp::socket socket, tsqueue<OwnedMessage<T>>& qIn)
            : m_asioContext(asioContext), m_socket(std::move(socket)), m_qMessagesIn(qIn)
    {
        // Connection is Server -> Client, construct random data for the client
        // to transform and send back for validation
        m_nHandshakeOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());

        m_nHandshakeCheck = scramble(m_nHandshakeOut);
    }

    virtual ~connection()
    {}

    uint32_t GetID() const
    {
        return id;
    }
public:

    // When client is connected, we need to validate it
    // So, send validation data to it and wait the answer
    void ConnectionToClient(server_interface<T>* server, uint32_t uid = 0)
    {
        if(m_socket.is_open())
        {
            id = uid;

            // A client has attempted to connect to the server, but we wish
            // the client to first validate itself, so first write out the
            // handshake data to be validated
            WriteValidation();

            // Next, issue a task to sit and wait async for precisely
            // the validation data sent back from the client
            ReadValidation(server);
        }
    }

    void Disconnect()
    {
        // Put a close socket task to asio
        // It will be performed, when can handle it
        if(IsConnected())
            asio::post(m_asioContext, [this]() { m_socket.close(); });
    }

    bool IsConnected() const
    {
        return m_socket.is_open();
    }

    // Prime the connection to wait for incoming messages
    void StartListening()
    {

    }

public:

    // ASYNC - Send a message, connections are one-to-one so no need to specifiy
    // the target, for a client, the target is the server and vice versa
    void Send(const message<T>& msg)
    {
        asio::post(m_asioContext,
                   [this, msg]()
                   {
                       // First, check if out queue is empty
                       bool bWritingMessage = !m_qMessagesOut.empty();

                       // Put a new message to queue
                       m_qMessagesOut.push_back(msg);

                       // If queue wasn't empty, it means that write task is already performing
                       // So, we have to register a new one only if there wasn't any writing messages
                       if(!bWritingMessage)
                       {
                           WriteHeader();
                       }
                   });
    }

private:
    // ASYNC - Prime context to write a message header
    void WriteHeader()
    {
        // If this function is called, we know the outgoing message queue must have
        // at least one message to send. So allocate a transmission buffer to hold
        // the message, and issue the work - asio, send these bytes
        asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.front().header, sizeof(Message_Header<T>)),
                          [this](std::error_code ec, std::size_t length)
                          {
                              // asio has now sent the bytes - if there was a problem
                              // an error would be available...
                              if(!ec)
                              {
                                  // ... no error, so check if the message header just sent also
                                  // has a message body...
                                  if(m_qMessagesOut.front().body.size() > 0)
                                  {
                                      // ...it does, so issue the task to write the body bytes
                                      WriteBody();
                                  }
                                  else
                                  {
                                      // ...it didn't, so we are done with this message. Remove it from
                                      // the outgoing message queue
                                      m_qMessagesOut.pop_front();

                                      // If the queue is not empty, there are more messages to send, so
                                      // make this happen by issuing the task to send next header
                                      if(!m_qMessagesOut.empty())
                                      {
                                          WriteHeader();
                                      }
                                  }
                              }
                              else
                              {
                                  auto now = std::chrono::system_clock::now();
                                  std::time_t time = std::chrono::system_clock::to_time_t(now);
                                  auto localtime = std::localtime(&time);

                                  // ...asio failed to write the message, we could analyse why but
                                  // for now simply assume the connection has died by closing the
                                  // socket. When a future attempt to write to this client fails due
                                  // to the closed socket, it will be tidied up.
                                  std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                                            << ": [" << id << "] Write Header Fail.\n";
                                  m_socket.close();
                              }
                          });
    }

    // ASYNC - Prime context to write a message body
    void WriteBody()
    {
        // If this function is called, a header has just been sent, and that header
        // indicated a body existed for this message. Fill a transmission buffer
        // with the body data, and sent it!
        asio::async_write(m_socket, asio::buffer(m_qMessagesOut.front().body.data(), m_qMessagesOut.front().body.size()),
                          [this](std::error_code ec, std::size_t length)
                          {
                              if(!ec)
                              {
                                  // Sending was successful, so we are done with the message
                                  // and remove it from the queue
                                  m_qMessagesOut.pop_front();

                                  // If the queue still has messages in it, then issue the task to
                                  // send the next message's header
                                  if(!m_qMessagesOut.empty())
                                  {
                                      WriteHeader();
                                  }
                              }
                              else
                              {
                                  // Sending failed, see WriteHeader() equivalent for description
                                  auto now = std::chrono::system_clock::now();
                                  std::time_t time = std::chrono::system_clock::to_time_t(now);
                                  auto localtime = std::localtime(&time);

                                  std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                                            << ": [" << id << "] Write Body Fail.\n";
                                  m_socket.close();
                              }
                          });
    }

    // ASYNC - Prime context ready to read a message header
    void ReadHeader()
    {
        // If this function is called, we are expecting asio to wait until it receives
        // enough bytes to form a header ot a message. We know the headers are a fixed
        // size, so allocate a transmission buffer large enough to store it. In fact,
        // we will construct the message in a "temporary" message object as it's
        // convenient to work with
        asio::async_read(m_socket, asio::buffer(&m_msgTemporaryIn.header, sizeof(Message_Header<T>)),
                         [this](std::error_code ec, std::size_t length)
                         {
                             if(!ec)
                             {
                                 // A complete message header has been read, check if this message
                                 // has a body to follow...
                                 if(m_msgTemporaryIn.header.size > 0)
                                 {
                                     // ...It does, so allocate enough space in the message's body
                                     // vector, and issue asio with the task to read the body.
                                     m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
                                     ReadBody();
                                 }
                                 else
                                 {
                                     // It doesn't, so add this bodyless message to the connections
                                     // incoming message queue
                                     AddToIncomingMessageQueue();
                                 }
                             }
                             else
                             {
                                 // Reading form the client went wrong, most likely a disconnect
                                 // has occurred. Close the socket and let the system tidy it up later.
                                 auto now = std::chrono::system_clock::now();
                                 std::time_t time = std::chrono::system_clock::to_time_t(now);
                                 auto localtime = std::localtime(&time);

                                 std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                                           << ": [" << id << "] Read Header Fail.\n";
                                 m_socket.close();
                             }
                         });
    }

    void ReadBody()
    {
        // If this function is called, a header has already been read, and that header
        // request we read a body. The space for that body has already been allocated
        // in the temporary message object, so just wait for the bytes to arrive...
        asio::async_read(m_socket, asio::buffer(m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
                         [this](std::error_code ec, std::size_t length)
                         {
                             if(!ec)
                             {
                                 // ...and they have! The message is now complete, so add
                                 // the whole message to incoming queue
                                 AddToIncomingMessageQueue();
                             }
                             else
                             {
                                 // As above!
                                 auto now = std::chrono::system_clock::now();
                                 std::time_t time = std::chrono::system_clock::to_time_t(now);
                                 auto localtime = std::localtime(&time);

                                 std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                                           << ": [" << id << "] Read Body Fail.\n";
                                 m_socket.close();
                             }
                         });
    }

    // "Encrypt" data
    uint64_t scramble(uint64_t nInput)
    {
        uint64_t out = nInput ^ 0xDEADBEEFC0DECAFE;
        out = (out & 0xF0F0F0F0F0F0F0) >> 4 | (out & 0x0F0F0F0F0F0F0F) << 4;
        return out ^ 0xC0DEFACE12345678;
    }

    // ASYNC - Used by server to write validation packet
    void WriteValidation()
    {
        asio::async_write(m_socket, asio::buffer(&m_nHandshakeOut, sizeof(uint64_t)),
                          [this](std::error_code ec, std::size_t length)
                          {
                              auto now = std::chrono::system_clock::now();
                              std::time_t time = std::chrono::system_clock::to_time_t(now);
                              auto localtime = std::localtime(&time);
                              if(!ec)
                              {
                                  std::cout << localtime->tm_hour << ":" << localtime->tm_min
                                            << ":" << localtime->tm_sec << ": Validation data sent, clients should sit and wait\n";
                              }
                              else
                              {
                                  std::cout << localtime->tm_hour << ":" << localtime->tm_min
                                            << ":" << localtime->tm_sec << ": Write validation error\n";
                                  m_socket.close();
                              }
                          });
    }

    void ReadValidation(server_interface<T>* server)
    {
        asio::async_read(m_socket, asio::buffer(&m_nHandshakeIn, sizeof(uint64_t)),
                         [this, server](std::error_code ec, std::size_t length)
                         {
                             auto now = std::chrono::system_clock::now();
                             std::time_t time = std::chrono::system_clock::to_time_t(now);
                             auto localtime = std::localtime(&time);

                             if(!ec)
                             {
                                 // Connection is a server, so check response from client

                                 // Compare sent data to actual solution
                                 if(m_nHandshakeIn == m_nHandshakeCheck)
                                 {
                                     // Client has provided valid solution, so allow it to connect properly
                                     std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                                               << ": Client Validated" << std::endl;

                                     server->OnClientValidated(this->shared_from_this());

                                     // Sit waiting to receive data now
                                     ReadHeader();
                                 }
                                 else
                                 {
                                     // Client gave incorrect data, so disconnect
                                     std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                                               << ": Client Disconnected (Fail validation)" << std::endl;
                                     m_socket.close();
                                 }
                             }
                             else
                             {
                                 // Some biggerfailure occured
                                 std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                                           << ": Client Disconnected (Read validation)" << std::endl;
                                 m_socket.close();
                             }
                         });
    }

    // Once a full message is received, add it to the incoming queue
    void AddToIncomingMessageQueue()
    {
        // Shove it in queue, converting it to an "owned message", by initialising
        // with the a shared pointer from this connection object
        m_qMessagesIn.push_back({ this->shared_from_this(), m_msgTemporaryIn });

        // We must now pride the asio context to receive the next message. It
        // will just sit and wait for bytes to arrive, and the message construction
        // process repeats itself.
        ReadHeader();
    }

protected:
    // Each connection has a unique socket
    asio::ip::tcp::socket m_socket;

    // Asio context is shared between all connections
    asio::io_context& m_asioContext;

    // This queue holds all messages to be sent to the remote side
    // of this connection
    tsqueue<message<T>> m_qMessagesOut;

    // Incoming messages queue. Queue owned by server,
    // so connection only has reference to it
    tsqueue<OwnedMessage<T>>& m_qMessagesIn;

    // Temp buffer for reading messages
    message<T> m_msgTemporaryIn;

    // Handshake Validation
    uint64_t m_nHandshakeOut = 0;
    uint64_t m_nHandshakeIn = 0;
    uint64_t m_nHandshakeCheck = 0;

    bool m_bValidHandshake = false;
    bool m_bConnectionEstablished = false;

    // Unique connection ID, so server can recognize clients
    uint32_t id = 0;
};

// Server
template<typename T>
class server_interface
{
public:
    // Create a server, ready to listen on specified port
    server_interface(uint64_t port)
            : m_asioAcceptor(m_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
    {

    }

    virtual ~server_interface()
    {
        // May as well try and tidy up
        Stop();
    }

    // Starts the server
    bool Start()
    {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        auto localtime = std::localtime(&time);

        try
        {
            // Add asio task to wait a client connection in async mode
            WaitForClientConnection();

            // Launch the asio context in its own thread
            m_threadContext = std::thread([this]() { m_asioContext.run(); });
        }
        catch(std::exception& e)
        {
            // Something prohibited the server from listening
            std::cerr << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                      << ": [SERVER] Exception: " << e.what() << "\n";
            return false;
        }

        std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                  << ": [SERVER] started!" << std::endl;
        return true;
    }

    // Stops the server
    void Stop()
    {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        auto localtime = std::localtime(&time);

        // Request the context to close
        m_asioContext.stop();

        // Tidy up the context thread
        if(m_threadContext.joinable()) m_threadContext.join();

        // Inform someone, anybody, if they care...
        std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                  << ": [SERVER] Stopped!" << std::endl;
    }

    // ASYNC - Instruct asio to wait for connection
    void WaitForClientConnection()
    {
        // Prime context with an instruction to wait until a socket connects. This
        // is the purpose of an "acceptor" object. It will provide a unique socket
        // for each incoming connection attempt
        m_asioAcceptor.async_accept(
                [this](std::error_code ec, asio::ip::tcp::socket socket)
                {
                    auto now = std::chrono::system_clock::now();
                    std::time_t time = std::chrono::system_clock::to_time_t(now);
                    auto localtime = std::localtime(&time);

                    // Triggered by incoming connection request
                    if(!ec)
                    {
                        // Display some useful information
                        std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                                  << ":[SERVER] New Connection!: " << socket.remote_endpoint() << std::endl;

                        // Create a new connection to handle this client
                        std::shared_ptr<connection<T>> newconn =
                                std::make_shared<connection<T>>(m_asioContext, std::move(socket), m_qMessagesIn);

                        // Give the user server a chance to deny connection
                        if(OnClientConnect(newconn))
                        {
                            // Connection allowed, so add to container of new connections
                            m_deqConnections.push_back(std::move(newconn));

                            // And very important! Issue a task to the connection's
                            // asio context to sit and wait for bytes to arrive!
                            m_deqConnections.back()->ConnectionToClient(this, nIDCounter++);

                            std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                                      << ":[" << m_deqConnections.back()->GetID() << "] Connection Approved" << std::endl;
                        }
                        else
                        {
                            std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                                      << ":[-----] Connection Denied" << std::endl;
                        }
                    }
                    else
                    {
                        std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                                  << ": [SERVER] New Connection Error: " << ec.message() << std::endl;
                    }

                    // Prime the asio context with more work - again simply wait for
                    // another connection...
                    WaitForClientConnection();
                });
    }

    // Send a message to a specific client
    void MessageClient(std::shared_ptr<connection<T>> client, const message<T>& msg)
    {
        // Check client is legitimate...
        if(client && client->IsConnected())
        {
            // ...and post the message via the connection
            client->Send(msg);
        }
        else
        {
            // If we can't communicate with client then we may as
            // well remove the client - let the server know, it may
            // be tracking it somehow
            OnClientDisconnect(client);

            // Client disconnected
            client.reset();

            // Then physically remove it from the container
            m_deqConnections.erase(
                    std::remove(m_deqConnections.begin(), m_deqConnections.end(), client), m_deqConnections.end());
        }
    }

    // Send message to all clients
    void MessageAllClients(const message<T>& msg, std::shared_ptr<connection<T>> pIgnoreClient = nullptr)
    {
        bool bInvalidClientExists = false;

        // Iterate through all clients in container
        for(auto& client : m_deqConnections)
        {
            // Check client is connected...
            if(client && client->IsConnected())
            {
                // ...it is
                if(client != pIgnoreClient)
                    client->Send(msg);
            }
            else
            {
                // The client couldn't be contacted, so assume it has
                // disconnected.
                OnClientDisconnect(client);

                client.reset();

                // Set this flag to then remove dead clients form container
                bInvalidClientExists = true;
            }
        }

        // Remove dead clients, all in one go - this way, we don't invalidate the
        // container as we iterated through it.
        if(bInvalidClientExists)
            m_deqConnections.erase(
                    std::remove(m_deqConnections.begin(), m_deqConnections.end(), nullptr), m_deqConnections.end());
    }

    // Force server to respond to incoming messages
    void Update(size_t nMaxMessages = -1, bool bWait = false)
    {
        if(bWait) m_qMessagesIn.wait();

        // Process as many messages as you can up to the value
        // specified
        size_t nMessageCount = 0;
        while(nMessageCount < nMaxMessages && !m_qMessagesIn.empty())
        {
            // Grab the front message
            auto msg = m_qMessagesIn.pop_front();

            // Pass to message handler
            OnMessage(msg.remote, msg.msg);

            nMessageCount++;
        }
    }

protected:
    // This server class should override these function to implement
    // customised functionality

    // Called when a client connects, you can veto the connection by returning false
    virtual bool OnClientConnect(std::shared_ptr<connection<T>> client)
    {
        return false;
    }

    // Called when a client appears to have disconnected
    virtual void OnClientDisconnect(std::shared_ptr<connection<T>> client)
    {

    }

    // Called when a message arrives
    virtual void OnMessage(std::shared_ptr<connection<T>> client, message<T>& msg)
    {

    }

public:
    // Called when a client is validated
    virtual void OnClientValidated(std::shared_ptr<connection<T>> client)
    {

    }

protected:
    // Thread Safe Queue for incoming message packets
    tsqueue<OwnedMessage<T>> m_qMessagesIn;

    // Container of active validated connections
    std::deque<std::shared_ptr<connection<T>>> m_deqConnections;

    // Order of declaration is important - it is also the order of initialisation
    asio::io_context m_asioContext;
    std::thread m_threadContext;

    // These things need an asio context
    asio::ip::tcp::acceptor m_asioAcceptor; // Handles new incoming connection attempts...

    // Clients will be identified in the "wider system" via an ID
    uint32_t nIDCounter = 10000;
};
