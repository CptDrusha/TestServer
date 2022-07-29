//
// Created by dml on 01.02.2022.
//

#include "ENetServer.h"
#include "../Serial/Timer.h"

void ENetServer::OnClientValidated(ENetPeer *peer) {
    std::cout << "Client " << peer->host << " validated!" << std::endl;

    gamepackage package;
    package << MessageTypes::ClientValidated;

    ENetPacket* packet = enet_packet_create(
            &package.data()[0],
            package.size(),
            ENET_PACKET_FLAG_RELIABLE);

    enet_peer_send(peer, 0, packet);
}

bool ENetServer::OnClientConnect(ENetPeer *peer) {
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    auto localtime = std::localtime(&time);

    auto client = new server_client(nIDCounter);
    event.peer->data = client;

    clients_[nIDCounter] = event.peer;

    std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
              << ": [SERVER] Client " << client->get_client_index() << " connected!\n";

    nIDCounter++;

    return true;
}

void ENetServer::OnClientDisconnect(ENetPeer *peer) {
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    auto localtime = std::localtime(&time);

    server_client client = *static_cast<server_client*>(peer->data);

    std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
              << ": Client " << client.get_client_index() << " disconnected" << std::endl;

    if(AuthorizedClientMap.find(client.get_id()) != AuthorizedClientMap.end())
    {
        int characterID = client.get_id();

        ProductInCharacter product;
        if(dbConnector->GetProductInCharacterById(characterID, product))
            dbConnector->DeleteSpaceActorByPIID(product.PIID);
    }

    auto it = clients_.find(client.get_client_index());
    if(it != clients_.end())
    {
        clients_.erase(it);
    }

    AuthorizedClientMap.erase(client.get_id());

    Msg<MessageTypes> msg;
    msg.type = MessageTypes::ClientDisconnected;
    msg.package << client.get_id();

    SendBroadcast(server, msg);
}

void ENetServer::SendMessage(ENetPeer *peer, Msg<MessageTypes>& msg) {
    auto data = msg.serialize();

    ENetPacket * packet = enet_packet_create(
            &data.data()[0],
            data.size(),
            ENET_PACKET_FLAG_RELIABLE);

    enet_peer_send(peer, 0, packet);
}

void ENetServer::SendBroadcast(ENetHost *host, Msg<MessageTypes> &msg) {
    auto data = msg.serialize();

    ENetPacket* packet = enet_packet_create(
            &data.data()[0],
            data.size(),
            ENET_PACKET_FLAG_RELIABLE);

    enet_host_broadcast(host, 0, packet);
}

void ENetServer::OnMessage(ENetEvent event) {
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    tm* localtime = std::localtime(&time);

    gamepackage package(
            event.packet->data,
            event.packet->dataLength);

    Msg<MessageTypes> msg;
    msg.deserialize(package);

    if(nonAuthorizedAvailableMessageTypes.find(msg.type) != nonAuthorizedAvailableMessageTypes.end())
    {

    }

    switch (msg.type) {
        case(MessageTypes::Ping):
        {
            std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                      << ": [SERVER] Message ping.\n";

            SendMessage(event.peer, msg);
            break;
        }
        case(MessageTypes::ClientRegistration):
        {
            std::string userName;
            msg.package >> userName;
            uint64_t hashedPassword = 0;
            msg.package >> hashedPassword;

            msg.clearData();

            int id = dbConnector->RegistrationClient(userName, hashedPassword);
            msg.package << id;

            SendMessage(event.peer, msg);
            break;
        }
        case(MessageTypes::ClientAuthorize):
        {
            std::string userName;
            msg.package >> userName;
            uint64_t hashedPassword = 0;
            msg.package >> hashedPassword;

            msg.clearData();

            int characterID = dbConnector->CheckAuthData(userName, hashedPassword);
            if(characterID > -1)
            {
                server_client* client = static_cast<server_client*>(event.peer->data);

                client->set_uid(characterID);

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": Client " << client->get_id() << " authorized!" << std::endl;

                ProductInCharacter productInCharacter;
                if(dbConnector->GetProductInCharacterById(characterID, productInCharacter))
                {
                    SpaceActor PlayerSpaceActor(-1, 0, "0, 0, 0", "0, 0, 0", productInCharacter.PIID, -1);

                    dbConnector->UpdateSpaceActor(PlayerSpaceActor);
                }

                client->set_username(userName);
                AuthorizedClientMap.emplace(characterID, client);

                msg.package << characterID;

                SendMessage(event.peer, msg);

                msg.type = MessageTypes::JoinPlayer;

                SendBroadcast(server, msg);
            } else
            {
                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": Client " << static_cast<server_client*>(event.peer->data)->get_id() << " authorize rejected" << std::endl;

                msg.package << characterID;
                msg.type = MessageTypes::ClientAuthorizeDecline;

                SendMessage(event.peer, msg);
            }
            break;
        }
    }

    server_client* client = static_cast<server_client*>(event.peer->data);
//    if(AuthorizedClientMap.find(client->get_id()) == AuthorizedClientMap.end())
//        return;

    switch(msg.type) {
        case MessageTypes::JoinPlayer:
        {

            break;
        }

        case MessageTypes::ClientPosition:
        {
            std::string parseError;
            std::string Msg;

            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            PlayerMovement position;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                position = PlayerMovement(JSON);

                position.CharacterID = client->get_id();

                if(Movements.find(position.CharacterID) != Movements.end())
                    Movements[position.CharacterID] = position;
                else
                    Movements.emplace(position.CharacterID, position);
            }
            break;
        }

        case MessageTypes::DbGetAllAuthUsers:
        {
            std::vector<Character> characterIDs;
            characterIDs.reserve(AuthorizedClientMap.size());
            for(auto charID : AuthorizedClientMap)
            {
                if(charID.first != client->get_id())
                {
                    Character character;
                    dbConnector->GetCharacterByID(charID.first, character);
                    characterIDs.push_back(character);
                }
            }

            std::string jsonString = json11::Json(characterIDs).dump();

            msg.clearData();
            msg.package << jsonString;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbGetAllCharacterBodyTypes:
        {
            std::vector<CharacterBodyType> bodyTypes;

            if(dbConnector->GetAllCharacterBodyTypes(bodyTypes))
            {
                std::string jsonString = json11::Json(bodyTypes).dump();

                msg.package << jsonString;

                SendMessage(event.peer, msg);
            }
            break;
        }

        case MessageTypes::DbDeleteCharacterBodyType:
        {
            uint32_t id;
            msg.package >> id;

            bool result = dbConnector->DeleteCharacterBodyType(id);

            msg.clearData();
            msg.package << result;

            SendMessage(event.peer, msg);
            break;
        }

        case MessageTypes::DbGetCharacterBodyType:
        {
            uint32_t id;
            msg.package >> id;

            CharacterBodyType bodyType;
            if(dbConnector->GetCharacterBodyType(id, bodyType))
            {
                std::string jsonString = json11::Json(bodyType).dump();

                msg.clearData();
                msg.package << jsonString;

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbUpdateCharacterBodyType:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            CharacterBodyType bodyType;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                bodyType = CharacterBodyType(JSON);
                bodyType.CharacterID = dbConnector->UpdateCharacterBodyType(bodyType);
            }

            std::string jsonString = json11::Json(bodyType).dump();

            msg.clearData();
            msg.package << jsonString;

            SendMessage(event.peer, msg);
            break;
        }
    }


    switch (msg.type) {

        case MessageTypes::DbGetProductArchetypes:
        {
            std::vector<ProductArchetype> archetypes;

            if(dbConnector->GetAllArchetypes(archetypes))
            {
                std::string jsonString = json11::Json(archetypes).dump();

                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending archetypes...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetProductArchetypeById:
        {
            uint32_t id;
            msg.package >> id;

            ProductArchetype archetype;
            if(dbConnector->GetArchetypeById(id, archetype))
            {
                std::string jsonString = json11::Json(archetype).dump();

                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending archetype by ID\n";

                SendMessage(event.peer, msg);
            }
            break;
        }

        case MessageTypes::DbGetProductArchetypeByName:
        {
            std::string name;
            msg.package >> name;

            ProductArchetype archetype;
            if(dbConnector->GetArchetypeByName(name, archetype))
            {
                std::string jsonString = json11::Json(archetype).dump();

                msg.clearData();
                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending archetype by Name\n";

                SendMessage(event.peer, msg);
            }
            break;
        }

        case MessageTypes::DbUpdateProductArchetype:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            ProductArchetype archetype;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                archetype = ProductArchetype(JSON);
                archetype.PAID = dbConnector->UpdateArchetype(archetype);

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Update archetype | New PAID: " << archetype.PAID << "\n";
            }

            std::string jsonString = json11::Json(archetype).dump();

            msg.clearData();
            msg.package << jsonString;

            SendMessage(event.peer, msg);
            break;
        }

        case MessageTypes::DbDeleteProductArchetype:
        {
            uint32_t id;
            msg.package >> id;

            bool result = dbConnector->DeleteArchetype(id);

            msg.clearData();
            msg.package << result;

            SendMessage(event.peer, msg);
            break;
        }

        case MessageTypes::DbGetContainerInstances:
        {
            std::vector<ContainerInstance> containerInstances;

            if(dbConnector->GetAllContainerInstances(containerInstances))
            {
                std::string jsonString = json11::Json(containerInstances).dump();

                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending instances...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetContainerInstanceBySID:
        {
            uint32_t id;
            msg.package >> id;

            std::vector<ProductInstance> products;
            if(dbConnector->GetProductInstancesBySID(id, products))
            {
                std::vector<ContainerInstance> instances;
                if(dbConnector->GetContainerInstancesByPIIDs(products, instances))
                {
                    std::string jsonString = json11::Json(instances).dump();

                    msg.clearData();
                    msg.package << jsonString;

                    std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                              << ": [SERVER] Sending instances by SID...\n";

                    SendMessage(event.peer, msg);
                }
            }

            break;
        }

        case MessageTypes::DbGetContainerInstanceById:
        {
            uint32_t id;
            msg.package >> id;

            ContainerInstance containerInstance;
            if(dbConnector->GetContainerInstanceById(id, containerInstance))
            {
                std::string jsonString = json11::Json(containerInstance).dump();

                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending instance by ID...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbUpdateContainerInstance:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            ContainerInstance containerInstance;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                containerInstance = ContainerInstance(JSON);
                containerInstance.CIID = dbConnector->UpdateContainerInstance(containerInstance);

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Update instance by ID...\n";
            }

            std::string jsonString = json11::Json(containerInstance).dump();

            msg.clearData();
            msg.package << jsonString;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbDeleteContainerInstance:
        {
            uint32_t id;
            msg.package >> id;

            bool result = dbConnector->DeleteContainerInstance(id);

            msg.clearData();
            msg.package << result;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbGetContainersInActors:
        {
            std::vector<ContainersInActor> containersInActors;

            if(dbConnector->GetAllContainersInActors(containersInActors))
            {
                std::string jsonString = json11::Json(containersInActors).dump();

                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending containers in actors...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetContainersInActorById:
        {
            uint32_t id;
            msg.package >> id;

            ContainersInActor containersInActor;
            if(dbConnector->GetContainersInActorById(id, containersInActor))
            {
                std::string jsonString = json11::Json(containersInActor).dump();

                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending container in actor by ID...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbUpdateContainersInActor:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            ContainersInActor containersInActor;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                containersInActor = ContainersInActor(JSON);
                containersInActor.CIID = dbConnector->UpdateContainersInActor(containersInActor);

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Update container in actor...\n";
            }

            std::string jsonString = json11::Json(containersInActor).dump();

            msg.clearData();
            msg.package << jsonString;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbDeleteContainersInActor:
        {
            uint32_t id;
            msg.package >> id;

            bool result = dbConnector->DeleteContainersInActor(id);

            msg.clearData();
            msg.package << result;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbGetGraphicsSets:
        {
            std::vector<GraphicsSet> graphicsSets;

            if(dbConnector->GetAllGraphicsSets(graphicsSets))
            {
                std::string jsonString = json11::Json(graphicsSets).dump();

                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending graphics sets...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetGraphicsSetById:
        {
            uint32_t id;
            msg.package >> id;

            GraphicsSet graphicsSet;
            if(dbConnector->GetGraphicsSetById(id, graphicsSet))
            {
                std::string jsonString = json11::Json(graphicsSet).dump();

                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending graphic set by ID...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbUpdateGraphicsSet:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            GraphicsSet graphicsSet;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                graphicsSet = GraphicsSet(JSON);
                graphicsSet.GraphicSetID = dbConnector->UpdateGraphicsSet(graphicsSet);

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Update graphic set...\n";
            }

            std::string jsonString = json11::Json(graphicsSet).dump();

            msg.clearData();
            msg.package << jsonString;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbDeleteGraphicsSet:
        {
            uint32_t id;
            msg.package >> id;

            bool result = dbConnector->DeleteGraphicsSet(id);

            msg.clearData();
            msg.package << result;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbGetGraphicsSetsActors:
        {
            std::vector<GraphicsSetsActor> graphicsSetsActors;

            if(dbConnector->GetAllGraphicsSetsActors(graphicsSetsActors))
            {
                std::string jsonString = json11::Json(graphicsSetsActors).dump();

                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending graphics sets actors...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetGraphicsSetsActorByGSID:
        {
            uint32_t id;
            msg.package >> id;

            std::vector<GraphicsSetsActor> graphicsSetsActors;
            if(dbConnector->GetGraphicsSetsActorsByGSID(id, graphicsSetsActors))
            {
                std::string jsonString = json11::Json(graphicsSetsActors).dump();

                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending graphics sets actors by GSID...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetGraphicsSetsActorByActorID:
        {
            uint32_t id;
            msg.package >> id;

            std::vector<GraphicsSetsActor> graphicsSetsActors;
            if(dbConnector->GetGraphicsSetsActorsByActorID(id, graphicsSetsActors))
            {
                std::string jsonString = json11::Json(graphicsSetsActors).dump();

                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending graphics sets actors by ActorID...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbUpdateGraphicsSetsActor:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            GraphicsSetsActor graphicsSetsActor;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                graphicsSetsActor = GraphicsSetsActor(JSON);
                graphicsSetsActor.GraphicSetID = dbConnector->UpdateGraphicsSetsActors(graphicsSetsActor);

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Update graphic set actor...\n";
            }

            std::string jsonString = json11::Json(graphicsSetsActor).dump();

            msg.clearData();
            msg.package << jsonString;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbDeleteGraphicsSetsActorByGSID:
        {
            uint32_t id;
            msg.package >> id;

            bool result = dbConnector->DeleteGraphicsSetsActorsByGSID(id);

            msg.clearData();
            msg.package << result;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbDeleteGraphicsSetsActorByActorID:
        {
            uint32_t id;
            msg.package >> id;

            bool result = dbConnector->DeleteGraphicsSetsActorsByActorID(id);

            msg.clearData();
            msg.package << result;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbGetGraphicsSetsProducts:
        {
            std::vector<GraphicsSetsProduct> graphicsSetsProducts;

            if(dbConnector->GetAllGraphicsSetsProducts(graphicsSetsProducts))
            {
                std::string jsonString = json11::Json(graphicsSetsProducts).dump();

                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending graphics sets products...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetGraphicsSetsProductById:
        {
            uint32_t id;
            msg.package >> id;

            GraphicsSetsProduct graphicsSetsProduct;
            if(dbConnector->GetGraphicsSetsProductById(id, graphicsSetsProduct))
            {
                std::string jsonString = json11::Json(graphicsSetsProduct).dump();

                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending graphic set product by ID...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbUpdateGraphicsSetsProduct:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            GraphicsSetsProduct graphicsSetsProduct;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                graphicsSetsProduct = GraphicsSetsProduct(JSON);
                graphicsSetsProduct.GraphicSetID = dbConnector->UpdateGraphicsSetsProduct(graphicsSetsProduct);

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Update graphic set product...\n";
            }

            std::string jsonString = json11::Json(graphicsSetsProduct).dump();

            msg.clearData();
            msg.package << jsonString;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbDeleteGraphicsSetsProduct:
        {
            uint32_t id;
            msg.package >> id;

            bool result = dbConnector->DeleteGraphicsSetsProduct(id);

            msg.clearData();
            msg.package << result;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbGetProductsInActors:
        {
            std::vector<ProductsInActor> productsInActors;
            if(dbConnector->GetAllProductsInActors(productsInActors))
            {
                std::string jsonString = json11::Json(productsInActors).dump();

                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending products in actors...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetProductsInActorById:
        {
            uint32_t id;
            msg.package >> id;

            ProductsInActor productsInActor;
            if(dbConnector->GetProductsInActorById(id, productsInActor))
            {
                std::string jsonString = json11::Json(productsInActor).dump();

                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending product in actor by ID...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbUpdateProductsInActor:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            ProductsInActor productsInActor;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
               productsInActor = ProductsInActor(JSON);
               productsInActor.PIID = dbConnector->UpdateProductsInActor(productsInActor);

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Update product in actor...\n";
            }

            std::string jsonString = json11::Json(productsInActor).dump();

            msg.clearData();
            msg.package << jsonString;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbDeleteProductsInActor:
        {
            uint32_t id;
            msg.package >> id;

            bool result = dbConnector->DeleteProductsInActor(id);

            msg.clearData();
            msg.package << result;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbGetProductsInContainers:
        {
            std::vector<ProductsInContainer> productsInContainers;
            if(dbConnector->GetAllProductsInContainers(productsInContainers))
            {
                std::string jsonString = json11::Json(productsInContainers).dump();

                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending products in containers...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetProductsInContainerById:
        {
            uint32_t id;
            msg.package >> id;

            std::vector<ProductsInContainer> productsInContainer;
            if(dbConnector->GetProductsInContainerById(id, productsInContainer))
            {
                std::string jsonString = json11::Json(productsInContainer).dump();

                msg.clearData();
                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending products in container by ID...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbUpdateProductsInContainer:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            ProductsInContainer productsInContainer;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                productsInContainer = ProductsInContainer(JSON);
                productsInContainer.PIID = dbConnector->UpdateProductsInContainer(productsInContainer);

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Update product in container...\n";
            }

            std::string jsonString = json11::Json(productsInContainer).dump();

            msg.clearData();
            msg.package << jsonString;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbDeleteProductsInContainer:
        {
            uint32_t id;
            msg.package >> id;

            bool result = dbConnector->DeleteProductsInContainer(id);

            msg.clearData();
            msg.package << result;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbGetProductInstances:
        {
            std::vector<ProductInstance> instances;

            if(dbConnector->GetAllInstances(instances))
            {
                std::string jsonString = json11::Json(instances).dump();

                msg.package << jsonString;

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetProductInstancesBySID:
        {
            uint32_t id;
            msg.package >> id;

            std::vector<ProductInstance> instances;
            if(dbConnector->GetProductInstancesBySID(id, instances))
            {
                std::string jsonString = json11::Json(instances).dump();

                msg.clearData();
                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Get product instances by SID " << id << "\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetWorkbenchInstanceBySID:
        {
            uint32_t id;
            msg.package >> id;

            std::vector<ProductInstance> productInstances;
            if(dbConnector->GetProductInstancesBySID(id, productInstances))
            {
                std::vector<WorkbenchInstance> instances;
                for(auto product : productInstances)
                {
                    WorkbenchInstance instance;
                    if(dbConnector->GetWorkbenchInstanceByPIID(product.PIID, instance))
                        instances.push_back(instance);
                }

                if(instances.size() > 0)
                {
                    std::string jsonString = json11::Json(instances).dump();

                    msg.clearData();
                    msg.package << jsonString;

                    std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                              << ": [SERVER] Get workbench instances by SID " << id << "\n";

                    SendMessage(event.peer, msg);
                }
            }

            break;
        }

        case MessageTypes::DbGetProductInstanceById:
        {
            uint32_t id;
            msg.package >> id;

            ProductInstance instance;
            if(dbConnector->GetInstanceById(id, instance))
            {
                std::string jsonString = json11::Json(instance).dump();

                msg.clearData();
                msg.package << jsonString;

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbUpdateProductInstance:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            ProductInstance instance;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                instance = ProductInstance(JSON);
                instance.PIID = dbConnector->UpdateInstance(instance);

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Update product instance | New PIID: " << instance.PIID << "\n";
            }

            std::string jsonString = json11::Json(instance).dump();

            msg.clearData();
            msg.package << jsonString;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbDeleteProductInstance:
        {
            uint32_t id;
            msg.package >> id;

            bool result = dbConnector->DeleteInstance(id);

            msg.clearData();
            msg.package << result;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbGetContainerArchetypes:
        {
            std::vector<ContainerArchetype> containerArchetypes;

            if(dbConnector->GetAllContainerArchetypes(containerArchetypes))
            {
                std::string jsonString = json11::Json(containerArchetypes).dump();

                msg.package << jsonString;

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetContainerArchetypeById:
        {
            uint32_t id;
            msg.package >> id;

            ContainerArchetype containerArchetype;
            if(dbConnector->GetContainerArchetypeById(id, containerArchetype))
            {
                std::string jsonString = json11::Json(containerArchetype).dump();

                msg.clearData();
                msg.package << jsonString;

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbUpdateContainerArchetype:
        {
            std::string parseError;

            json11::Json JSON = json11::Json::parse(msg.GetString(), parseError);

            ContainerArchetype containerArchetype;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                containerArchetype = ContainerArchetype(JSON);
                containerArchetype.CAID = dbConnector->UpdateContainerArchetype(containerArchetype);
            }

            std::string jsonString = json11::Json(containerArchetype).dump();

            msg.clearData();
            msg.package << jsonString;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbDeleteContainerArchetype:
        {
            uint32_t id;
            msg.package >> id;

            bool result = dbConnector->DeleteContainerArchetype(id);

            msg.clearData();
            msg.package << result;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbGetSpaceActors:
        {
            std::vector<SpaceActor> spaceActors;

            if(dbConnector->GetAllSpaceActors(spaceActors))
            {
                std::string jsonString = json11::Json(spaceActors).dump();

                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending space actors...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetSpaceActorBySID:
        {
            uint32_t id;
            msg.package >> id;

            std::vector<SpaceActor> spaceActors;
            if(dbConnector->GetSpaceActorBySID(id, spaceActors))
            {
                std::string jsonString = json11::Json(spaceActors).dump();

                msg.clearData();
                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending space actors by SID...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetSpaceActorByActorID:
        {
            uint32_t id;
            msg.package >> id;

            SpaceActor spaceActor;
            if(dbConnector->GetSpaceActorByPIID(id, spaceActor))
            {
                std::string jsonString = json11::Json(spaceActor).dump();

                msg.clearData();
                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending space actor by ActorID...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbUpdateSpaceActor:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            SpaceActor spaceActor;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                spaceActor = SpaceActor(JSON);
                spaceActor.ActorID = dbConnector->UpdateSpaceActor(spaceActor);

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Update space actor...\n";
            }

            std::string jsonString = json11::Json(spaceActor).dump();

            msg.clearData();
            msg.package << jsonString;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbDeleteSpaceActor:
        {
            uint32_t id;
            msg.package >> id;

            bool result = dbConnector->DeleteSpaceActor(id);

            msg.clearData();
            msg.package << result;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbGetSpaceAreas:
        {
            std::vector<Space> spaceAreas;

            if(dbConnector->GetAllSpaceAreas(spaceAreas))
            {
                std::string jsonString = json11::Json(spaceAreas).dump();

                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending space areas...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetSpaceAreaBySID:
        {
            uint32_t id;
            msg.package >> id;

            Space spaceArea;
            if(dbConnector->GetSpaceAreaBySID(id, spaceArea))
            {
                std::string jsonString = json11::Json(spaceArea).dump();

                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending space area by SID...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbUpdateSpaceArea:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            Space spaceArea;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                spaceArea = Space(JSON);
                spaceArea.SID = dbConnector->UpdateSpaceArea(spaceArea);

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Update space area...\n";
            }

            std::string jsonString = json11::Json(spaceArea).dump();

            msg.clearData();
            msg.package << jsonString;

            SendMessage(event.peer, msg);
            break;
        }

        case MessageTypes::DbDeleteSpaceArea:
        {
            uint32_t id;
            msg.package >> id;

            bool result = dbConnector->DeleteSpaceArea(id);

            msg.clearData();
            msg.package << result;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbGetPlayerInventoryByID:
        {
            uint32_t id;
            msg.package >> id;

            PlayerInventory inventory;
            if(dbConnector->GetPlayerInventoryByID(id, inventory))
            {
                std::string jsonString = json11::Json(inventory).dump();

                msg.clearData();
                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending player inventory by ID\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbAddPlayerInventory:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            PlayerInventory inventory;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                inventory = PlayerInventory(JSON);
                inventory.UserID = dbConnector->UpdatePlayerInventory(inventory);

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Update player inventory | New UserID: " << inventory.UserID << "\n";
            }

            std::string jsonString = json11::Json(inventory).dump();

            msg.clearData();
            msg.package << jsonString;

            SendMessage(event.peer, msg);
            break;
        }

        case MessageTypes::DbDeletePlayerInventory:
        {
            uint32_t id;
            msg.package >> id;

            bool result = dbConnector->DeletePlayerInventory(id);

            msg.clearData();
            msg.package << result;

            SendMessage(event.peer, msg);
            break;
        }

        case MessageTypes::DbGetCharacterByID:
        {
            uint32_t id;
            msg.package >> id;

            Character character;
            if(dbConnector->GetCharacterByID(id, character))
            {
                std::string jsonString = json11::Json(character).dump();

                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending character by ID...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetContainersInProducts:
        {
            std::vector<ContainerInProduct> containersInProducts;
            if(dbConnector->GetAllContainersInProducts(containersInProducts))
            {
                std::string jsonString = json11::Json(containersInProducts).dump();

                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending containers in products...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetContainerInProductByID:
        {
            uint32_t id;
            msg.package >> id;

            ContainerInProduct containerInProduct;
            if(dbConnector->GetContainerInProductById(id, containerInProduct))
            {
                std::string jsonString = json11::Json(containerInProduct).dump();

                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending container in product by ID...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbUpdateContainerInProduct:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            ContainerInProduct containerInProduct;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                containerInProduct = ContainerInProduct(JSON);
                containerInProduct.CIID = dbConnector->UpdateContainerInProduct(containerInProduct);

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Update container in product...\n";
            }

            std::string jsonString = json11::Json(containerInProduct).dump();

            msg.clearData();
            msg.package << jsonString;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbDeleteContainerInProduct:
        {
            uint32_t id;
            msg.package >> id;

            bool result = dbConnector->DeleteContainerInProduct(id);

            msg.clearData();
            msg.package << result;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbTransferProduct:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            TransferContainerToContainer transfer;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                transfer = TransferContainerToContainer(JSON);
                bool result;
                ProductsInContainer productInContainer;
                if(dbConnector->GetProductInContainerByPIID(transfer.PIID, productInContainer))
                {
                    result = dbConnector->TransferProduct(transfer.PIID,
                                                               transfer.CIID,
                                                               transfer.deltaMass);

                    std::vector<ProductsInContainer> products;
                    if(dbConnector->GetProductsInContainerById(productInContainer.CIID, products))
                    {
                        std::string jsonString = json11::Json(products).dump();

                        MsgBroadcast.clearData();
                        MsgBroadcast.type = MessageTypes::ContainerChangedBroadcast;

                        MsgBroadcast.package << jsonString;
                        MsgBroadcast.package << productInContainer.CIID;

                        SendBroadcast(server, MsgBroadcast);

                        products.clear();
                        if(dbConnector->GetProductsInContainerById(transfer.CIID, products))
                        {
                            jsonString = json11::Json(products).dump();

                            MsgBroadcast.clearData();
                            MsgBroadcast.type = MessageTypes::ContainerChangedBroadcast;

                            MsgBroadcast.package << jsonString;
                            MsgBroadcast.package << transfer.CIID;

                            SendBroadcast(server, MsgBroadcast);
                        }
                    }
                }

                msg.clearData();
                msg.package << result;

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbTransferProductContainerToHand:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            ProductInCharacter productInCharacter;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                productInCharacter = ProductInCharacter(JSON);

                bool result;

                ProductsInContainer productInContainer;
                if(dbConnector->GetProductInContainerByPIID(productInCharacter.PIID, productInContainer))
                {
                    result = dbConnector->TransferProductContainerToHand(productInCharacter.PIID,
                                                                              productInCharacter.CharacterID);

                    std::vector<ProductsInContainer> products;
                    if(dbConnector->GetProductsInContainerById(productInContainer.CIID, products)) {
                        std::string jsonString = json11::Json(products).dump();

                        MsgBroadcast.clearData();
                        MsgBroadcast.type = MessageTypes::ContainerChangedBroadcast;

                        MsgBroadcast.package << jsonString;
                        MsgBroadcast.package << productInContainer.CIID;

                        SendBroadcast(server, MsgBroadcast);
                    }
                }

                msg.clearData();
                msg.package << result;

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbTransferProductHandToContainer:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            ProductsInContainer productInContainer;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                productInContainer = ProductsInContainer(JSON);

                bool result = dbConnector->TransferProductHandToContainer(productInContainer.PIID,
                                                                          productInContainer.CIID);

                std::vector<ProductsInContainer> products;
                if(dbConnector->GetProductsInContainerById(productInContainer.CIID, products)) {
                    std::string jsonString = json11::Json(products).dump();

                    MsgBroadcast.clearData();
                    MsgBroadcast.type = MessageTypes::ContainerChangedBroadcast;

                    MsgBroadcast.package << jsonString;
                    MsgBroadcast.package << productInContainer.CIID;

                    SendBroadcast(server, MsgBroadcast);
                }

                msg.clearData();
                msg.package << result;

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbTransferProductContainerToSpaceActor:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            TransferContainerToSpaceActor transfer;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                transfer = TransferContainerToSpaceActor(JSON);

                bool result;

                result = dbConnector->TransferProductContainerToSpaceActor(transfer.actor);

                MsgBroadcast.clearData();
                MsgBroadcast.type = MessageTypes::CreateProductActorToScene;
                std::string jsonString = json11::Json(transfer.actor).dump();
                MsgBroadcast.package << jsonString;
                MsgBroadcast.package << -1;

                if(result)
                    SendBroadcast(server, MsgBroadcast);

                ProductsInContainer productInContainer;
                if(dbConnector->GetProductInContainerByPIID(transfer.actor.PIID, productInContainer))
                {
                    std::vector<ProductsInContainer> products;
                    if(dbConnector->GetProductsInContainerById(productInContainer.CIID, products)) {
                        jsonString = json11::Json(products).dump();

                        MsgBroadcast.clearData();
                        MsgBroadcast.type = MessageTypes::ContainerChangedBroadcast;

                        MsgBroadcast.package << jsonString;
                        MsgBroadcast.package << productInContainer.CIID;

                        SendBroadcast(server, MsgBroadcast);
                    }
                }

                msg.clearData();
                msg.package << result;

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbTransferProductSpaceActorToHand:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            TransferSpaceActorToHand transfer;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                transfer = TransferSpaceActorToHand(JSON);

                bool result = dbConnector->TransferProductSpaceActorToHand(transfer.ActorID,
                                                                           transfer.CharacterID);

                SpaceActor spaceActor;
                spaceActor.ActorID = transfer.ActorID;

                MsgBroadcast.clearData();
                MsgBroadcast.type = MessageTypes::DeleteProductActorFromScene;
                std::string jsonString = json11::Json(spaceActor).dump();
                MsgBroadcast.package << jsonString;
                MsgBroadcast.package << transfer.CharacterID;

                if(result)
                    SendBroadcast(server, MsgBroadcast);

                msg.clearData();
                msg.package << result;

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbTransferProductHandToSpaceActor:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            TransferHandToSpaceActor transfer;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                transfer = TransferHandToSpaceActor(JSON);

                bool result = dbConnector->TransferProductHandToSpaceActor(transfer.actor);

                MsgBroadcast.clearData();
                MsgBroadcast.type = MessageTypes::CreateProductActorToScene;
                std::string jsonString = json11::Json(transfer.actor).dump();
                MsgBroadcast.package << jsonString;
                MsgBroadcast.package << transfer.CharacterID;

                if(result)
                    SendBroadcast(server, MsgBroadcast);

                msg.clearData();
                msg.package << result;

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbCreateProductToSpaceActor:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            if(Msg.length() < 3)
                return;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            SpaceActor spaceActor;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                spaceActor = SpaceActor(JSON);

                bool result = dbConnector->TransferProductToSpaceActor(spaceActor);

                MsgBroadcast.clearData();
                MsgBroadcast.type = MessageTypes::CreateProductActorToScene;
                std::string jsonString = json11::Json(spaceActor).dump();
                MsgBroadcast.package << jsonString;
                MsgBroadcast.package << -1;

                if(result)
                    SendBroadcast(server, MsgBroadcast);

                msg.clearData();
            }

            break;
        }
        case MessageTypes::DbCreateProductInstance:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            ProductInstance instance;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                instance = ProductInstance(JSON);
                instance.PIID = -1;

                instance.PIID = dbConnector->UpdateInstance(instance);

                ContainersInProductArchetype pcArchetype;
                if(dbConnector->GetContainersInProductArchetypeByPAID(instance.PAID, pcArchetype))
                {
                    ContainerInstance newContainer(-1, pcArchetype.CAID, instance.PIID, 1);
                    newContainer.CIID = dbConnector->UpdateContainerInstance(newContainer);

                    ContainerInProduct containerInProduct(instance.PIID, newContainer.CIID);
                    dbConnector->UpdateContainerInProduct(containerInProduct);
                }

                ContainerInProduct containerInProduct;
                if(dbConnector->GetContainerInProductById(instance.PIID, containerInProduct))
                {
                    ContainerInstance container;
                    dbConnector->GetContainerInstanceById(containerInProduct.CIID, container);

                    MsgBroadcast.clearData();
                    MsgBroadcast.type = MessageTypes::DbCreateContainerInstance;
                    std::string jsonString = json11::Json(container).dump();
                    MsgBroadcast.package << jsonString;
                    MsgBroadcast.package << -1;

                    if(container.CIID > -1)
                        SendBroadcast(server, MsgBroadcast);
                }

                MsgBroadcast.clearData();
                MsgBroadcast.type = MessageTypes::DbCreateProductInstance;
                std::string jsonString = json11::Json(instance).dump();
                MsgBroadcast.package << jsonString;
                MsgBroadcast.package << -1;

                if(instance.PIID > -1)
                    SendBroadcast(server, MsgBroadcast);

                MsgBroadcast.clearData();
                MsgBroadcast.type = MessageTypes::DbCreateContainerInProduct;
                jsonString = json11::Json(containerInProduct).dump();
                MsgBroadcast.package << jsonString;
                MsgBroadcast.package << -1;

                if(containerInProduct.PIID > -1)
                    SendBroadcast(server, MsgBroadcast);

                msg.clearData();
            }

            break;
        }

        case MessageTypes::DbGetProductInCharacterByID:
        {
            uint32_t id;
            msg.package >> id;

            ProductInCharacter productInCharacter;
            if(dbConnector->GetProductInCharacterById(id, productInCharacter))
            {
                std::string jsonString = json11::Json(productInCharacter).dump();

                msg.clearData();
                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                            << ": [SERVER] Sending product in character by ID...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetAllWorkbenchArchetypes:
        {
            std::vector<WorkbenchArchetype> workbenchArchetypes;

            if(dbConnector->GetAllWorkbenchArchetypes(workbenchArchetypes))
            {
                std::string jsonString = json11::Json(workbenchArchetypes).dump();

                msg.clearData();
                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                         << ": [SERVER] Sending workbench archetypes...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetWorkbenchArchetypeByPAID:
        {
            uint32_t id;
            msg.package >> id;

            WorkbenchArchetype archetype;
            if(dbConnector->GetWorkbenchArchetypeByPAID(id, archetype))
            {
                std::string jsonString = json11::Json(archetype).dump();

                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending workbench archetype...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetAllWorkbenchInstances:
        {
            std::vector<WorkbenchInstance> instances;

            if(dbConnector->GetAllWorkbenchInstances(instances))
            {
                std::string jsonString = json11::Json(instances).dump();

                msg.clearData();
                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending workbench instances...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetWorkbenchInstanceByPIID:
        {
            uint32_t id;
            msg.package >> id;

            WorkbenchInstance instance;
            if(dbConnector->GetWorkbenchInstanceByPIID(id, instance))
            {
                std::string jsonString = json11::Json(instance).dump();

                msg.clearData();
                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending workbench instance...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbUpdateWorkbenchInstance:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            WorkbenchInstance instance;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                instance = WorkbenchInstance(JSON);
                instance.WBID = dbConnector->UpdateWorkbenchInstance(instance);

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Update workbench instance...\n";
            }

            std::string jsonString = json11::Json(instance).dump();

            msg.clearData();
            msg.package << jsonString;

            SendMessage(event.peer, msg);
            break;
        }

        case MessageTypes::DbDeleteWorkbenchInstance:
        {
            uint32_t id;
            msg.package >> id;

            bool result = dbConnector->DeleteWorkbenchInstance(id);

            msg.clearData();
            msg.package << result;

            SendMessage(event.peer, msg);

            break;
        }

        case MessageTypes::DbGetAllProductArchetypeComponents:
        {
            std::vector<ProductArchetypeComponent> productArchetypeComponents;

            if(dbConnector->GetAllProductArchetypeComponent(productArchetypeComponents))
            {
                std::string jsonString = json11::Json(productArchetypeComponents).dump();

                msg.package << jsonString;

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetProductArchetypeComponentsByPAID:
        {
            uint32_t id;
            msg.package >> id;

            std::vector<ProductArchetypeComponent> components;
            if(dbConnector->GetProductArchetypeComponentsByPAID(id, components))
            {
                std::string jsonString = json11::Json(components).dump();

                msg.clearData();
                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Get product archetype components by PAID " << id << "\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetAllContainerComponents:
        {
            std::vector<ContainerComponent> components;
            if(dbConnector->GetAllContainerComponents(components))
            {
                std::string jsonString = json11::Json(components).dump();

                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending container components...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetContainerComponentByCCID:
        {
            uint32_t id;
            msg.package >> id;

            ContainerComponent component;
            if(dbConnector->GetContainerComponentByCCID(id, component))
            {
                std::string jsonString = json11::Json(component).dump();

                msg.clearData();
                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Get workbench instances by SID " << id << "\n";


                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetAllInteractiveObjectComponents:
        {
            std::vector<InteractiveObjectComponent> interactComponents;
            if(dbConnector->GetAllInteractiveObjectComponents(interactComponents))
            {
                std::string jsonString = json11::Json(interactComponents).dump();

                msg.clearData();
                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending interactive object components...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetInteractiveObjectComponentByIOCID:
        {
            uint32_t id;
            msg.package >> id;

            InteractiveObjectComponent component;
            if(dbConnector->GetInteractiveObjectComponentByIOCID(id, component))
            {
                std::string jsonString = json11::Json(component).dump();

                msg.clearData();
                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending interactive object componet by IOCID...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetAllWorkbenchComponents:
        {
            std::vector<WorkbenchComponent> components;
            if(dbConnector->GetAllWorkbenchComponents(components))
            {
                std::string jsonString = json11::Json(components).dump();

                msg.clearData();
                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending workbench components...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetWorkbenchComponentByWBCID:
        {
            uint32_t id;
            msg.package >> id;

            WorkbenchComponent component;
            if(dbConnector->GetWorkbenchComponentByID(id, component))
            {
                std::string jsonString = json11::Json(component).dump();

                msg.clearData();
                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending workbench component by ID...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetAllWorkbenchComponentsInWorkbenchArchetypes:
        {
            std::vector<WorkbenchComponentInWorkbenchArchetype> links;
            if(dbConnector->GetAllWorkbenchComponentsInWorkbenchArchetype(links))
            {
                std::string jsonString = json11::Json(links).dump();

                msg.clearData();
                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending workbench components in workbench archetypes...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetWorkbenchComponentSlotByWBSID:
        {
            uint32_t id;
            msg.package >> id;

            WorkbenchComponentSlot slot;
            if(dbConnector->GetWorkbenchComponentSlotByID(id, slot))
            {

            }
        }

        case MessageTypes::DbGetAllWorkbenchSchemes:
        {
            std::vector<WorkbenchScheme> schemes;
            if(dbConnector->GetAllWorkbenchSchemes(schemes))
            {
                std::string jsonString = json11::Json(schemes).dump();

                msg.clearData();
                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending workbench schemes...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetAllConversionWorkbenchSchemes:
        {
            std::vector<ConversionWorkbenchScheme> schemes;
            if(dbConnector->GetAllConversionWorkbenchSchemes(schemes))
            {
                std::string jsonString = json11::Json(schemes).dump();

                msg.clearData();
                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending conversion workbench schemes...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetAllConversionSourcesWorkbenchSchemes:
        {
            std::vector<ConversionSourcesWorkbenchScheme> schemes;
            if(dbConnector->GetAllConversionSourcesWorkbenchSchemes(schemes))
            {
                std::string jsonString = json11::Json(schemes).dump();

                msg.clearData();
                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending conversion sources workbench schemes...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetAllConversionProductsWorkbenchSchemes:
        {
            std::vector<ConversionProductsWorkbenchScheme> schemes;
            if(dbConnector->GetAllConversionProductsWorkbenchSchemes(schemes))
            {
                std::string jsonString = json11::Json(schemes).dump();

                msg.clearData();
                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending conversion products workbench schemes...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::DbGetAllContainersInWorkbenchArchetypes:
        {
            std::vector<ContainersInWorkbenchArchetype> containers;
            if(dbConnector->GetAllContainersInWorkbenchArchetype(containers))
            {
                std::string jsonString = json11::Json(containers).dump();

                msg.clearData();
                msg.package << jsonString;

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Sending containers in workbench archetypes...\n";

                SendMessage(event.peer, msg);
            }

            break;
        }

        case MessageTypes::StartConversionProcess:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            ConversionInfo Info;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                Info = ConversionInfo(JSON);
                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Update conversion process...\n";

                msg.clearData();

                if(dbConnector->CheckConversionProcess(Info))
                {
                    ConversionWorkbenchScheme scheme;
                    if(dbConnector->GetConversionWorkbenchSchemeByID(Info.SchemeID, scheme))
                    {
                        long int timeNow = static_cast<long int>(time);
                        auto EndTime = timeNow + (long int)round(scheme.Time);

                        std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                                  << ": [SERVER] Time now: " << timeNow << "; End Time of conversion: " << EndTime << "\n";

                        if(TimeList.empty())
                        {
                            TimeList.push_back(EndTime);
                            ConversionInfoList.push_back(Info);
                        } else
                        {
                            for(int i = 0; i < TimeList.size(); i++)
                            {
                                if(TimeList[i] < EndTime)
                                {
                                    TimeList.insert(TimeList.begin() + i, 1, EndTime);
                                    ConversionInfoList.insert(ConversionInfoList.begin() + i, 1, Info);
                                    break;
                                }
                            }
                        }
                    }

                    std::string jsonString = json11::Json(Info).dump();

                    MsgBroadcast.clearData();
                    MsgBroadcast.package << jsonString;
                    MsgBroadcast.package << -1;

                    MsgBroadcast.type = MessageTypes::StartingConversionProcessBroadcast;

                    SendBroadcast(server, MsgBroadcast);
                } else
                {
                    // Conversion Denied
                    std::string jsonString = json11::Json(Info).dump();

                    MsgBroadcast.clearData();
                    MsgBroadcast.package << jsonString;
                    MsgBroadcast.package << -1;

                    MsgBroadcast.type = MessageTypes::CheckConversionProcessDenied;

                    SendBroadcast(server, MsgBroadcast);
                }
            }
            break;
        }

        case MessageTypes::StopConversionProcess:
        {
            std::string parseError;
            std::string Msg;
            msg.package >> Msg;

            json11::Json JSON = json11::Json::parse(Msg, parseError);

            ConversionInfo Info;
            if(!parseError.empty() || JSON.is_null() || !JSON.is_object())
            {
                Info = ConversionInfo(JSON);

                std::cout << localtime->tm_hour << ":" << localtime->tm_min << ":" << localtime->tm_sec
                          << ": [SERVER] Stop conversion process...\n";

                bool found = false;
                for(int i = 0; i < ConversionInfoList.size(); i++)
                {
                    if(ConversionInfoList[i] == Info)
                    {
                        ConversionInfoList.erase(ConversionInfoList.begin() + i);
                        TimeList.erase(TimeList.begin() + i);

                        found = true;
                        break;
                    }
                }

                if(found)
                {
                    std::string jsonString = json11::Json(Info).dump();

                    MsgBroadcast.clearData();
                    MsgBroadcast.package << jsonString;
                    MsgBroadcast.package << -1;

                    MsgBroadcast.type = MessageTypes::BreakingConversionProcessBroadcast;

                    SendBroadcast(server, MsgBroadcast);
                }
            }

            break;
        }

        case MessageTypes::DbGetAllWorkbenchComponentSlots:
        {


            break;
        }
    }
}

bool ENetServer::Start() {
    MsgPosition.type = MessageTypes::ClientPosition;

    old_time = clock();

    return server_interface::Start();
}

void ENetServer::Update() {
    if(curr_time >= send_time && !Movements.empty())
    {
        std::vector<PlayerMovement> movementsArray;
        movementsArray.reserve(Movements.size());

        for(auto value : Movements)
        {
            movementsArray.push_back(value.second);
        }
        std::string jsonString = json11::Json(movementsArray).dump();

        MsgPosition.clearData();
        MsgPosition.package << jsonString;

        MsgPosition.package << Time::timestamp();

        SendBroadcast(server, MsgPosition);

        curr_time = .0f;
    }

    curr_time += clock() - old_time;
    old_time = clock();

    if(!TimeList.empty())
    {
        auto now = std::chrono::system_clock::now();
        std::time_t timerNow = std::chrono::system_clock::to_time_t(now);

        long int longNow = static_cast<long int>(timerNow);

        while(TimeList.back() <= longNow && !TimeList.empty())
        {
            ConversionInfo Info = ConversionInfoList.back();

            if(dbConnector->StartConversionProcess(Info))
            {
                std::string jsonString = json11::Json(Info).dump();

                MsgBroadcast.clearData();
                MsgBroadcast.package << jsonString;
                MsgBroadcast.package << -1;

                MsgBroadcast.type = MessageTypes::CompletedConversionProcessBroadcast;

                SendBroadcast(server, MsgBroadcast);
            }

            TimeList.pop_back();
            ConversionInfoList.pop_back();
        }
    }

    server_interface::Update();
}