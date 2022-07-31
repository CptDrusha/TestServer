#include <unordered_map>
#include "DbConnector.h"

DbConnector::DbConnector() {
    // Try get sql driver, that will handle connections
    try {
        driver = get_driver_instance();
    }
    catch (sql::SQLException& e) {
        std::cout << "Creating db driver error: " << e.what() << std::endl;
    }
}

DbConnector::~DbConnector() {
    if (conn != nullptr) {
        conn->close();
    }
}

bool DbConnector::Connect() {
    if (driver == nullptr) {
        std::cout << "Connection to db error: driver not initialized" << std::endl;
        return false;
    }

    // Try to connect to database
    // and create statement that will handle all requests
    try {
        conn = driver->connect("tcp://127.0.0.1", "dml", "dml1407");

        // Tell MySQL to use database Products to this connection
        sql::Statement* stmt = conn->createStatement();
        stmt->execute("USE Products");
    }
    catch (sql::SQLException& e) {
        std::cout << "Connection to db error: " << e.what() << std::endl;
        return false;
    }

    return true;
}

bool DbConnector::isConnect() {
    if(conn == nullptr)
        return Connect();

    if(!conn->isClosed())
        return true;

    return false;
}

bool DbConnector::GetAllCharacterBodyTypes(std::vector<CharacterBodyType> &BodyTypes) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting character body type: connection not initialized" << std::endl;
        return false;
    }

    sql::Statement* stmt = conn->createStatement();
    sql::ResultSet* res = stmt->executeQuery("SELECT * FROM CharacterBodyTypes");

    while(res->next())
    {
        CharacterBodyType bodyType(res->getInt(1),
                                   res->getBoolean(2),
                                   res->getDouble(3),
                                   res->getDouble(4),
                                   res->getDouble(5),
                                   res->getDouble(6),
                                   res->getDouble(7),
                                   res->getDouble(8),
                                   res->getDouble(9),
                                   res->getDouble(10),
                                   res->getDouble(11));

        BodyTypes.push_back(bodyType);
    }

    return true;
}

bool DbConnector::GetCharacterBodyType(uint32_t id, CharacterBodyType &BodyType) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting character body type: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM CharacterBodyTypes"
                                                  " WHERE CharacterID=?");
    stmt->setInt(1, id);

    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
       BodyType.SetData(res->getInt(1),
                        res->getBoolean(2),
                        res->getDouble(3),
                        res->getDouble(4),
                        res->getDouble(5),
                        res->getDouble(6),
                        res->getDouble(7),
                        res->getDouble(8),
                        res->getDouble(9),
                        res->getDouble(10),
                        res->getDouble(11));

       return true;
    }

    return false;
}

int DbConnector::UpdateCharacterBodyType(CharacterBodyType &BodyType) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting character body type: connection not initialized" << std::endl;
        return -1;
    }

    sql::PreparedStatement* checkExistStmt = conn->prepareStatement("SELECT * FROM CharacterBodyTypes"
                                                                    " WHERE CharacterID=(?)");

    checkExistStmt->setInt(1, BodyType.CharacterID);

    sql::ResultSet* checkRes = checkExistStmt->executeQuery();

    if(!checkRes->next())
        return AddCharacterBodyType(BodyType);

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM CharacterBodyTypes"
                                                               " WHERE Male=?"
                                                               " AND Height=?"
                                                               " AND Shoulders=?"
                                                               " AND ArmLength=?"
                                                               " AND LegLength=?"
                                                               " AND Muscle=?"
                                                               " AND Belly=?"
                                                               " AND Fat=?"
                                                               " AND Breast=?"
                                                               " AND Hips=?");

    checkStmt->setBoolean(1, BodyType.bMale);
    checkStmt->setDouble(2, BodyType.Height);
    checkStmt->setDouble(3, BodyType.Shoulders);
    checkStmt->setDouble(4, BodyType.ArmLength);
    checkStmt->setDouble(5, BodyType.LegLength);
    checkStmt->setDouble(6, BodyType.Muscle);
    checkStmt->setDouble(7, BodyType.Belly);
    checkStmt->setDouble(8, BodyType.Fat);
    checkStmt->setDouble(9, BodyType.Breast);
    checkStmt->setDouble(10, BodyType.Hips);

    sql::ResultSet* res = checkStmt->executeQuery();

    if(!res->next())
    {
        sql::PreparedStatement* stmt = conn->prepareStatement("UPDATE CharacterBodyTypes"
                                                              " SET"
                                                              " Male=?,"
                                                              " Height=?,"
                                                              " Shoulders=?,"
                                                              " ArmLength=?,"
                                                              " LegLength=?,"
                                                              " Muscle=?,"
                                                              " Belly=?,"
                                                              " Fat=?,"
                                                              " Breast=?,"
                                                              " Hips=?"
                                                              " WHERE CharacterID=(?)");
        stmt->setBoolean(1, BodyType.bMale);
        stmt->setDouble(2, BodyType.Height);
        stmt->setDouble(3, BodyType.Shoulders);
        stmt->setDouble(4, BodyType.ArmLength);
        stmt->setDouble(5, BodyType.LegLength);
        stmt->setDouble(6, BodyType.Muscle);
        stmt->setDouble(7, BodyType.Belly);
        stmt->setDouble(8, BodyType.Fat);
        stmt->setDouble(9, BodyType.Breast);
        stmt->setDouble(10, BodyType.Hips);
        stmt->setInt(11, BodyType.CharacterID);

        stmt->executeQuery();

        return FindCharacterBodyType(BodyType);
    } else
        return FindCharacterBodyType(BodyType);
}

bool DbConnector::DeleteCharacterBodyType(uint32_t id) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting character body type: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("DELETE FROM CharacterBodyTypes"
                                                          " WHERE CharacterID=?");

    stmt->setInt(1, id);
    stmt->executeQuery();

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM CharacterBodyTypes"
                                                               " WHERE CharacterID=?");

    checkStmt->setInt(1, id);
    sql::ResultSet* res = checkStmt->executeQuery();

    return !res->next();
}

bool DbConnector::GetCharacterBodyCloth(uint32_t id, std::vector<CharacterBodyCloth> &BodyCloths) {
    return false;
}

int DbConnector::UpdateCharacterBodyCloth(CharacterBodyCloth &BodyCloth) {
    return 0;
}

bool DbConnector::DeleteCharacterBodyCloth(uint32_t id) {
    return false;
}


bool DbConnector::GetAllArchetypes(std::vector<ProductArchetype>& archetypes) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting archetypes: connection not initialized" << std::endl;
        return false;
    }

    sql::Statement* stmt = conn->createStatement();
    sql::ResultSet* res = stmt->executeQuery("SELECT * FROM ProductArchetypes");

    // Get structs from query results
    while (res->next()) {
        ProductArchetype archetype(res->getInt(1),
                                   res->getString(2),
                                   res->getString(3),
                                   res->getString(4),
                                   res->getDouble(5),
                                   res->getString(6));
        archetypes.push_back(archetype);
    }

    return true;
}

bool DbConnector::GetArchetypeById(uint32_t id, ProductArchetype& archetype) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting archetype: connection not initialized" << std::endl;
        return false;
    }

    // Prepare query
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ProductArchetypes WHERE PAID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    if (res->next()) {
        archetype.SetData(res->getInt(1),
                          res->getString(2),
                          res->getString(3),
                          res->getString(4),
                          res->getDouble(5),
                          res->getString(6));

        return true;
    }

    return false;
}

bool DbConnector::GetArchetypeByName(std::string name, ProductArchetype &archetype) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting archetype: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ProductArchetypes"
                                                          " WHERE Name=(?)");
    stmt->setString(1, name);
    sql::ResultSet* res = stmt->executeQuery();

    if(res->next()) {
        archetype.SetData(res->getInt(1),
                          res->getString(2),
                          res->getString(3),
                          res->getString(4),
                          res->getDouble(5),
                          res->getString(6));

        return true;
    }

    return false;
}

int DbConnector::UpdateArchetype(ProductArchetype &archetype) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error updating archetype: connection not initialized" << std::endl;
        return -1;
    }

    std::cout << "Update archetype - " << archetype.name << " with PAID - " << archetype.PAID << std::endl;

    // First, check if we have to add new item
    if (archetype.PAID < 0) {
        sql::PreparedStatement *checkStmt = conn->prepareStatement("SELECT * FROM ProductArchetypes"
                                                                   " WHERE PAID=?");
        checkStmt->setInt(1, archetype.PAID);
        sql::ResultSet* res = checkStmt->executeQuery();

        std::cout << "res is " << !res->next() << std::endl;

        if (!res->next()) {
            return AddArchetype(archetype);
        }
    }

    // Now, we have to update its values
    // Prepare query
    sql::PreparedStatement* stmt = conn->prepareStatement("UPDATE ProductArchetypes"
                                                          " SET"
                                                          " Name=?,"
                                                          " Description=?,"
                                                          " ActorClass=?,"
                                                          " Density=?,"
                                                          " Type=?"
                                                          " WHERE PAID=?");
    stmt->setString(1, archetype.name);
    stmt->setString(2, archetype.description);
    stmt->setString(3, archetype.actorClass);
    stmt->setInt(4, archetype.density);
    stmt->setString(5, archetype.GetTypeString());
    stmt->setInt(6, archetype.PAID);

    stmt->executeQuery();

    // Return new archetype id
    return FindArchetypeByFields(archetype);
}

bool DbConnector::DeleteArchetype(uint32_t id) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error deleting archetype: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("DELETE FROM ProductArchetypes"
                                                          " WHERE PAID=?");
    stmt->setInt(1, id);
    stmt->executeQuery();

    // Check that there's no more archetype set with this id
    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM ProductArchetypes"
                                                               " WHERE PAID=(?)");
    checkStmt->setInt(1, id);
    sql::ResultSet* res = checkStmt->executeQuery();

    return !res->next();
}




bool DbConnector::GetAllGraphicsSets(std::vector<GraphicsSet>& graphicsSets) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting graphics sets: connection not initialized" << std::endl;
        return false;
    }

    sql::Statement* stmt = conn->createStatement();
    sql::ResultSet* res = stmt->executeQuery("SELECT * FROM GraphicsSets");

    // Get structs from query results
    while (res->next()) {
        GraphicsSet graphicsSet(res->getInt(1));
        graphicsSets.push_back(graphicsSet);
    }

    return true;
}

bool DbConnector::GetGraphicsSetById(uint32_t id, GraphicsSet& graphicsSets) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting graphics set: connection not initialized" << std::endl;
        return false;
    }

    // Prepare query
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM GraphicsSets"
                                                          " WHERE GraphicSetID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    if (res->next()) {
        graphicsSets.SetData(res->getInt(1));

        return true;
    }

    return false;
}

int DbConnector::UpdateGraphicsSet(GraphicsSet &graphicsSet) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error updating graphics set: connection not initialized" << std::endl;
        return -1;
    }

    // First, check if we have to add new item
    if (graphicsSet.GraphicSetID < 0) {
        return -1;
    }

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM GraphicsSets WHERE GraphicSetID=(?)");
    checkStmt->setInt(1, graphicsSet.GraphicSetID);

    sql::ResultSet* res = checkStmt->executeQuery();

    if (res->next()) {
        int id = res->getInt(1);

        delete res;
        delete checkStmt;

        return id;
    } else
    {
        return AddGraphicsSet(graphicsSet);
    }
}

bool DbConnector::DeleteGraphicsSet(uint32_t id) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error deleting graphics set: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("DELETE FROM GraphicsSets"
                                                          " WHERE GraphicSetID=?");
    stmt->setInt(1, id);
    stmt->executeQuery();

    // Check that there's no more graphics set with this id
    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM GraphicsSets"
                                                               " WHERE GraphicSetID=(?)");
    checkStmt->setInt(1, id);
    sql::ResultSet* res = checkStmt->executeQuery();

    return !res->next();
}

// GraphicsSetsActors
bool DbConnector::GetAllGraphicsSetsActors(std::vector<GraphicsSetsActor> &graphicsSetsActors) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if(driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting archetypes: connection not initialized" << std::endl;
        return false;
    }

    sql::Statement* stmt = conn->createStatement();
    sql::ResultSet* res = stmt->executeQuery("SELECT * FROM GraphicsSetsActors");

    // Get struct from query results
    while (res->next()) {
        GraphicsSetsActor graphicsSetsActor(res->getInt(1),
                                            res->getInt(2));
        graphicsSetsActors.push_back(graphicsSetsActor);
    }

    return true;
}

bool DbConnector::GetGraphicsSetsActorsByGSID(uint32_t id, std::vector<GraphicsSetsActor> &graphicsSetsActors) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if(driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting archetypes: connection not initialized" << std::endl;
        return false;
    }

    // Prepare query
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM GraphicsSetsActors"
                                                          " WHERE GraphicSetID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    while(res->next()) {
        GraphicsSetsActor graphicsSetsActor(res->getInt(1),
                                            res->getInt(2));

        graphicsSetsActors.push_back(graphicsSetsActor);
    }

    return true;
}

bool DbConnector::GetGraphicsSetsActorsByActorID(uint32_t id, std::vector<GraphicsSetsActor> &graphicsSetsActors) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if(driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting archetypes: connection not initialized" << std::endl;
        return false;
    }

    // Prepare query
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM GraphicsSetsActors"
                                                          " WHERE ActorID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    while(res->next()) {
        GraphicsSetsActor graphicsSetsActor(res->getInt(1),
                                            res->getInt(2));

        graphicsSetsActors.push_back(graphicsSetsActor);
    }

    return true;
}

int DbConnector::UpdateGraphicsSetsActors(GraphicsSetsActor &graphicsSetsActor) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if(driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting archetypes: connection not initialized" << std::endl;
        return -1;
    }

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM GraphicsSetsActors"
                                                          " WHERE GraphicSetID=?"
                                                          " AND ActorID=?");
    checkStmt->setInt(1, graphicsSetsActor.GraphicSetID);
    checkStmt->setInt(2, graphicsSetsActor.ActorID);

    sql::ResultSet* res = checkStmt->executeQuery();

    if(!res->next())
        return AddGraphicsSetsActor(graphicsSetsActor);
    else
        return FindGraphicsSetsActorByFields(graphicsSetsActor);
}

bool DbConnector::DeleteGraphicsSetsActorsByGSID(uint32_t id) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error deleting archetype: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("DELETE FROM GraphicsSetsActors"
                                                          " WHERE GraphicSetID=?");
    stmt->setInt(1, id);
    stmt->executeQuery();

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM GraphicsSetsActors"
                                                               " WHERE GraphicSetID=(?)");
    checkStmt->setInt(1, id);
    sql::ResultSet* res = checkStmt->executeQuery();

    return !res->next();
}

bool DbConnector::DeleteGraphicsSetsActorsByActorID(uint32_t id) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error deleting archetype: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("DELETE FROM GraphicsSetsActors"
                                                          " WHERE ActorID=?");
    stmt->setInt(1, id);
    stmt->executeQuery();

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM GraphicsSetsActors"
                                                               " WHERE ActorID=(?)");
    checkStmt->setInt(1, id);
    sql::ResultSet* res = checkStmt->executeQuery();

    return !res->next();
}

// GraphicsSetsProducts
bool DbConnector::GetAllGraphicsSetsProducts(std::vector<GraphicsSetsProduct> &graphicsSetsProducts) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting graphics sets: connection not initialized" << std::endl;
        return false;
    }

    sql::Statement* stmt = conn->createStatement();
    sql::ResultSet* res = stmt->executeQuery("SELECT * FROM GraphicsSetsProducts");

    // Get struct from query results
    while(res->next()) {
        GraphicsSetsProduct graphicsSetsProduct(res->getInt(1),
                                                res->getInt(2),
                                                res->getInt(3));
        graphicsSetsProducts.push_back(graphicsSetsProduct);
    }

    return true;
}

bool DbConnector::GetGraphicsSetsProductById(uint32_t id, GraphicsSetsProduct &graphicsSetsProduct) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting graphics sets: connection not initialized" << std::endl;
        return false;
    }

    // Prepare query
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM GraphicsSetsProducts"
                                                          " WHERE GraphicSetID=?");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    if(res->next()) {
        graphicsSetsProduct.SetData(res->getInt(1),
                                    res->getInt(2),
                                    res->getInt(3));

        return true;
    }

    return false;
}

bool DbConnector::GetGraphicSetProductByPAID(uint32_t id, GraphicsSetsProduct &graphicSetsProduct) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting graphics sets: connection not initialized" << std::endl;
        return false;
    }

    // Prepare query
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM GraphicsSetsProducts"
                                                          " WHERE PAID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    if(res->next()) {
        graphicSetsProduct.SetData(res->getInt(1),
                                    res->getInt(2),
                                    res->getInt(3));

        return true;
    }

    return false;
}

int DbConnector::UpdateGraphicsSetsProduct(GraphicsSetsProduct &graphicsSetsProduct) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting graphics sets: connection not initialized" << std::endl;
        return -1;
    }

    if(graphicsSetsProduct.GraphicSetID < 0)
    {
        sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM GraphicsSetsProducts"
                                                                   " WHERE GraphicSetID=?");
        checkStmt->setInt(1, graphicsSetsProduct.GraphicSetID);
        sql::ResultSet* res = checkStmt->executeQuery();

        if(!res->next())
            return AddGraphicsSetsProduct(graphicsSetsProduct);
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("UPDATE GraphicsSetsProducts"
                                                          " SET"
                                                          " PAID=?,"
                                                          " GraphicSetClientID=?"
                                                          " WHERE GraphicSetID=?");
    stmt->setInt(1, graphicsSetsProduct.PAID);
    stmt->setInt(2, graphicsSetsProduct.GraphicSetClientID);
    stmt->setInt(3, graphicsSetsProduct.GraphicSetID);

    stmt->executeQuery();

    return FindGraphicsSetsProductByFields(graphicsSetsProduct);
}

bool DbConnector::DeleteGraphicsSetsProduct(uint32_t id) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting graphics sets: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("DELETE FROM GraphicsSetsProducts"
                                                          " WHERE GraphicSetID=?");
    stmt->setInt(1, id);
    stmt->executeQuery();

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM GraphicsSetsProducts"
                                                               " WHERE GraphicSetID=(?)");
    checkStmt->setInt(1, id);
    sql::ResultSet* res = checkStmt->executeQuery();

    return !res->next();
}


bool DbConnector::GetAllInstances(std::vector<ProductInstance>& instances) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting instances: connection not initialized" << std::endl;
        return false;
    }

    sql::Statement* stmt = conn->createStatement();
    sql::ResultSet* res = stmt->executeQuery("SELECT * FROM ProductInstances");

    // Get structs from query results
    while (res->next()) {
        ProductInstance instance(res->getInt(1),
                                 res->getInt(2),
                                 res->getDouble(3),
                                 res->getDouble(4),
                                 res->getDouble(5),
                                 res->getInt(6),
                                 res->getString(7),
                                 res->getString(8));
        instances.push_back(instance);
    }

    return true;
}

bool DbConnector::GetProductInstancesBySID(uint32_t id, std::vector<ProductInstance> &instances) {
    std::vector<SpaceActor> actors;
    GetSpaceActorBySID(id, actors);

    for(auto actor : actors)
    {
        ProductInstance instance;
        if(GetInstanceById(actor.PIID, instance))
        {
            instances.push_back(instance);

            std::vector<ContainerInstance> containers;
            if(GetContainerInstancesByPIID(instance.PIID, containers))
            {
                for(auto container : containers)
                {
                    std::vector<ProductsInContainer> products;
                    GetProductsInContainerById(container.CIID, products);

                    for(auto prod : products)
                    {
                        ProductInstance productInContainer;
                        if(GetInstanceById(prod.PIID, productInContainer))
                            instances.push_back(productInContainer);
                    }
                }
            }
        }
    }

    std::vector<Character> characters;
    GetAllCharacters(characters);

    for(auto character : characters)
    {
        PlayerInventory inventory;
        if(GetPlayerInventoryByID(character.CharacterID, inventory))
        {
            std::vector<ProductsInContainer> products;
            GetProductsInContainerById(inventory.CIID, products);

            for(auto prod : products)
            {
                ProductInstance productInContainer;
                if(GetInstanceById(prod.PIID, productInContainer))
                    instances.push_back(productInContainer);
            }
        }
    }

    return true;
}

bool DbConnector::GetInstanceById(uint32_t id, ProductInstance& instance) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting instance: connection not initialized" << std::endl;
        return false;
    }

    // Prepare query
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ProductInstances"
                                                          " WHERE PIID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    if (res->next()) {
        instance.SetData(res->getInt(1),
                         res->getInt(2),
                         res->getDouble(3),
                         res->getDouble(4),
                         res->getDouble(5),
                         res->getInt(6),
                         res->getString(7),
                         res->getString(8));

        return true;
    }

    return false;
}

int DbConnector::UpdateInstance(ProductInstance &instance) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error updating instance: connection not initialized" << std::endl;
        return false;
    }


    // First, check if we have to add new item
    if (instance.PIID < 0) {
        sql::PreparedStatement *checkStmt = conn->prepareStatement("SELECT * FROM ProductInstances"
                                                                   " WHERE PIID=?");
        checkStmt->setInt(1, instance.PIID);
        sql::ResultSet* res = checkStmt->executeQuery();

        if (!res->next()) {
            return AddInstance(instance);
        }
    }

    // Now, we have to update its values
    // Prepare query
    sql::PreparedStatement* stmt = conn->prepareStatement("UPDATE ProductInstances"
                                                          " SET"
                                                          " PAID=?,"
                                                          " Mass=?,"
                                                          " Volume=?,"
                                                          " Quality=?,"
                                                          " CurrentGraphicSetID=?,"
                                                          " MetaData=?,"
                                                          " PresenceType=?"
                                                          " WHERE PIID=(?)");
    stmt->setInt(1, instance.PAID);
    stmt->setDouble(2, instance.mass);
    stmt->setDouble(3, instance.volume);
    stmt->setDouble(4, instance.quality);
    stmt->setInt(5, instance.CurrentGSID);
    stmt->setString(6, instance.meta);
    stmt->setString(7, instance.GetTypeString());
    stmt->setInt(8, instance.PIID);

    stmt->executeQuery();

    // Return new archetype id
    return FindInstanceByFields(instance);
}

bool DbConnector::DeleteInstance(uint32_t id) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error deleting instance: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement *stmt = conn->prepareStatement("DELETE FROM ProductInstances"
                                                          " WHERE PIID=?");
    stmt->setInt(1, id);
    stmt->executeQuery();

    // Check that there's no more instance with this id
    sql::PreparedStatement *checkStmt = conn->prepareStatement("SELECT * FROM ProductInstances"
                                                               " WHERE PIID=(?)");
    checkStmt->setInt(1, id);
    sql::ResultSet *res = checkStmt->executeQuery();

    return !res->next();
}

// ContainerInstances
bool DbConnector::GetAllContainerInstances(std::vector<ContainerInstance> &instances) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting instances: connection not initialized" << std::endl;
        return false;
    }

    sql::Statement* stmt = conn->createStatement();
    sql::ResultSet* res = stmt->executeQuery("SELECT * FROM ContainerInstances");

    while(res->next()) {
        ContainerInstance instance(res->getInt(1),
                                   res->getInt(2),
                                   res->getInt(3),
                                   res->getInt(4));

        instances.push_back(instance);
    }

    return true;
}

bool DbConnector::GetContainerInstancesByPIIDs(std::vector<ProductInstance> Products, std::vector<ContainerInstance> &instances) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting instances: connection not initialized" << std::endl;
        return false;
    }

    for(auto Product : Products)
    {
        sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ContainerInstances"
                                      " WHERE PIID=(?)");
        stmt->setInt(1, Product.PIID);

        sql::ResultSet* res = stmt->executeQuery();
        if(res->next())
        {
            ContainerInstance instance(res->getInt(1),
                                       res->getInt(2),
                                       res->getInt(3),
                                       res->getInt(4));

            instances.push_back(instance);

            delete stmt;
            delete res;
        }
    }

    return true;
}

bool DbConnector::GetContainerInstancesByPIID(uint32_t id, std::vector<ContainerInstance>& instances)
{
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();


    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting instances: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ContainerInstances"
                                                          " WHERE PIID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    while(res->next())
    {
        ContainerInstance instance(res->getInt(1),
                                   res->getInt(2),
                                   res->getInt(3),
                                   res->getInt(4));

        instances.push_back(instance);
    }

    return true;
}

bool DbConnector::GetContainerInstanceById(uint32_t id, ContainerInstance &instance) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();


    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting instances: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ContainerInstances"
                                                          " WHERE CIID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    if(res->next()) {
        instance.SetData(res->getInt(1),
                         res->getInt(2),
                         res->getInt(3),
                         res->getInt(4));

        return true;
    }

    return false;
}

int DbConnector::UpdateContainerInstance(ContainerInstance &instance) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting instances: connection not initialized" << std::endl;
        return -1;
    }

    if (instance.CIID < 0) {
        sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM ContainerInstances"
                                                                   " WHERE CIID=?");
        checkStmt->setInt(1, instance.CIID);
        sql::ResultSet* res = checkStmt->executeQuery();

        if(!res->next())
            return AddContainerInstance(instance);
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("UPDATE ContainerInstances"
                                                          " SET"
                                                          " CAID=?,"
                                                          " PIID=?"
                                                          " WHERE CIID=(?)");
    stmt->setInt(1, instance.CAID);
    stmt->setInt(2, instance.PIID);
    stmt->setInt(3, instance.CIID);

    stmt->executeQuery();

    return FindContainerInstanceByFields(instance);
}

bool DbConnector::DeleteContainerInstance(uint32_t id) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting instances: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("DELETE FROM ContainerInstances"
                                                          " WHERE CIID=?");
    stmt->setInt(1, id);
    stmt->executeQuery();

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM ContainerInstances"
                                                               " WHERE CIID=(?)");
    checkStmt->setInt(1, id);
    sql::ResultSet* res = checkStmt->executeQuery();

    return !res->next();
}

bool DbConnector::GetAllWorkbenchInstances(std::vector<WorkbenchInstance> &instances) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting workbench instances: connection not initialized" << std::endl;
        return false;
    }

    sql::Statement* stmt = conn->createStatement();
    sql::ResultSet* res = stmt->executeQuery("SELECT * FROM WorkbenchInstances");

    while(res->next())
    {
        WorkbenchInstance instance(res->getInt(1),
                                   res->getInt(2),
                                   res->getInt(3));

        instances.push_back(instance);
    }

    return true;
}

bool DbConnector::GetWorkbenchInstanceByPIID(uint32_t id, WorkbenchInstance &instance) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting workbench instance: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM WorkbenchInstances"
                                                          " WHERE PIID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        instance.SetData(res->getInt(1),
                         res->getInt(2),
                         res->getInt(3));

        return true;
    }

    return false;
}

bool DbConnector::GetWorkbenchInstanceByID(uint32_t id, WorkbenchInstance &instance) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error getting workbench instance: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM WorkbenchInstances"
                                                          " WHERE WBID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        instance.SetData(res->getInt(1),
                         res->getInt(2),
                         res->getInt(3));

        return true;
    }

    return false;
}

int DbConnector::UpdateWorkbenchInstance(WorkbenchInstance &instance) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error updating workbench instance: connection not initialized" << std::endl;
        return -1;
    }

    if(instance.WBID < 0)
    {
        sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM WorkbenchInstances"
                                                                   " WHERE WBID=(?)");
        checkStmt->setInt(1, instance.WBID);
        sql::ResultSet* res = checkStmt->executeQuery();

        if(!res->next())
            return AddWorkbenchInstance(instance);
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("UPDATE WorkbenchInstances"
                                                          " SET"
                                                          " WBAID=?,"
                                                          " PIID=?"
                                                          " WHERE WBID=(?)");
    stmt->setInt(1, instance.WBAID);
    stmt->setInt(2, instance.PIID);
    stmt->setInt(3, instance.WBID);

    stmt->executeQuery();

    return FindWorkbenchInstanceByFields(instance);
}

bool DbConnector::DeleteWorkbenchInstance(uint32_t id) {
    return false;
}

bool DbConnector::GetAllWorkbenchSchemes(std::vector<WorkbenchScheme> &schemes) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM WorkbenchSchemes");

    sql::ResultSet* res = stmt->executeQuery();

    while(res->next())
    {
        WorkbenchScheme scheme(res->getInt(1),
                               res->getInt(2),
                               res->getString(3));

        schemes.push_back(scheme);
    }

    return true;
}

bool DbConnector::GetWorkbenchSchemeByID(uint32_t id, WorkbenchScheme &scheme) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM WorkbenchSchemes"
                                                          " WHERE SchemeID=(?)");
    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        scheme.SetData(res->getInt(1),
                       res->getInt(2),
                       res->getString(3));

        return true;
    }

    return false;
}

bool DbConnector::GetAllConversionProductsWorkbenchSchemes(std::vector<ConversionProductsWorkbenchScheme> &schemes) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ConversionProductsWorkbenchSchemes");

    sql::ResultSet* res = stmt->executeQuery();

    while(res->next())
    {
        ConversionProductsWorkbenchScheme scheme(res->getInt(1),
                                                 res->getInt(2),
                                                 res->getInt(3),
                                                 res->getDouble(4),
                                                 res->getDouble(5));

        schemes.push_back(scheme);
    }

    return true;
}

bool DbConnector::GetConversionProductsWorkbenchSchemesBySchemeID(uint32_t id,
                                                                  std::vector<ConversionProductsWorkbenchScheme> &schemes) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ConversionProductsWorkbenchSchemes"
                                                          " WHERE SchemeID=(?)");
    stmt->setInt(1, id);

    sql::ResultSet* res = stmt->executeQuery();

    while(res->next())
    {
        ConversionProductsWorkbenchScheme scheme(res->getInt(1),
                                                 res->getInt(2),
                                                 res->getInt(3),
                                                 res->getDouble(4),
                                                 res->getDouble(5));

        schemes.push_back(scheme);
    }

    return true;
}

bool DbConnector::GetConversionProductsWorkbenchSchemeByID(uint32_t id, ConversionProductsWorkbenchScheme &scheme) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ConversionProductsWorkbenchSchemes"
                                                          " WHERE SchemeID=(?)");
    stmt->setInt(1, id);

    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        scheme.SetData(res->getInt(1),
                       res->getInt(2),
                       res->getInt(3),
                       res->getDouble(4),
                       res->getDouble(5));

        return true;
    }

    return false;
}

bool DbConnector::GetAllConversionSourcesWorkbenchSchemes(std::vector<ConversionSourcesWorkbenchScheme> &schemes) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ConversionSourcesWorkbenchSchemes");

    sql::ResultSet* res = stmt->executeQuery();

    while(res->next())
    {
        ConversionSourcesWorkbenchScheme scheme(res->getInt(1),
                                                res->getInt(2),
                                                res->getInt(3),
                                                res->getDouble(4));

        schemes.push_back(scheme);
    }

    return true;
}

bool DbConnector::GetConversionSourcesWorkbenchSchemesBySchemeID(uint32_t id,
                                                                 std::vector<ConversionSourcesWorkbenchScheme> &schemes) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ConversionSourcesWorkbenchSchemes"
                                                          " WHERE SchemeID=(?)");

    stmt->setInt(1, id);

    sql::ResultSet* res = stmt->executeQuery();

    while(res->next())
    {
        ConversionSourcesWorkbenchScheme scheme(res->getInt(1),
                                                res->getInt(2),
                                                res->getInt(3),
                                                res->getDouble(4));

        schemes.push_back(scheme);
    }

    return true;
}

bool DbConnector::GetConversionSourcesWorkbenchSchemeByID(uint32_t id, ConversionSourcesWorkbenchScheme &scheme) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ConversionSourcesWorkbenchSchemes"
                                                          " WHERE SchemeID=(?)");
    stmt->setInt(1, id);

    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        scheme.SetData(res->getInt(1),
                       res->getInt(2),
                       res->getInt(3),
                       res->getDouble(4));

        return true;
    }

    return false;
}

bool DbConnector::GetAllConversionWorkbenchSchemes(std::vector<ConversionWorkbenchScheme> &schemes) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ConversionWorkbenchSchemes");

    sql::ResultSet* res = stmt->executeQuery();

    while(res->next())
    {
        ConversionWorkbenchScheme scheme(res->getInt(1),
                                         res->getDouble(2));

        schemes.push_back(scheme);
    }

    return true;
}

bool DbConnector::GetConversionWorkbenchSchemeByID(uint32_t id, ConversionWorkbenchScheme &scheme) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ConversionWorkbenchSchemes"
                                                          " WHERE SchemeID=(?)");
    stmt->setInt(1, id);

    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        scheme.SetData(res->getInt(1),
                       res->getDouble(2));

        return true;
    }

    return false;
}

bool DbConnector::GetAllWorkbenchComponents(std::vector<WorkbenchComponent> &components) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM WorkbenchComponents");

    sql::ResultSet* res = stmt->executeQuery();

    while(res->next())
    {
        WorkbenchComponent component(res->getInt(1),
                                     res->getBoolean(2),
                                     res->getBoolean(3),
                                     res->getInt(4));

        components.push_back(component);
    }

    return true;
}

bool DbConnector::GetWorkbenchComponentByID(uint32_t id, WorkbenchComponent &component) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM WorkbenchComponents"
                                                          " WHERE WBCID=(?)");

    stmt->setInt(1, id);

    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        component.SetData(res->getInt(1),
                          res->getBoolean(2),
                          res->getBoolean(3),
                          res->getInt(4));

        return true;
    }

    return false;
}

bool DbConnector::GetAllWorkbenchComponentSlots(std::vector<WorkbenchComponentSlot> &components) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM WorkbenchComponentSlots");

    sql::ResultSet* res = stmt->executeQuery();

    while(res->next())
    {
        WorkbenchComponentSlot component(res->getInt(1),
                                         res->getInt(2));

        components.push_back(component);
    }

    return true;
}

bool DbConnector::GetWorkbenchComponentSlotsByWBCID(uint32_t id, std::vector<WorkbenchComponentSlot> &components) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM WorkbenchComponentSlots"
                                                          " WHERE WBCID=(?)");
    stmt->setInt(1, id);

    sql::ResultSet* res = stmt->executeQuery();

    while(res->next())
    {
        WorkbenchComponentSlot component(res->getInt(1),
                                         res->getInt(2));

        components.push_back(component);
    }

    return true;
}

bool DbConnector::GetWorkbenchComponentSlotByID(uint32_t id, WorkbenchComponentSlot &component) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM WorkbenchComponentSlots"
                                                          " WHERE WBSID=(?)");

    stmt->setInt(1, id);

    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        component.SetData(res->getInt(1),
                          res->getInt(2));

        return true;
    }

    return false;
}

bool DbConnector::GetAllContainersInWorkbenchArchetype(std::vector<ContainersInWorkbenchArchetype> &containers) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ContainersInWorkbenchArchetype");

    sql::ResultSet* res = stmt->executeQuery();

    while(res->next())
    {
        ContainersInWorkbenchArchetype container(res->getInt(1),
                                                 res->getInt(2),
                                                 res->getString(3));

        containers.push_back(container);
    }

    return true;
}

bool DbConnector::GetContainersInWorkbenchArchetypeByID(uint32_t id,
                                                        std::vector<ContainersInWorkbenchArchetype> &containers) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ContainersInWorkbenchArchetype"
                                                          " WHERE WBAID=(?)");
    stmt->setInt(1, id);

    sql::ResultSet* res = stmt->executeQuery();

    while(res->next())
    {
        ContainersInWorkbenchArchetype container(res->getInt(1),
                                                 res->getInt(2),
                                                 res->getString(3));

        containers.push_back(container);
    }

    return true;
}

bool DbConnector::GetAllWorkbenchComponentsInWorkbenchArchetype(
        std::vector<WorkbenchComponentInWorkbenchArchetype> &components) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM WorkbenchComponentInWorkbenchArchetype");

    sql::ResultSet* res = stmt->executeQuery();

    while(res->next())
    {
        WorkbenchComponentInWorkbenchArchetype component(res->getInt(1),
                                                         res->getInt(2));

        components.push_back(component);
    }

    return true;
}

bool DbConnector::GetWorkbenchComponentInWorkbenchArchetypeByID(uint32_t id,
                                                                WorkbenchComponentInWorkbenchArchetype &component) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM WorkbenchComponentInWorkbenchArchetype"
                                                          " WHERE WBAID=(?)");

    stmt->setInt(1, id);

    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        component.SetData(res->getInt(1),
                          res->getInt(2));

        return true;
    }

    return false;
}



bool DbConnector::CheckConversionProcess(ConversionInfo info) {
    std::map<int, std::list<WorkbenchSlotInfo>>::iterator status_it;
    std::list<WorkbenchSlotInfo>::iterator slot_it;
    status_it = WorkbenchSlotInfoMap.find(info.WBID);
    if (status_it != WorkbenchSlotInfoMap.end()) {
        for (slot_it = status_it->second.begin(); slot_it != status_it->second.end(); slot_it++) {
            if (slot_it->WBSID == info.WBSID) {
                if (slot_it->status != Ready)       // This will prevent checks if we get multiple calls with same info
                    return false;
                break;
            }
        }
    }

    std::vector<ConversionSourcesWorkbenchScheme> sources;
    GetConversionSourcesWorkbenchSchemesBySchemeID(info.SchemeID, sources);

    // Combine sources and product archetypes to blend items with same PAID
    std::unordered_map<int, std::vector<ConversionSourceStruct>> sourceMap;
    std::unordered_map<int, std::vector<ConversionSourceStruct>>::iterator source_it;

    for(auto value : sources)
    {
        source_it = sourceMap.find(value.PAID);
        if(source_it == sourceMap.end())
        {
            std::vector<ConversionSourceStruct> sourceStruct = {ConversionSourceStruct(value.ProductPAID, value.MassFraction)};
            sourceMap.emplace(value.PAID, sourceStruct);
        } else
        {
            source_it->second.push_back(ConversionSourceStruct(value.ProductPAID, value.MassFraction));
        }
    }

    std::vector<int> SourceKeys;

    for(source_it = sourceMap.begin(); source_it!=sourceMap.end(); ++source_it)
    {
        SourceKeys.push_back(source_it->first);
    }

    WorkbenchInstance workbenchInstance;
    if(!GetWorkbenchInstanceByID(info.WBID, workbenchInstance))
    {
        return false;
    }

    std::vector<ContainerInstance> containerInstances;
    GetContainerInstancesByPIID(workbenchInstance.PIID, containerInstances);

    // Get containers by type for different check procedures
    std::vector<ContainersInWorkbenchArchetype> WorkbenchContainersInfo;
    GetContainersInWorkbenchArchetypeByID(workbenchInstance.WBAID, WorkbenchContainersInfo);

    std::vector<ContainerInstance> InputContainers;
    std::vector<ContainerInstance> OutputContainers;
    std::vector<ContainerInstance> BothContainers;

    for(auto containerInfo : WorkbenchContainersInfo)
    {
        for(auto container : containerInstances)
        {
            if(container.LIID == containerInfo.CLIID)
            {
                if(containerInfo.Type == ContainerType::Output)
                    OutputContainers.push_back(container);
                if(containerInfo.Type != ContainerType::Output)
                    InputContainers.push_back(container);
                if(containerInfo.Type == ContainerType::Both)
                    BothContainers.push_back(container);
            }
        }
    }

    // Out containers (only OUT, not BOTH) must be empty
    for(auto& container : OutputContainers)
    {
        std::vector<ProductsInContainer> products;
        if(GetProductsInContainerById(container.CIID, products))
        {
            if(products.size() > 0)
                return false;
        }
    }

    // IN and BOTH containers (both in In array) are more complex
    // 1. They must not have any products that are not in scheme
    // 2. Unless thees products are locked by other already running scheme (in case of multi=processing)
    std::vector<int> SavedPAIDs;        // Used to safely include multiple items with same PAID

    std::map<int, float> totalMassMap;  // Also start gather masses for later mass-check

    // TODO make total input mass for each both containers!
    float totalInputMass = 0.f;
    bool bDataIsGood = false;

    std::list<int> localLockPIIDs;

    for(auto& container : InputContainers)
    {
        bDataIsGood = false;
        std::vector<ProductsInContainer> products;
        if(GetProductsInContainerById(container.CIID, products))
        {
            for(auto& product : products)
            {
                bDataIsGood = false;

                for (auto& elem : localLockPIIDs) {
                    if (product.PIID == elem) {
                        bDataIsGood = true;
                        break;
                    }
                }
                if (bDataIsGood)
                    continue;

                if (status_it != WorkbenchSlotInfoMap.end()) {
                    for (auto& slot : status_it->second) {
                        if (slot.HasPIID(product.PIID)) {
                            bDataIsGood = true;
                            break;
                        }
                    }
                }
                if (bDataIsGood)
                    continue;

                ProductInstance instance;
                if(GetInstanceById(product.PIID, instance))
                {
                    if(Find(SavedPAIDs, instance.PAID))
                    {
                        bDataIsGood = true;
                        localLockPIIDs.push_back(instance.PIID);
                        totalInputMass += instance.mass;
                        source_it = sourceMap.find(instance.PAID);
                        if(source_it != sourceMap.end())
                        {
                            for(auto& value : source_it->second)
                            {
                                if(totalMassMap.find(value.PAID) == totalMassMap.end())
                                {
                                    float mass = instance.mass * value.MF;
                                    totalMassMap.emplace(value.PAID, mass);
                                } else
                                {
                                    totalMassMap[value.PAID] += instance.mass * value.MF;
                                }
                            }
                        }
                        continue;
                    }

                    for(int Idx = 0; Idx < SourceKeys.size(); Idx++)
                    {
                        auto& source = SourceKeys[Idx];
                        if(source == instance.PAID)
                        {
                            bDataIsGood = true;
                            SourceKeys.erase(SourceKeys.begin() + Idx);
                            SavedPAIDs.push_back(source);

                            localLockPIIDs.push_back(instance.PIID);

                            totalInputMass += instance.mass;

                            source_it = sourceMap.find(instance.PAID);
                            if(source_it != sourceMap.end())
                            {
                                for(auto& value : source_it->second)
                                {
                                    if(totalMassMap.find(value.PAID) == totalMassMap.end())
                                    {
                                        float mass = instance.mass * value.MF;
                                        totalMassMap.emplace(value.PAID, mass);
                                    } else
                                    {
                                        totalMassMap[value.PAID] += instance.mass * value.MF;
                                    }
                                }
                            }
                            break;
                        }
                    }
                }

                if(!bDataIsGood)
                    break;
            }

            if(!bDataIsGood)
                break;
        }
    }

    if(!bDataIsGood || SourceKeys.size() > 0)
        return false;

    // Now do mass-check. Total mass is sorted by descension, so we first try to put in heavy objects
    SortByValue(totalMassMap);
    // Collect containers by type
    std::unordered_map<ProductArchetypeType, std::vector<ContainerPhantomCheckData>> containerMap;

    containerMap.emplace(ProductArchetypeType::Solid, std::vector<ContainerPhantomCheckData>());
    containerMap.emplace(ProductArchetypeType::Liquid, std::vector<ContainerPhantomCheckData>());
    containerMap.emplace(ProductArchetypeType::Grainy, std::vector<ContainerPhantomCheckData>());
    containerMap.emplace(ProductArchetypeType::Gaseous, std::vector<ContainerPhantomCheckData>());

    for(auto& container : OutputContainers)
    {
        ContainerArchetype archetype;
        if(GetContainerArchetypeById(container.CAID, archetype))
        {
            // Pure OUT containers will be empty, so we can simply caount it as is
            ContainerPhantomCheckData check(container, 0.f, 0.f);
            if(HasFlag(archetype.availableTypes, ProductArchetypeType::Solid))
            {
                containerMap[ProductArchetypeType::Solid].push_back(check);
            }
            if(HasFlag(archetype.availableTypes, ProductArchetypeType::Liquid))
            {
                containerMap[ProductArchetypeType::Liquid].push_back(check);
            }
            if(HasFlag(archetype.availableTypes, ProductArchetypeType::Grainy))
            {
                containerMap[ProductArchetypeType::Grainy].push_back(check);
            }
            if(HasFlag(archetype.availableTypes, ProductArchetypeType::Gaseous))
            {
                containerMap[ProductArchetypeType::Gaseous].push_back(check);
            }
        }
    }

    for(auto& it : BothContainers)
    {
        ContainerArchetype archetype;
        if(GetContainerArchetypeById(it.CAID, archetype))
        {
            // Pure BOTH containers can contain items (f.e. locked by other process)
            // so we have to gather it mass and volume
            float currentMass = 0.f;
            float currentVolume = 0.f;

            std::vector<ProductsInContainer> products;
            if(GetProductsInContainerById(it.CIID, products))
            {
                for(auto& prodIt : products)
                {
                    ProductInstance instance;
                    if(GetInstanceById(prodIt.PIID, instance))
                    {
                        currentMass += instance.mass;
                        currentVolume += instance.volume;
                    }
                }
            }

            currentMass -= totalInputMass;

            ContainerPhantomCheckData check(it, currentMass, currentVolume);
            if(HasFlag(archetype.availableTypes, ProductArchetypeType::Solid))
            {
                containerMap[ProductArchetypeType::Solid].push_back(check);
            }
            if(HasFlag(archetype.availableTypes, ProductArchetypeType::Liquid))
            {
                containerMap[ProductArchetypeType::Liquid].push_back(check);
            }
            if(HasFlag(archetype.availableTypes, ProductArchetypeType::Grainy))
            {
                containerMap[ProductArchetypeType::Grainy].push_back(check);
            }
            if(HasFlag(archetype.availableTypes, ProductArchetypeType::Gaseous))
            {
                containerMap[ProductArchetypeType::Gaseous].push_back(check);
            }
        }
    }

    // And now check all possible output mass - we must be able to put it in
    for(auto& it : totalMassMap)
    {
        bDataIsGood = false;
        ProductArchetype archetype;
        if(GetArchetypeById(it.first, archetype))
        {
            bool bSolid = archetype.type == ProductArchetypeType::Solid;

            float itMass = it.second;
            float itVolume = it.second / archetype.density;

            for(auto& conIt : containerMap[archetype.type])
            {
                ContainerArchetype conArchetype;
                if(GetContainerArchetypeById(conIt.Container.CAID, conArchetype))
                {
                    float MaxMass = conArchetype.maxMass;
                    float MaxVolume = conArchetype.maxVolume;

                    if(bSolid)
                    {
                        if(conIt.phMass + itMass > MaxMass)
                            continue;

                        if(conIt.phVolume + itVolume > MaxVolume)
                            continue;

                        conIt.phVolume += itVolume;
                        conIt.phMass += itMass;
                    } else
                    {
                        if(conIt.phMass + itMass > MaxMass)
                        {
                            if(conIt.phVolume + itVolume > MaxVolume)
                            {
                                float deltaV = MaxVolume - conIt.phVolume;
                                float deltaM = deltaV * archetype.density;

                                itVolume -= deltaV;
                                itMass -= deltaM;

                                conIt.phMass += deltaM;
                                conIt.phVolume = MaxVolume;
                                continue;
                            } else
                            {
                                float deltaM = MaxMass - conIt.phMass;
                                float deltaV = deltaM / archetype.density;

                                itMass -= deltaM;
                                itVolume -= deltaV;

                                conIt.phMass = MaxMass;
                                conIt.phVolume = deltaV;
                                continue;
                            }
                        } else
                        {
                            if(conIt.phVolume + itVolume > MaxVolume)
                            {
                                float deltaV = MaxVolume - conIt.phVolume;
                                float deltaM = deltaV * archetype.density;

                                itVolume -= deltaV;
                                itMass -= deltaM;

                                conIt.phMass += deltaM;
                                conIt.phVolume = MaxVolume;
                                continue;
                            } else
                            {
                                conIt.phMass += itMass;
                                conIt.phVolume += itVolume;
                            }
                        }
                    }
                    bDataIsGood = true;
                    break;
                }
            }
            if(!bDataIsGood)
                return false;
        }
    }

    if (status_it != WorkbenchSlotInfoMap.end()) {
        if (slot_it != status_it->second.end()) {
            // This actually never should happen
            slot_it->PIIDs = localLockPIIDs;
            slot_it->status = Process;
            std::cout << "WARNING: found slot " << info.WBID << ":" << info.WBSID
                      << " with status Ready. That should not happen.";
        } else {
            WorkbenchSlotInfo newInfo(info.WBSID, Process, localLockPIIDs);
            status_it->second.push_back(newInfo);
        }
    } else {
        WorkbenchSlotInfo newInfo(info.WBSID, Process, localLockPIIDs);
        WorkbenchSlotInfoMap.emplace(info.WBID, std::list<WorkbenchSlotInfo>{newInfo});
    }

    return true;
}

bool DbConnector::StartConversionProcess(ConversionInfo info) {
    std::map<int, std::list<WorkbenchSlotInfo>>::iterator status_it;
    std::list<WorkbenchSlotInfo>::iterator slot_it;
    status_it = WorkbenchSlotInfoMap.find(info.WBID);
    if (status_it == WorkbenchSlotInfoMap.end())
        return false;

    for (slot_it = status_it->second.begin(); slot_it != status_it->second.end(); slot_it++) {
        if (slot_it->WBSID == info.WBSID) {
            if (slot_it->status != Process)
                return false;
            break;
        }
    }
    if (slot_it == status_it->second.end())
        return false;

    std::vector<ConversionSourcesWorkbenchScheme> sources;
    GetConversionSourcesWorkbenchSchemesBySchemeID(info.SchemeID, sources);

    std::unordered_map<int, std::vector<ConversionSourceStruct>> sourceMap;
    std::unordered_map<int, std::vector<ConversionSourceStruct>>::iterator source_it;

    for(auto& value : sources)
    {
        source_it = sourceMap.find(value.PAID);
        if(source_it == sourceMap.end())
        {
            std::vector<ConversionSourceStruct> sourceStruct = {ConversionSourceStruct(value.ProductPAID, value.MassFraction)};
            sourceMap.emplace(value.PAID, sourceStruct);
        } else
        {
            source_it->second.push_back(ConversionSourceStruct(value.ProductPAID, value.MassFraction));
        }
    }

    std::vector<ConversionProductsWorkbenchScheme> products;
    GetConversionProductsWorkbenchSchemesBySchemeID(info.SchemeID, products);

    std::unordered_map<int, std::vector<ConversionProductStruct>> productsMap;
    std::unordered_map<int, std::vector<ConversionProductStruct>>::iterator prod_it;
    std::unordered_map<int, float> prodPRSum;       // Saved sum of PRs to use normalized value later
    std::unordered_map<int, float>::iterator PR_it;

    for (auto& prod : products)
    {
        prod_it = productsMap.find(prod.PAID);
        PR_it = prodPRSum.find(prod.PAID);
        if (prod_it == productsMap.end())
        {
            std::vector<ConversionProductStruct> prodStruct = {ConversionProductStruct(prod.ProductPAID, prod.PerfectRatio, prod.QualityMultiplier)};
            productsMap.emplace(prod.PAID, prodStruct);
            prodPRSum.emplace(prod.PAID, prod.PerfectRatio);
        }
        else
        {
            prod_it->second.push_back(ConversionProductStruct(prod.ProductPAID, prod.PerfectRatio, prod.QualityMultiplier));
            PR_it->second += prod.PerfectRatio;
        }
    }

    WorkbenchInstance workbenchInstance;
    if(!GetWorkbenchInstanceByID(info.WBID, workbenchInstance))
    {
        return false;
    }

    std::vector<ContainerInstance> containerInstances;
    GetContainerInstancesByPIID(workbenchInstance.PIID, containerInstances);

    // Get containers by type
    std::vector<ContainersInWorkbenchArchetype> WorkbenchContainersInfo;
    GetContainersInWorkbenchArchetypeByID(workbenchInstance.WBAID, WorkbenchContainersInfo);

    std::vector<ContainerInstance> InputContainers;
    std::vector<ContainerInstance> OutputContainers;

    for(auto containerInfo : WorkbenchContainersInfo)
    {
        for(auto container : containerInstances)
        {
            if(container.LIID == containerInfo.CLIID)
            {
                if(containerInfo.Type != ContainerType::Input)
                    OutputContainers.push_back(container);
                if(containerInfo.Type != ContainerType::Output)
                    InputContainers.push_back(container);
            }
        }
    }

    // Collect containers by type
    std::unordered_map<ProductArchetypeType, std::vector<ContainerInstance>> containerMap;

    containerMap.emplace(ProductArchetypeType::Solid, std::vector<ContainerInstance>());
    containerMap.emplace(ProductArchetypeType::Liquid, std::vector<ContainerInstance>());
    containerMap.emplace(ProductArchetypeType::Grainy, std::vector<ContainerInstance>());
    containerMap.emplace(ProductArchetypeType::Gaseous, std::vector<ContainerInstance>());

    for(auto& container : OutputContainers)
    {
        ContainerArchetype archetype;
        if(GetContainerArchetypeById(container.CAID, archetype))
        {
            // Pure OUT containers will be empty, so we can simply caount it as is
            ContainerPhantomCheckData check(container, 0.f, 0.f);
            if(HasFlag(archetype.availableTypes, ProductArchetypeType::Solid))
            {
                containerMap[ProductArchetypeType::Solid].push_back(container);
            }
            if(HasFlag(archetype.availableTypes, ProductArchetypeType::Liquid))
            {
                containerMap[ProductArchetypeType::Liquid].push_back(container);
            }
            if(HasFlag(archetype.availableTypes, ProductArchetypeType::Grainy))
            {
                containerMap[ProductArchetypeType::Grainy].push_back(container);
            }
            if(HasFlag(archetype.availableTypes, ProductArchetypeType::Gaseous))
            {
                containerMap[ProductArchetypeType::Gaseous].push_back(container);
            }
        }
    }

    // Generate result products
    /** PAID : {MASS, QUALITY} */
    std::map<int, std::pair<float, float>> newProductsMap;
    std::map<int, std::pair<float, float>>::iterator npm_it;
    for (auto& prod : productsMap)
        newProductsMap.emplace(prod.first, std::pair<float, float>(0.f, 0.f));

    // Add mass
    float totalConsumedMass = 0.f;
    for (auto& PIID : slot_it->PIIDs)
    {
        ProductInstance sourceItem;
        if (!GetInstanceById(PIID, sourceItem))
            return false;

        totalConsumedMass += sourceItem.mass;
        source_it = sourceMap.find(sourceItem.PAID);
        if (source_it == sourceMap.end())
        {
            std::cout << "ERROR: Attempt to use PAID " << sourceItem.PAID << " that doesn't exist in Sources!";
            return false;
        }

        for (auto& outer : source_it->second)
        {
            npm_it = newProductsMap.find(outer.PAID);
            if (npm_it == newProductsMap.end())
            {
                std::cout << "ERROR: ProductPAID " << outer.PAID << " doesn't gathered as Product!";
                return false;
            }

            npm_it->second.first += sourceItem.mass * outer.MF;
        }
    }

    // Add quality
    for (auto& prod : productsMap)
    {
        npm_it = newProductsMap.find(prod.first);
        if (npm_it == newProductsMap.end())
        {
            std::cout << "ERROR: ProductPAID " << prod.first << " doesn't gathered as Product!";
            return false;
        }

        PR_it = prodPRSum.find(prod.first);

        float PR = 0.f;
        int PRcount = 0;
        float QM = 1.f;
        float QM_sumQ = 0.f;
        float QM_sumBase = 0.f;
        for (auto& inner : prod.second)
        {
            float innerPR = 0.f;
            float massPAID = 0.f;
            float qualPAID = 0.f;
            int qualCount = 0;
            for (auto& src : slot_it->PIIDs)
            {
                ProductInstance inst;
                if (!GetInstanceById(src, inst))
                    return false;

                if (inner.PAID == inst.PAID)
                {
                    massPAID += inst.mass;
                    qualPAID += inst.quality;
                    qualCount++;
                }
            }
            qualPAID /= qualCount;

            QM_sumQ += massPAID * qualPAID * inner.QM;
            QM_sumBase += massPAID * inner.QM;

            if (inner.PR != 0.f)
            {
                innerPR = (inner.PR / PR_it->second) / (massPAID / totalConsumedMass);
                if (innerPR > 1.f)
                    innerPR = 1.f / innerPR;

                PRcount++;
                PR += innerPR;
            }
        }
        if (PRcount > 0)
            PR /= PRcount;
        QM = QM_sumQ / QM_sumBase;
        if (PR > 0.f)
            QM = (PR + QM) / 2.f;

        npm_it->second.second = QM;
    }

    SortByValue(newProductsMap);

    bool bNeedRecovery = false;
    std::vector<ProductsInContainer> recoveryData = {};
    std::vector<ProductsInContainer> recoveryInstance;
    for (auto& container : InputContainers)
    {
        if (!GetProductsInContainerById(container.CIID, recoveryInstance))
            return false;
        recoveryData.insert(recoveryData.end(), recoveryInstance.begin(), recoveryInstance.end());
    }

    for (auto& PIID : slot_it->PIIDs)
        DeleteProductsInContainer(PIID);

    std::list<int> localCompletedPIIDs;

    int localIdx = -1;
    for (npm_it = newProductsMap.begin(); npm_it != newProductsMap.end(); npm_it++)
    {
        localIdx++;
        ProductArchetype newArch;
        if (!GetArchetypeById(npm_it->first, newArch))
        {
            bNeedRecovery = true;
            break;
        }

        float goodMass = 0.f;
        for (auto& container : containerMap[newArch.type])
        {
            float volume = npm_it->second.first / newArch.density;

            GraphicsSetsProduct graphicSet;
            if (!GetGraphicSetProductByPAID(npm_it->first, graphicSet))
            {
                bNeedRecovery = true;
                break;
            }

            std::string meta = "Created item " +
                               std::to_string(npm_it->first) + ":" +
                               std::to_string(localIdx) + " by conversion process at Workbench " +
                               std::to_string(info.WBID) + ":" +
                               std::to_string(info.WBSID) + " from " +
                               std::to_string(*(slot_it->PIIDs.begin()));

            PresenceType prType = PresenceType::Container;

            ProductInstance newInstance = (ProductInstance(
                    -1,
                    newArch.PAID,
                    npm_it->second.first,
                    volume,
                    npm_it->second.second,
                    graphicSet.GraphicSetID,
                    meta,
                    prType));

            if (!CheckTransferPossible(newInstance, container.CIID, newInstance.mass, goodMass))
                continue;

            if (newArch.type == ProductArchetypeType::Solid)
            {
                newInstance.PIID = UpdateInstance(newInstance);
                TransferProduct(newInstance.PIID, container.CIID, 0.f);
            } else {
                if (goodMass == newInstance.mass)
                {
                    newInstance.PIID = UpdateInstance(newInstance);
                    TransferProduct(newInstance.PIID, container.CIID, 0.f);
                } else {
                    float remainedMass = newInstance.mass - goodMass;
                    auto next_it = npm_it;
                    next_it++;
                    std::pair<int, std::pair<float, float>> newPair (npm_it->first, {remainedMass, npm_it->second.second});
                    newProductsMap.insert(next_it, newPair);

                    newInstance.mass = goodMass;
                    newInstance.PIID = UpdateInstance(newInstance);
                    TransferProduct(newInstance.PIID, container.CIID, 0.f);
                }
            }

            localCompletedPIIDs.push_back(newInstance.PIID);

            break;
        }

        if (bNeedRecovery)
            break;
    }

    if (bNeedRecovery)
    {
        for (auto& Data : recoveryData)
        {
            TransferProduct(Data.PIID, Data.CIID, 0.f);
        }
        return false;
    }

    for (auto& PIID : slot_it->PIIDs)
    {
        DeleteInstance(PIID);
    }

    slot_it->status = Finish;
    slot_it->PIIDs = localCompletedPIIDs;

    return true;
}

bool DbConnector::GetAllContainerArchetypes(std::vector<ContainerArchetype> &containerArchetypes) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting container archetypes: connection not initialized" << std::endl;
        return false;
    }

    sql::Statement* stmt = conn->createStatement();
    sql::ResultSet* res = stmt->executeQuery("SELECT * FROM ContainerArchetypes");

    // Get structs from query results
    while (res->next())
    {
        ContainerArchetype containerArchetype(res->getInt(1),
                                              res->getString(2),
                                              res->getDouble(3),
                                              res->getDouble(4),
                                              res->getInt(5),
                                              res->getString(6));
        containerArchetypes.push_back(containerArchetype);
    }

    return true;
}

bool DbConnector::GetContainerArchetypeById(uint32_t id, ContainerArchetype &containerArchetype) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting container archetype: connection is not initialized" << std::endl;
        return false;
    }

    // Prepare query
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ContainerArchetypes"
                                                          " WHERE CAID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    if (res->next())
    {
        containerArchetype.SetData(res->getInt(1),
                                   res->getString(2),
                                   res->getDouble(3),
                                   res->getDouble(4),
                                   res->getInt(5),
                                   res->getString(6));

        return true;
    }

    return false;
}

int DbConnector::UpdateContainerArchetype(ContainerArchetype &containerArchetype) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error updating container archetype: connection is not initialized" << std::endl;
        return -1;
    }

    std::cout << "Update container archetype - " << containerArchetype.name << " with CAID - " << containerArchetype.CAID << std::endl;

    // First, check if we have to add new item
    if (containerArchetype.CAID < 0)
    {
        sql::PreparedStatement * checkStmt = conn->prepareStatement("SELECT * FROM ContainerArchetypes"
                                                                    " WHERE CAID=?");

        checkStmt->setInt(1, containerArchetype.CAID);
        sql::ResultSet* res = checkStmt->executeQuery();

        std::cout << "res is " << !res->next() << std::endl;

        if (!res->next())
        {
            return AddContainerArchetype(containerArchetype);
        }
    }

    // Now, we have to update its values
    // Prepare query
    sql::PreparedStatement* stmt = conn->prepareStatement("UPDATE ContainerArchetypes"
                                                          " SET"
                                                          " Name=?"
                                                          " MaxMass=?"
                                                          " MaxVolume=?"
                                                          " AvailableTypes=?"
                                                          " AvailableArchetypes=?"
                                                          " WHERE CAID=?");
    stmt->setString(1, containerArchetype.name);
    stmt->setInt(2, containerArchetype.maxMass);
    stmt->setInt(3, containerArchetype.maxVolume);
    stmt->setInt(4, containerArchetype.availableTypes);
    stmt->setString(5, containerArchetype.availableArchetypes);
    stmt->setInt(6, containerArchetype.CAID);

    stmt->executeQuery();

    // Return new container archetype id
    return FindContainerArchetypeByFields(containerArchetype);
}

bool DbConnector::DeleteContainerArchetype(uint32_t id) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error deleting container archetype: connection is not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("DELETE FROM ContainerArchetypes"
                                                          " WHERE CAID=?");
    stmt->setInt(1, id);
    stmt->executeQuery();

    // Check that there's no more container archetype set with this id
    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM ContainerArchetypes"
                                                               " WHERE CAID=(?)");
    checkStmt->setInt(1, id);
    sql::ResultSet* res = checkStmt->executeQuery();

    return !res->next();
}

bool DbConnector::GetAllWorkbenchArchetypes(std::vector<WorkbenchArchetype> &workbenchArchetypes) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting workbench archetypes: connection is not initialized" << std::endl;
        return false;
    }

    sql::Statement* stmt = conn->createStatement();
    sql::ResultSet* res = stmt->executeQuery("SELECT * FROM WorkbenchArchetypes");

    while(res->next())
    {
        WorkbenchArchetype workbenchArchetype(res->getInt(1),
                                              res->getInt(2));

        workbenchArchetypes.push_back(workbenchArchetype);
    }

    return true;
}

bool DbConnector::GetWorkbenchArchetypeByPAID(uint32_t id, WorkbenchArchetype &workbenchArchetype) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting workbench archetype: connection is not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM WorkbenchArchetypes"
                                                          " WHERE PAID=(?)");

    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        workbenchArchetype.SetData(res->getInt(1),
                                   res->getInt(2));

        return true;
    }

    return false;
}

bool DbConnector::DeleteWorkbenchArchetype(uint32_t id) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error deleting workbench archetype: connection is not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("DELETE * FROM WorkbenchArchetypes"
                                                          " WHERE PAID=(?)");
    stmt->setInt(1, id);
    stmt->executeQuery();

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM WorkbenchArchetypes"
                                                               " WHERE PAID=(?)");
    checkStmt->setInt(1, id);
    sql::ResultSet* res = checkStmt->executeQuery();

    return !res->next();
}

bool DbConnector::GetContainersInProductArchetypeByPAID(uint32_t id, ContainersInProductArchetype& productContainerArchetype) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting product/container archetype: connection is not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ContainersInProductArchetype"
                                                          " WHERE PAID = (?)");

    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        productContainerArchetype.SetData(res->getInt(1),
                                          res->getInt(2),
                                          res->getInt(3));

        return true;
    }

    return false;
}

// ContainersInActors
bool DbConnector::GetAllContainersInActors(std::vector<ContainersInActor> &containersInActors) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting container archetypes: connection not initialized" << std::endl;
        return false;
    }

    sql::Statement* stmt = conn->createStatement();
    sql::ResultSet* res = stmt->executeQuery("SELECT * FROM ContainersInActors");

    // Get struct from query results
    while(res->next())
    {
        ContainersInActor containersInActor(res->getInt(1),
                                            res->getInt(2));

        containersInActors.push_back(containersInActor);
    }

    return true;
}

bool DbConnector::GetContainersInActorById(uint32_t id, ContainersInActor &containersInActor) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting container archetypes: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ContainersInActors"
                                                          " WHERE CIID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        containersInActor.SetData(res->getInt(1),
                                  res->getInt(2));

        return true;
    }

    return false;
}

int DbConnector::UpdateContainersInActor(ContainersInActor &containersInActor) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting container archetypes: connection not initialized" << std::endl;
        return -1;
    }

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM ContainersInActors"
                                                               " WHERE CIID=?"
                                                               " AND ActorID=?");
    checkStmt->setInt(1, containersInActor.CIID);
    checkStmt->setInt(2, containersInActor.ActorID);

    sql::ResultSet* res = checkStmt->executeQuery();

    if(!res->next())
        return AddContainersInActor(containersInActor);
    else
        return FindContainersInActorByFields(containersInActor);
}

bool DbConnector::DeleteContainersInActor(uint32_t id) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting container archetypes: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("DELETE FROM ContainersInActors"
                                                          " WHERE CIID=?");
    stmt->setInt(1, id);
    stmt->executeQuery();

    // Check that there's no more containers in actor with this id
    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM ContainersInActors"
                                                               " WHERE CIID=(?)");
    checkStmt->setInt(1, id);
    sql::ResultSet* res = checkStmt->executeQuery();

    return !res->next();
}

// ProductsInActors
bool DbConnector::GetAllProductsInActors(std::vector<ProductsInActor> &productsInActors) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting container archetypes: connection not initialized" << std::endl;
        return false;
    }

    sql::Statement* stmt = conn->createStatement();
    sql::ResultSet* res = stmt->executeQuery("SELECT * FROM ProductsInActors");

    // Get struct from query results
    while(res->next())
    {
        ProductsInActor productsInActor(res->getInt(1),
                                        res->getInt(2));

        productsInActors.push_back(productsInActor);
    }

    return true;
}

bool DbConnector::GetProductsInActorById(uint32_t id, ProductsInActor &productsInActor) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting container archetypes: connection not initialized" << std::endl;
        return false;
    }

    // Prepare query
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ProductsInActors"
                                                          " WHERE PIID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        productsInActor.SetData(res->getInt(1),
                                res->getInt(2));

        return true;
    }

    return false;
}

int DbConnector::UpdateProductsInActor(ProductsInActor &productsInActor) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting container archetypes: connection not initialized" << std::endl;
        return -1;
    }

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM ProductsInActors"
                                                               " WHERE PIID=?"
                                                               " AND ActorID=?");
    checkStmt->setInt(1, productsInActor.PIID);
    checkStmt->setInt(2, productsInActor.ActorID);

    sql::ResultSet* res = checkStmt->executeQuery();

    if(!res->next())
        return AddProductsInActor(productsInActor);
    else
        return FindProductsInActor(productsInActor);
}

bool DbConnector::DeleteProductsInActor(uint32_t id) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting container archetypes: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("DELETE FROM ProductsInActors"
                                                          " WHERE PIID=?");
    stmt->setInt(1, id);
    stmt->executeQuery();

    // Check that there's no more products in actor with this id
    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM ProductsInActors"
                                                               " WHERE PIID=?");
    checkStmt->setInt(1, id);
    sql::ResultSet* res = checkStmt->executeQuery();

    return !res->next();
}

// ProductsInContainers
bool DbConnector::GetAllProductsInContainers(std::vector<ProductsInContainer> &productsInContainers) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting container archetypes: connection not initialized" << std::endl;
        return false;
    }

    sql::Statement* stmt = conn->createStatement();
    sql::ResultSet* res = stmt->executeQuery("SELECT * FROM ProductsInContainers");

    // Get struct from query results
    while(res->next())
    {
        ProductsInContainer productsInContainer(res->getInt(1),
                                                res->getInt(2));

        productsInContainers.push_back(productsInContainer);
    }

    return true;
}

bool DbConnector::GetProductsInContainerById(uint32_t id, std::vector<ProductsInContainer> &productsInContainer) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting container archetypes: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ProductsInContainers"
                                                          " WHERE CIID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    while(res->next())
    {
        ProductsInContainer productInContainer(res->getInt(1),
                                               res->getInt(2));

        productsInContainer.push_back(productInContainer);
    }

    return true;
}

bool DbConnector::GetProductInContainerByPIID(uint32_t id, ProductsInContainer& productInContainer) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting product in container: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ProductsInContainers"
                                                          " WHERE PIID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        productInContainer.SetData(res->getInt(1),
                                   res->getInt(2));

        return true;
    }

    return false;
}

int DbConnector::UpdateProductsInContainer(ProductsInContainer &productsInContainer) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting container archetypes: connection not initialized" << std::endl;
        return -1;
    }

    sql::PreparedStatement* checkExistStmt = conn->prepareStatement("SELECT * FROM ProductsInContainers"
                                                                    " WHERE PIID=?");
    checkExistStmt->setInt(1, productsInContainer.PIID);

    sql::ResultSet* checkRes = checkExistStmt->executeQuery();

    if(!checkRes->next())
        return AddProductsInContainer(productsInContainer);

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM ProductsInContainers"
                                                               " WHERE PIID=?"
                                                               " AND CIID=?");
    checkStmt->setInt(1, productsInContainer.PIID);
    checkStmt->setInt(2, productsInContainer.CIID);

    sql::ResultSet* res = checkStmt->executeQuery();

    if(!res->next())
    {
        sql::PreparedStatement* stmt = conn->prepareStatement("UPDATE ProductsInContainers"
                                                              " SET"
                                                              " CIID=?"
                                                              " WHERE PIID=(?)");
        stmt->setInt(1, productsInContainer.CIID);
        stmt->setInt(2, productsInContainer.PIID);

        stmt->executeQuery();

        return FindProductsInContainer(productsInContainer);
    }
    else
        return FindProductsInContainer(productsInContainer);
}

bool DbConnector::DeleteProductsInContainer(uint32_t id) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting container archetypes: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("DELETE FROM ProductsInContainers"
                                                          " WHERE PIID=?");
    stmt->setInt(1, id);
    stmt->executeQuery();

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM ProductsInContainers"
                                                               " WHERE PIID=?");
    checkStmt->setInt(1, id);
    sql::ResultSet* res = checkStmt->executeQuery();

    return !res->next();
}

bool DbConnector::GetAllProductsInCharacters(std::vector<ProductInCharacter> &productsInCharacters) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting products in characters: connection not initialized" << std::endl;
        return false;
    }

    sql::Statement* stmt = conn->createStatement();
    sql::ResultSet* res = stmt->executeQuery("SELECT * FROM ProductsInCharacters");

    while(res->next())
    {
        ProductInCharacter productInCharacter(res->getInt(1),
                                              res->getInt(2),
                                              res->getString(3));

        productsInCharacters.push_back(productInCharacter);
    }

    return true;
}

bool DbConnector::GetProductsInCharacterByCharacterId(uint32_t id, std::vector<ProductInCharacter>& productsInCharacter) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting products in characters: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ProductsInCharacter"
                                                          " WHERE CharacterID=(?)");

    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    while(res->next())
    {
        ProductInCharacter productInCharacter(res->getInt(1),
                                              res->getInt(2),
                                              res->getString(3));

        productsInCharacter.push_back(productInCharacter);
    }

    return true;
}

bool DbConnector::GetProductInCharacterById(uint32_t id, ProductInCharacter &productInCharacter) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting products in characters: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ProductsInCharacters"
                                                          " WHERE CharacterID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        productInCharacter.SetData(res->getInt(1),
                                   res->getInt(2),
                                   res->getString(3));

        return true;
    }

    return false;
}

bool DbConnector::GetSlotIsEmpty(uint32_t id, std::string slot, ProductInCharacter &productInCharacter) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting products in characters: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ProductsInCharacters"
                                                          " WHERE CharacterID=(?)"
                                                          " AND Slot=(?)");

    stmt->setInt(1, id);
    stmt->setString(2, slot);

    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        productInCharacter.SetData(res->getInt(1),
                                   res->getInt(2),
                                   res->getString(3));

        return true;
    }

    return false;
}

int DbConnector::UpdateProductInCharacter(ProductInCharacter &productInCharacter) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting products in characters: connection not initialized" << std::endl;
        return -1;
    }

    sql::PreparedStatement* checkExistStmt = conn->prepareStatement("SELECT * FROM ProductsInCharacters"
                                                                    " WHERE CharacterID=(?)");
    checkExistStmt->setInt(1, productInCharacter.CharacterID);

    sql::ResultSet* checkRes = checkExistStmt->executeQuery();

    if(!checkRes->next())
        return AddProductInCharacter(productInCharacter);

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM ProductsInCharacters"
                                                               " WHERE PIID=?"
                                                               " AND CharacterID=?");
    checkStmt->setInt(1, productInCharacter.PIID);
    checkStmt->setInt(2, productInCharacter.CharacterID);

    sql::ResultSet* res = checkStmt->executeQuery();

    if(!res->next())
    {
        sql::PreparedStatement* stmt = conn->prepareStatement("UPDATE ProductsInCharacters"
                                                              " SET"
                                                              " CharacterID=?"
                                                              " Slot=?"
                                                              " WHERE PIID=(?)");
        stmt->setInt(1, productInCharacter.CharacterID);
        stmt->setString(2, productInCharacter.GetTypeString());
        stmt->setInt(3, productInCharacter.PIID);

        stmt->executeQuery();

        return FindProductInCharacter(productInCharacter);
    } else
        return FindProductInCharacter(productInCharacter);
}

bool DbConnector::DeleteProductInCharacter(uint32_t id) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting products in characters: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("DELETE FROM ProductsInCharacters"
                                                          " WHERE PIID=?");

    stmt->setInt(1, id);
    stmt->executeQuery();

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM ProductsInCharacters"
                                                               " WHERE PIID=?");

    checkStmt->setInt(1, id);
    sql::ResultSet* res = checkStmt->executeQuery();

    return !res->next();
}

bool DbConnector::GetAllSpaceActors(std::vector<SpaceActor> &spaceActors) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::Statement* stmt = conn->createStatement();
    sql::ResultSet* res = stmt->executeQuery("SELECT * FROM SpaceActors");

    while (res->next()) {
        SpaceActor spaceActor(res->getInt(1),
                              res->getInt(2),
                              res->getString(3),
                              res->getString(4),
                              res->getInt(5),
                              res->getInt(6));
        spaceActors.push_back(spaceActor);
    }

    return true;
}

bool DbConnector::GetSpaceActorBySID(uint32_t id, std::vector<SpaceActor>& spaceActors) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM SpaceActors"
                                                          " WHERE SID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    while(res->next()) {
        SpaceActor spaceActor(res->getInt(1),
                           res->getInt(2),
                           res->getString(3),
                           res->getString(4),
                           res->getInt(5),
                           res->getInt(6));

        spaceActors.push_back(spaceActor);
    }

    return true;
}

bool DbConnector::GetSpaceActorByPIID(uint32_t id, SpaceActor& spaceActor) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM SpaceActors"
                                                          " WHERE PIID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    if(res->next()) {
        spaceActor.SetData(res->getInt(1),
                           res->getInt(2),
                           res->getString(3),
                           res->getString(4),
                           res->getInt(5),
                           res->getInt(6));

        return true;
    }

    return false;
}

// TODO Get by PIID
bool DbConnector::GetSpaceActorByActorID(uint32_t id, SpaceActor &spaceActor) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM SpaceActors"
                                                          " WHERE ActorID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    if(res->next()) {
        spaceActor.SetData(res->getInt(1),
                           res->getInt(2),
                           res->getString(3),
                           res->getString(4),
                           res->getInt(5),
                           res->getInt(6));

        return true;
    }

    return false;
}

int DbConnector::UpdateSpaceActor(SpaceActor &spaceActor) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return -1;
    }

    if(spaceActor.ActorID < 0) {
        sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM SpaceActors"
                                                                   " WHERE ActorID=?");
        checkStmt->setInt(1, spaceActor.ActorID);
        sql::ResultSet* res = checkStmt->executeQuery();

        if(!res->next())
            return AddSpaceActor(spaceActor);
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("UPDATE SpaceActors"
                                                          " SET"
                                                          " SID=?,"
                                                          " Position=?,"
                                                          " Rotation=?,"
                                                          " PIID=?,"
                                                          " WBIID=?"
                                                          " WHERE ActorID=(?)");
    stmt->setInt(1, spaceActor.SID);
    stmt->setString(2, spaceActor.Pos);
    stmt->setString(3, spaceActor.Rot);
    stmt->setInt(4, spaceActor.PIID);
    stmt->setInt(5, spaceActor.WBIID);
    stmt->setInt(6, spaceActor.ActorID);

    stmt->executeQuery();

    return FindSpaceActor(spaceActor);

}

bool DbConnector::DeleteSpaceActorByPIID(uint32_t id) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("DELETE FROM SpaceActors"
                                                          " WHERE PIID=?");
    stmt->setInt(1, id);
    stmt->executeQuery();

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM SpaceActors"
                                                               " WHERE PIID=(?)");
    checkStmt->setInt(1, id);
    sql::ResultSet* res = checkStmt->executeQuery();

    return !res->next();
}

bool DbConnector::DeleteSpaceActor(uint32_t id) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("DELETE FROM SpaceActors"
                                                          " WHERE ActorID=?");
    stmt->setInt(1, id);
    stmt->executeQuery();

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM SpaceActors"
                                                               " WHERE ActorID=(?)");
    checkStmt->setInt(1, id);
    sql::ResultSet* res = checkStmt->executeQuery();

    return !res->next();
}

bool DbConnector::GetAllContainerComponents(std::vector<ContainerComponent>& spaceComponents) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ContainerComponents");

    sql::ResultSet* res = stmt->executeQuery();

    while(res->next())
    {
        ContainerComponent component(res->getInt(1),
                                     res->getInt(2),
                                     res->getInt(3));

        spaceComponents.push_back(component);
    }

    return true;
}

bool DbConnector::GetContainerComponentByCCID(uint32_t id, ContainerComponent &spaceComponent) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ContainerComponents"
                                                          " WHERE CCID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    if(res->next()) {
        spaceComponent.SetData(res->getInt(1),
                               res->getInt(2),
                               res->getInt(3));

        return true;
    }

    return false;
}

bool DbConnector::DeleteContainerComponent(uint32_t id) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("DELETE FROM ContainerComponents"
                                                          " WHERE CCID=(?)");
    stmt->setInt(1, id);
    stmt->executeQuery();

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM ContainerComponents"
                                                               " WHERE CCID=(?)");
    checkStmt->setInt(1, id);
    sql::ResultSet* res = checkStmt->executeQuery();

    return !res->next();
}

bool DbConnector::GetAllInteractiveObjectComponents(std::vector<InteractiveObjectComponent>& interactComponents) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM InteractiveObjectComponents");

    sql::ResultSet* res = stmt->executeQuery();

    while(res->next())
    {
        InteractiveObjectComponent component(res->getInt(1),
                                             res->getInt(2),
                                             res->getString(3),
                                             res->getString(4),
                                             res->getBoolean(5),
                                             res->getString(6),
                                             res->getBoolean(7),
                                             res->getString(8),
                                             res->getBoolean(9),
                                             res->getString(10),
                                             res->getString(11),
                                             res->getString(12));

        interactComponents.push_back(component);
    }

    return true;
}

bool DbConnector::GetInteractiveObjectComponentByIOCID(uint32_t id, InteractiveObjectComponent &interactComponent) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM InteractObjectComponents"
                                                          " WHERE IOCID=(?)");
    stmt->setInt(1, id);

    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        interactComponent.SetData(res->getInt(1),
                                  res->getInt(2),
                                  res->getString(3),
                                  res->getString(4),
                                  res->getBoolean(5),
                                  res->getString(6),
                                  res->getBoolean(7),
                                  res->getString(8),
                                  res->getBoolean(9),
                                  res->getString(10),
                                  res->getString(11),
                                  res->getString(12));

        return true;
    }

    return false;
}

bool DbConnector::DeleteInteractiveObjectComponent(uint32_t id) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("DELETE FROM InteractiveObjectComponents"
                                                          " WHERE IOCID=(?)");
    stmt->setInt(1, id);
    stmt->executeQuery();

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM InteractiveObjectComponents"
                                                               " WHERE IOCID=(?)");
    checkStmt->setInt(1, id);
    sql::ResultSet* res = checkStmt->executeQuery();

    return !res->next();
}

bool DbConnector::GetAllProductArchetypeComponent(std::vector<ProductArchetypeComponent> &spaceArchetypeComponents) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ProductArchetypeComponents");

    sql::ResultSet* res = stmt->executeQuery();

    while(res->next())
    {
        ProductArchetypeComponent component(res->getInt(1),
                                            res->getInt(2));

        spaceArchetypeComponents.push_back(component);
    }

    return true;
}

bool DbConnector::GetProductArchetypeComponentsByPAID(uint32_t id,
                                                       std::vector<ProductArchetypeComponent> &spaceArchetypeComponents) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ProductArchetypeComponents"
                                                          " WHERE PAID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    while(res->next()) {
        ProductArchetypeComponent component(res->getInt(1),
                                            res->getInt(2));

        spaceArchetypeComponents.push_back(component);
    }

    return true;
}

bool DbConnector::DeleteProductArchetypeComponentsByPAID(uint32_t id) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("DELETE FROM ProductArchetypeComponents"
                                                          " WHERE PAID=(?)");
    stmt->setInt(1, id);
    stmt->executeQuery();

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM ProductArchetypeComponents"
                                                               " WHERE PAID=(?)");
    checkStmt->setInt(1, id);
    sql::ResultSet* res = checkStmt->executeQuery();

    return !res->next();
}

bool DbConnector::GetAllSpaceAreas(std::vector<Space> &spaceAreas) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::Statement* stmt = conn->createStatement();
    sql::ResultSet* res = stmt->executeQuery("SELECT * FROM Spaces");

    while(res->next())
    {
        Space spaceArea(res->getInt(1),
                        res->getString(2),
                        res->getString(3),
                        res->getString(4),
                        res->getDouble(5),
                        res->getDouble(6),
                        res->getDouble(7));

        spaceAreas.push_back(spaceArea);
    }

    return true;
}

bool DbConnector::GetSpaceAreaBySID(uint32_t id, Space &spaceArea) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM Spaces"
                                                          " WHERE SID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    while(res->next()) {
        Space spaceArea(res->getInt(1),
                        res->getString(2),
                        res->getString(3),
                        res->getString(4),
                        res->getDouble(5),
                        res->getDouble(6),
                        res->getDouble(7));

        return true;
    }

    return false;
}

int DbConnector::UpdateSpaceArea(Space &spaceArea) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return -1;
    }

    if(spaceArea.SID < 0) {
        sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM Spaces"
                                                                   " WHERE SID=?");
        checkStmt->setInt(1, spaceArea.SID);
        sql::ResultSet* res = checkStmt->executeQuery();

        if(!res->next())
            return AddSpaceArea(spaceArea);
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("UPDATE Spaces"
                                                          " SET"
                                                          " SID=?,"
                                                          " Name=?,"
                                                          " Position=?,"
                                                          " Rotation=?,"
                                                          " Height=?,"
                                                          " Width=?,"
                                                          " Length=?");
    stmt->setInt(1, spaceArea.SID);
    stmt->setString(2, spaceArea.Name);
    stmt->setString(3, spaceArea.Pos);
    stmt->setString(4, spaceArea.Rot);
    stmt->setDouble(5, spaceArea.Height);
    stmt->setDouble(6, spaceArea.Width);
    stmt->setDouble(7, spaceArea.Length);

    stmt->executeQuery();

    return FindSpaceArea(spaceArea);
}

bool DbConnector::DeleteSpaceArea(uint32_t id) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed())
    {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("DELETE FROM Spaces"
                                                          " WHERE SID=?");
    stmt->setInt(1, id);
    stmt->executeQuery();

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM Spaces"
                                                               " WHERE SID=(?)");
    checkStmt->setInt(1, id);
    sql::ResultSet* res = checkStmt->executeQuery();

    return !res->next();
}

bool DbConnector::GetPlayerInventoryByID(uint32_t id, PlayerInventory &inventory) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if(driver == nullptr || conn == nullptr || conn->isClosed())
    {
        std::cout << "Error getting inventory: connection not initialized" << std::endl;
        return false;
    }

    // Prepare query
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM PlayerInventories"
                                                          " WHERE CharacterID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        inventory.SetData(res->getInt(1),
                          res->getInt(2));

        return true;
    }

    return false;
}

int DbConnector::UpdatePlayerInventory(PlayerInventory &inventory) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error updating inventory: connection not initialized" << std::endl;
        return -1;
    }

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM PlayerInventories"
                                                               " WHERE CharacterID=?"
                                                               " AND CIID=?");
    checkStmt->setInt(1, inventory.UserID);
    checkStmt->setInt(2, inventory.CIID);

    sql::ResultSet* res = checkStmt->executeQuery();

    if(!res->next())
        return AddPlayerInventory(inventory);
    else
        return FindPlayerInventory(inventory);
}

bool DbConnector::DeletePlayerInventory(uint32_t id) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error deleting inventory: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("DELETE FROM PlayerInventories"
                                                          " WHERE CharacterID=?");
    stmt->setInt(1, id);
    stmt->executeQuery();

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM PlayerInventories"
                                                               " WHERE CharacterID=?");
    checkStmt->setInt(1, id);
    sql::ResultSet* res = checkStmt->executeQuery();

    return !res->next();
}

bool DbConnector::GetAllContainersInProducts(std::vector<ContainerInProduct>& containersInProducts) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error: connection not initialized" << std::endl;
        return false;
    }

    sql::Statement* stmt = conn->createStatement();
    sql::ResultSet* res = stmt->executeQuery("SELECT * FROM ContainersInProducts");

    while(res->next())
    {
        ContainerInProduct containerInProduct(res->getInt(1),
                                              res->getInt(2));

        containersInProducts.push_back(containerInProduct);
    }

    return true;
}

bool DbConnector::GetContainerInProductById(uint32_t id, ContainerInProduct& containerInProduct) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ContainersInProducts"
                                                          " WHERE PIID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        containerInProduct.SetData(res->getInt(1),
                                   res->getInt(2));

        return true;
    }

    return false;
}

bool DbConnector::GetContainerInProductByCIID(uint32_t id, ContainerInProduct& containerInProduct) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM ContainersInProducts"
                                                          " WHERE CIID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        containerInProduct.SetData(res->getInt(1),
                                   res->getInt(2));

        return true;
    }

    return false;
}

int DbConnector::UpdateContainerInProduct(ContainerInProduct& containerInProduct) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error deleting inventory: connection not initialized" << std::endl;
        return -1;
    }

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM ContainersInProducts"
                                                               " WHERE PIID=?");
    checkStmt->setInt(1, containerInProduct.PIID);

    sql::ResultSet* res = checkStmt->executeQuery();

    if(!res->next())
        return AddContainerInProduct(containerInProduct);
    else
        return FindContainerInProduct(containerInProduct);
}

bool DbConnector::DeleteContainerInProduct(uint32_t id) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error deleting inventory: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("DELETE FROM ContainersInProducts"
                                                          " WHERE PIID=?");
    stmt->setInt(1, id);
    stmt->executeQuery();

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM ContainersInProducts"
                                                               " WHERE PIID=?");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    return !res->next();
}

int DbConnector::UpdateClientData(User& user, uint64_t hashedPassword) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error deleting instance: connection not initialized" << std::endl;
        return -1;
    }

    if(user.UserID < 0)
    {
        sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM Users "
                                                                   " WHERE UserName=?");
        checkStmt->setString(1, user.UserName);
        sql::ResultSet* res = checkStmt->executeQuery();

        if(!res->next())
            return AddClientData(user, hashedPassword);

        return -1;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("UPDATE Users"
                                                          " SET"
                                                          " UserName=?,"
                                                          " HashedPassword=?,"
                                                          " CharacterID=?"
                                                          " WHERE UserID=(?)");
    stmt->setString(1, user.UserName);
    stmt->setInt64(2, hashedPassword);
    stmt->setInt(3, user.CharacterID);
    stmt->setInt(4, user.UserID);

    stmt->executeQuery();

    return FindClientData(user);
}

bool DbConnector::DeleteClientData(uint32_t id) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error deleting instance: connection not initialized" << std::endl;
        return -1;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("DELETE FROM Users"
                                                          " WHERE UserID=?");

    stmt->setInt(1, id);
    stmt->executeQuery();

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM Users"
                                                               " WHERE UserID=?");

    checkStmt->setInt(1, id);
    sql::ResultSet* res = checkStmt->executeQuery();

    return !res->next();
}

int DbConnector::CheckAuthData(std::string userName, uint64_t hashedPassword) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error deleting instance: connection not initialized" << std::endl;
        return -1;
    }

    sql::PreparedStatement *stmt = conn->prepareStatement("SELECT * FROM Users"
                                                          " WHERE UserName=? AND HashedPassword=?");
    stmt->setString(1, userName);
    stmt->setInt64(2, hashedPassword);

    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        return res->getInt(4);
    }

    std::cout << "Unknown user or password" << std::endl;
    return -1;
}

int DbConnector::RegistrationClient(std::string userName, uint64_t hashedPassword) {
    int userID = CheckAuthData(userName, hashedPassword);
    if(userID > -1)
    {
        std::cerr << ": [ERROR] Client is already exists!" << std::endl;
        return -1;
    }
    else {
        ProductInstance newInstance(Config::ProductBackpackConfig(userName));

        newInstance.PIID = UpdateInstance(newInstance);

        std::cout << ": [SERVER] Update product instance | New PIID: " << newInstance.PIID << "\n";

        ContainerInstance newContainerInstance(Config::ContainerBackpackConfig(newInstance.PIID));

        newContainerInstance.CIID = UpdateContainerInstance(newContainerInstance);

        std::cout << ": [SERVER] Update container instance | New CIID: " << newContainerInstance.CIID << "\n";

        ContainerInProduct newContainerInProduct(newInstance.PIID, newContainerInstance.CIID);
        int id = UpdateContainerInProduct(newContainerInProduct);

        if(id > -1)
        {
            Character newCharacter(Config::CharacterConfig(userName, newInstance.PIID));

            newCharacter.CharacterID = UpdateCharacter(newCharacter);

            User newUser(Config::UserConfig(userName, newCharacter.CharacterID));
            newUser.UserID = UpdateClientData(newUser, hashedPassword);

            std::cout << ": [SERVER] Update user | New UserID: " << newUser.UserID << "\n";

            uint64_t hashedUsername = hashedData(userName);

            CharacterBodyType BodyType(newUser.CharacterID,
                                       RandomBool(hashedUsername),
                                       RandomFloat(hashedUsername, -1.f, 1.f),
                                       RandomFloat(hashedUsername, -0.8f, 1.f),
                                       RandomFloat(hashedUsername, -1.f, 1.f),
                                       RandomFloat(hashedUsername, -1.f, 1.f),
                                       RandomFloat(hashedUsername, -0.28f, 1.f),
                                       RandomFloat(hashedUsername, -0.5f, 1.f),
                                       RandomFloat(hashedUsername, -0.2f, 1.f),
                                       RandomFloat(hashedUsername, -0.1f, 1.f),
                                       RandomFloat(hashedUsername, -0.3f, 0.8f));

            UpdateCharacterBodyType(BodyType);

            PlayerInventory inventory(newUser.CharacterID, newContainerInstance.CIID);

            UpdatePlayerInventory(inventory);

            return newUser.UserID;
        }
        else {
            std::cerr << ": [ERROR] Update container in product is not done \n";
            DeleteInstance(newInstance.PIID);
            return -1;
        }
    }
}

bool DbConnector::GetAllCharacters(std::vector<Character>& characters)
{
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM Characters");

    sql::ResultSet* res = stmt->executeQuery();

    while(res->next())
    {
        Character character(res->getInt(1),
                            res->getString(2),
                            res->getInt(3));

        characters.push_back(character);
    }

    return true;
}

bool DbConnector::GetCharacterByID(uint32_t id, Character &character) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error deleting instance: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT * FROM Characters"
                                                          " WHERE CharacterID=(?)");
    stmt->setInt(1, id);
    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        character.SetData(res->getInt(1),
                          res->getString(2),
                          res->getInt(3));

        return true;
    }

    return false;
}

int DbConnector::UpdateCharacter(Character &character) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error deleting instance: connection not initialized" << std::endl;
        return -1;
    }

    if(character.CharacterID < 0)
    {
        sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM Characters"
                                                                   " WHERE CharacterID=?");
        checkStmt->setInt(1, character.CharacterID);
        sql::ResultSet* res = checkStmt->executeQuery();

        if(!res->next())
            return AddCharacter(character);
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("UPDATE Characters"
                                                          " SET"
                                                          " Name=?,"
                                                          " InventoryID=?"
                                                          " WHERE CharacterID=(?)");
    stmt->setString(1, character.Name);
    stmt->setInt(2, character.InventoryID);
    stmt->setInt(3, character.CharacterID);

    stmt->executeQuery();

    return FindCharacter(character);
}

bool DbConnector::DeleteCharacter(uint32_t id) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if(!isConnect())
        conn->reconnect();

    // Return if something is not initialized
    if (driver == nullptr || conn == nullptr || conn->isClosed()) {
        std::cout << "Error deleting instance: connection not initialized" << std::endl;
        return false;
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("DELETE FROM Characters"
                                                          " WHERE CharacterID=?");
    stmt->setInt(1, id);
    stmt->executeQuery();

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM Characters"
                                                               " WHERE CharacterID=(?)");
    checkStmt->setInt(1, id);
    sql::ResultSet* res = checkStmt->executeQuery();

    return !res->next();
}



int DbConnector::AddCharacterBodyType(CharacterBodyType& BodyType) {
    int id = FindCharacterBodyType(BodyType);
    if(id > 0)
        return id;

    sql::PreparedStatement* stmt = conn->prepareStatement("INSERT INTO CharacterBodyTypes"
                                                          " (CharacterID, Male, Height, Shoulders, ArmLength, LegLength, Muscle, Belly, Fat, Breast, Hips)"
                                                          " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

    stmt->setInt(1, BodyType.CharacterID);
    stmt->setBoolean(2, BodyType.bMale);
    stmt->setDouble(3, BodyType.Height);
    stmt->setDouble(4, BodyType.Shoulders);
    stmt->setDouble(5, BodyType.ArmLength);
    stmt->setDouble(6, BodyType.LegLength);
    stmt->setDouble(7, BodyType.Muscle);
    stmt->setDouble(8, BodyType.Belly);
    stmt->setDouble(9, BodyType.Fat);
    stmt->setDouble(10, BodyType.Breast);
    stmt->setDouble(11, BodyType.Hips);

    stmt->executeQuery();

    std::cout << "Add new CharacterBodyType\n";

    return FindCharacterBodyType(BodyType);
}

int DbConnector::AddArchetype(ProductArchetype &archetype) {
    // Check, if this archetype already exists
    // We don't need copies in database
    int id = FindArchetypeByFields(archetype);
    if (id > 0)
        return id;

    sql::PreparedStatement* stmt = conn->prepareStatement("INSERT INTO ProductArchetypes"
                                                          " (Name, Description, ActorClass, Density, Type)"
                                                          " VALUES (?, ?, ?, ?, ?)");

    stmt->setString(1, archetype.name);
    stmt->setString(2, archetype.description);
    stmt->setString(3, archetype.actorClass);
    stmt->setInt(4, archetype.density);
    stmt->setString(5, archetype.GetTypeString());

    stmt->executeQuery();

    std::cout << "Add new Product Archetype - " << archetype.name << std::endl;

    // Return new archetype id
    return FindArchetypeByFields(archetype);
}

int DbConnector::AddGraphicsSet(GraphicsSet& graphicsSet) {
    // Check, if this archetype already exists
    // We don't need copies in database
    int id = FindGraphicsSetByFields(graphicsSet);
    if (id > 0)
        return id;

    sql::PreparedStatement* stmt = conn->prepareStatement("INSERT INTO GraphicsSets"
                                                          " (GraphicSetID)"
                                                          " VALUES (?)");

    stmt->setInt(1, graphicsSet.GraphicSetID);

    stmt->executeQuery();

    // Return new archetype id
    return FindGraphicsSetByFields(graphicsSet);
}

int DbConnector::AddInstance(ProductInstance& instance) {
    // Check, if this archetype already exists
    // We don't need copies in database
    int id = FindInstanceByFields(instance);
    if (id > 0)
        return id;

    sql::PreparedStatement* stmt = conn->prepareStatement("INSERT INTO ProductInstances"
                                                          " (PAID, Mass, Volume, Quality, CurrentGraphicSetID, MetaData, PresenceType)"
                                                          " VALUES (?, ?, ?, ?, ?, ?, ?)");

    stmt->setInt(1, instance.PAID);
    stmt->setDouble(2, instance.mass);
    stmt->setDouble(3, instance.volume);
    stmt->setDouble(4, instance.quality);
    stmt->setInt(5, instance.CurrentGSID);
    stmt->setString(6, instance.meta);
    stmt->setString(7, instance.GetTypeString());


    stmt->executeQuery();

    // Return new archetype id
    return FindInstanceByFields(instance);
}

int DbConnector::AddContainerArchetype(ContainerArchetype &containerArchetype) {
    // Check, if this container archetype already exists
    // We don't need copies in database
    int id = FindContainerArchetypeByFields(containerArchetype);
    if (id > 0)
        return id;

    sql::PreparedStatement* stmt = conn->prepareStatement("INSERT INTO ContainerArchetypes"
                                                          " (Name, MaxMass, MaxVolume, AvailableTypes, AvailableArchetypes"
                                                          " VALUES(?, ?, ?, ?, ?)");

    stmt->setString(1, containerArchetype.name);
    stmt->setInt(2, containerArchetype.maxMass);
    stmt->setInt(3, containerArchetype.maxVolume);
    stmt->setInt(4, containerArchetype.availableTypes);
    stmt->setString(5, containerArchetype.availableArchetypes);

    stmt->executeQuery();

    std::cout << "Add new Container Archetype - " << containerArchetype.name << std::endl;

    // Return new container archetype id
    return FindContainerArchetypeByFields(containerArchetype);
}

int DbConnector::AddContainerInstance(ContainerInstance &containerInstance) {
    int id = FindContainerInstanceByFields(containerInstance);
    if(id > 0)
        return id;

    sql::PreparedStatement* stmt = conn->prepareStatement("INSERT INTO ContainerInstances"
                                                          " (CAID, PIID)"
                                                          " VALUES (?, ?)");

    stmt->setInt(1, containerInstance.CAID);
    stmt->setInt(2, containerInstance.PIID);

    stmt->executeQuery();

    return FindContainerInstanceByFields(containerInstance);
}

int DbConnector::AddWorkbenchInstance(WorkbenchInstance& workbenchInstance) {
    int id = FindWorkbenchInstanceByFields(workbenchInstance);
    if(id > 0)
        return id;

    sql::PreparedStatement* stmt = conn->prepareStatement("INSERT INTO WorkbenchInstances"
                                                          " (WBAID, PIID)"
                                                          " VALUES (?, ?)");

    stmt->setInt(1, workbenchInstance.WBAID);
    stmt->setInt(2, workbenchInstance.PIID);

    stmt->executeQuery();

    return FindWorkbenchInstanceByFields(workbenchInstance);
}

int DbConnector::AddGraphicsSetsActor(GraphicsSetsActor &graphicsSetActor) {
    int id = FindGraphicsSetsActorByFields(graphicsSetActor);
    if(id > 0)
        return id;

    sql::PreparedStatement* stmt = conn->prepareStatement("INSERT INTO GraphicsSetsActors"
                                                          " (GraphicSetID, ActorID)"
                                                          " VALUES (?, ?)");

    stmt->setInt(1, graphicsSetActor.GraphicSetID);
    stmt->setInt(2, graphicsSetActor.ActorID);

    stmt->executeQuery();

    return FindGraphicsSetsActorByFields(graphicsSetActor);
}

int DbConnector::AddGraphicsSetsProduct(GraphicsSetsProduct &graphicsSetProduct) {
    int id = FindGraphicsSetsProductByFields(graphicsSetProduct);
    if(id > 0)
        return id;

    sql::PreparedStatement* stmt = conn->prepareStatement("INSERT INTO GraphicsSetsProducts"
                                                          " (PAID, GraphicSetClientID)"
                                                          " VALUES (?, ?)");

    stmt->setInt(1, graphicsSetProduct.PAID);
    stmt->setInt(2, graphicsSetProduct.GraphicSetClientID);

    stmt->executeQuery();

    return FindGraphicsSetsProductByFields(graphicsSetProduct);
}

int DbConnector::AddContainersInActor(ContainersInActor &containersInActor) {
    int id = FindContainersInActorByFields(containersInActor);
    if(id > 0)
        return id;

    sql::PreparedStatement* stmt = conn->prepareStatement("INSERT INTO ContainersInActors"
                                                          " (CIID, ActorID)"
                                                          " VALUES (?, ?)");

    stmt->setInt(1, containersInActor.CIID);
    stmt->setInt(2, containersInActor.ActorID);

    stmt->executeQuery();

    return FindContainersInActorByFields(containersInActor);
}

int DbConnector::AddProductsInActor(ProductsInActor &productsInActor) {
    int id = FindProductsInActor(productsInActor);
    if(id > 0)
        return id;

    sql::PreparedStatement* stmt = conn->prepareStatement("INSERT INTO ProductsInActors"
                                                          " (PIID, ActorID)"
                                                          " VALUES (?, ?)");

    stmt->setInt(1, productsInActor.PIID);
    stmt->setInt(2, productsInActor.ActorID);

    stmt->executeQuery();

    return FindProductsInActor(productsInActor);
}

int DbConnector::AddProductsInContainer(ProductsInContainer &productsInContainer) {
//    int id = FindProductsInContainer(productsInContainer);
//    if(id > 0)
//        return id;

    sql::PreparedStatement* stmt = conn->prepareStatement("INSERT INTO ProductsInContainers"
                                                          " (PIID, CIID)"
                                                          " VALUES (?, ?)");

    stmt->setInt(1, productsInContainer.PIID);
    stmt->setInt(2, productsInContainer.CIID);

    stmt->executeQuery();

    return FindProductsInContainer(productsInContainer);
}

int DbConnector::AddContainerInProduct(ContainerInProduct& containerInProduct) {
//    int id = FindContainerInProduct(containerInProduct);
//    if(id > 0)
//        return id;

    sql::PreparedStatement* stmt = conn->prepareStatement("INSERT INTO ContainersInProducts"
                                                          " (PIID, CIID)"
                                                          " VALUES (?, ?)");
    stmt->setInt(1, containerInProduct.PIID);
    stmt->setInt(2, containerInProduct.CIID);

    stmt->executeQuery();

    return FindContainerInProduct(containerInProduct);
}

int DbConnector::AddSpaceActor(SpaceActor &spaceActor) {
    int id = FindSpaceActor(spaceActor);
    if(id > 0)
        return id;

    sql::PreparedStatement* checkStmt = conn->prepareStatement("SELECT * FROM SpaceActors"
                                                               " WHERE PIID=?");
    checkStmt->setInt(1, spaceActor.PIID);
    sql::ResultSet* res = checkStmt->executeQuery();

    if(res->next())
    {
        SpaceActor UspaceActor (
                res->getInt(1),
                spaceActor.SID,
                spaceActor.Pos,
                spaceActor.Rot,
                res->getInt(5),
                spaceActor.WBIID);

        return UpdateSpaceActor(UspaceActor);
    }

    sql::PreparedStatement* stmt = conn->prepareStatement("INSERT INTO SpaceActors"
                                                          " (SID, Position, Rotation, PIID, WBIID)"
                                                          " VALUES (?, ?, ?, ?, ?)");
    stmt->setInt(1, spaceActor.SID);
    stmt->setString(2, spaceActor.Pos);
    stmt->setString(3, spaceActor.Rot);
    stmt->setInt(4, spaceActor.PIID);
    stmt->setInt(5, spaceActor.WBIID);

    stmt->executeQuery();

    return FindSpaceActor(spaceActor);
}

int DbConnector::AddSpaceArea(Space &spaceArea) {
    int id = FindSpaceArea(spaceArea);
    if(id > 0)
        return id;

    sql::PreparedStatement* stmt = conn->prepareStatement("INSERT INTO Spaces"
                                                          " (Name, Position, Rotation, Height, Width, Length)"
                                                          " VALUES (?, ?, ?, ?, ?, ?)");
    stmt->setString(1, spaceArea.Name);
    stmt->setString(2, spaceArea.Pos);
    stmt->setString(3, spaceArea.Rot);
    stmt->setDouble(4, spaceArea.Height);
    stmt->setDouble(5, spaceArea.Width);
    stmt->setDouble(6, spaceArea.Length);

    stmt->executeQuery();

    return FindSpaceArea(spaceArea);
}

int DbConnector::AddPlayerInventory(PlayerInventory &inventory) {
    int id = FindPlayerInventory(inventory);
    if(id > 0)
        return id;

    sql::PreparedStatement* stmt = conn->prepareStatement("INSERT INTO PlayerInventories"
                                                          " (CharacterID, CIID)"
                                                          " VALUES (?, ?)");
    stmt->setInt(1, inventory.UserID);
    stmt->setInt(2, inventory.CIID);

    stmt->executeQuery();

    return FindPlayerInventory(inventory);
}

int DbConnector::AddClientData(User& user, uint64_t hashedPassword) {
    int id = FindClientData(user);
    if(id > 0)
        return id;

    sql::PreparedStatement* stmt = conn->prepareStatement("INSERT INTO Users"
                                                          " (UserName, HashedPassword, CharacterID)"
                                                          " VALUES (?, ?, ?)");
    stmt->setString(1, user.UserName);
    stmt->setInt64(2, hashedPassword);
    stmt->setInt(3, user.CharacterID);

    stmt->executeQuery();

    return FindClientData(user);
}

int DbConnector::AddCharacter(Character &character) {
    int id = FindCharacter(character);
    if(id > 0)
        return id;

    sql::PreparedStatement* stmt = conn->prepareStatement("INSERT INTO Characters"
                                                          " (Name, InventoryID)"
                                                          " VALUES (?, ?)");
    stmt->setString(1, character.Name);
    stmt->setInt(2, character.InventoryID);

    stmt->executeQuery();

    return FindCharacter(character);
}

int DbConnector::AddProductInCharacter(ProductInCharacter& productInCharacter) {
    int id = FindProductInCharacter(productInCharacter);
    if(id > 0)
        return id;

    sql::PreparedStatement* stmt = conn->prepareStatement("INSERT INTO ProductsInCharacters"
                                                          " (PIID, CharacterID, Slot)"
                                                          " VALUES (?, ?, ?)");
    stmt->setInt(1, productInCharacter.PIID);
    stmt->setInt(2, productInCharacter.CharacterID);
    stmt->setString(3, productInCharacter.GetTypeString());

    stmt->executeQuery();

    return FindProductInCharacter(productInCharacter);
}

int DbConnector::FindCharacterBodyType(CharacterBodyType& BodyType) {
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT CharacterID FROM CharacterBodyTypes"
                                                          " WHERE CharacterID=?");

    stmt->setInt(1, BodyType.CharacterID);

    sql::ResultSet* res = stmt->executeQuery();
    if(res->next())
    {
        int id = res->getInt(1);

        delete res;
        delete stmt;

        return id;
    }

    return -1;
}

int DbConnector::FindArchetypeByFields(ProductArchetype& archetype) {
    // Try to find this archetype
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT PAID FROM ProductArchetypes"
                                                          " WHERE Name=(?)"
                                                          " AND Description=(?)"
                                                          " AND ActorClass=(?)"
                                                          " AND Density=(?)"
                                                          " AND Type=(?)");

    stmt->setString(1, archetype.name);
    stmt->setString(2, archetype.description);
    stmt->setString(3, archetype.actorClass);
    stmt->setInt(4, archetype.density);
    stmt->setString(5, archetype.GetTypeString());

    sql::ResultSet* res = stmt->executeQuery();

    // If it exists, return its id
    if (res->next()) {
        int id = res->getInt(1);

        delete res;
        delete stmt;

        return id;
    }

    return -1;
}

int DbConnector::FindGraphicsSetByFields(GraphicsSet& graphicsSet) {
    // Try to find this archetype
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT GraphicSetID FROM GraphicsSets"
                                                          " WHERE GraphicSetID=?");

    stmt->setInt(1, graphicsSet.GraphicSetID);

    sql::ResultSet* res = stmt->executeQuery();

    // If it exists, return its id
    if (res->next()) {
        int id = res->getInt(1);

        delete res;
        delete stmt;

        return id;
    }

    return -1;
}

int DbConnector::FindInstanceByFields(ProductInstance& instance) {
    // Try to find this archetype
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT PIID FROM ProductInstances"
                                                          " WHERE PAID=?"
                                                          " AND Mass=?"
                                                          " AND Volume=?"
                                                          " AND Quality=?"
                                                          " AND CurrentGraphicSetID=?"
                                                          " AND MetaData=?"
                                                          " AND PresenceType=?");

    stmt->setInt(1, instance.PAID);
    stmt->setDouble(2, instance.mass);
    stmt->setDouble(3, instance.volume);
    stmt->setDouble(4, instance.quality);
    stmt->setInt(5, instance.CurrentGSID);
    stmt->setString(6, instance.meta);
    stmt->setString(7, instance.GetTypeString());


    sql::ResultSet* res = stmt->executeQuery();

    // If it exists, return its id
    if (res->next()) {
        int id = res->getInt(1);

        delete res;
        delete stmt;

        return id;
    }

    return -1;
}

int DbConnector::FindContainerArchetypeByFields(ContainerArchetype &containerArchetype) {
    // Try to find this archetype
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT CAID FROM ContainerArchetypes"
                                                          " WHERE Name=(?)"
                                                          " AND MaxMass=(?)"
                                                          " AND MaxVolume=(?)"
                                                          " AND AvailableTypes=(?)"
                                                          " AND AvailableArchetypes=(?)");

    stmt->setString(1, containerArchetype.name);
    stmt->setInt(2, containerArchetype.maxMass);
    stmt->setInt(3, containerArchetype.maxVolume);
    stmt->setInt(4, containerArchetype.availableTypes);
    stmt->setString(5, containerArchetype.availableArchetypes);

    sql::ResultSet* res = stmt->executeQuery();

    // If it exists, return its id
    if (res->next())
    {
        int id = res->getInt(1);

        delete res;
        delete stmt;

        return id;
    }

    return -1;
}

int DbConnector::FindContainerInstanceByFields(ContainerInstance &containerInstance) {
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT CIID FROM ContainerInstances"
                                                          " WHERE CAID=?"
                                                          " AND PIID=?");

    stmt->setInt(1, containerInstance.CAID);
    stmt->setInt(2, containerInstance.PIID);

    sql::ResultSet* res = stmt->executeQuery();

    if(res->next()) {
        int id = res->getInt(1);

        delete res;
        delete stmt;

        return id;
    }

    return -1;
}

int DbConnector::FindWorkbenchInstanceByFields(WorkbenchInstance& workbenchInstance) {
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT WBID FROM WorkbenchInstances"
                                                          " WHERE WBAID=?"
                                                          " AND PIID=?");

    stmt->setInt(1, workbenchInstance.WBAID);
    stmt->setInt(2, workbenchInstance.PIID);

    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        int id = res->getInt(1);

        delete res;
        delete stmt;

        return id;
    }

    return -1;
}

int DbConnector::FindGraphicsSetsActorByFields(GraphicsSetsActor &graphicsSetsActor) {
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT GraphicSetID FROM GraphicsSetsActors"
                                                          " WHERE ActorID=?");

    stmt->setInt(1, graphicsSetsActor.ActorID);

    sql::ResultSet* res = stmt->executeQuery();

    if(res->next()) {
        int id = res->getInt(1);

        delete res;
        delete stmt;

        return id;
    }

    return -1;
}

int DbConnector::FindGraphicsSetsProductByFields(GraphicsSetsProduct &graphicsSetsProduct) {
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT GraphicSetID FROM GraphicsSetsProducts"
                                                          " WHERE PAID=?"
                                                          " AND GraphicSetClientID=?");

    stmt->setInt(1, graphicsSetsProduct.PAID);
    stmt->setInt(2, graphicsSetsProduct.GraphicSetClientID);

    sql::ResultSet* res = stmt->executeQuery();

    if(res->next()) {
        int id = res->getInt(1);

        delete res;
        delete stmt;

        return id;
    }

    return -1;
}

int DbConnector::FindContainersInActorByFields(ContainersInActor &containersInActor) {
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT CIID FROM ContainersInActors"
                                                          " WHERE ActorID=?");

    stmt->setInt(1, containersInActor.ActorID);

    sql::ResultSet* res = stmt->executeQuery();

    if(res->next()) {
        int id = res->getInt(1);

        delete res;
        delete stmt;

        return id;
    }

    return -1;
}

int DbConnector::FindProductsInActor(ProductsInActor &productsInActor) {
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT PIID FROM ProductsInActors"
                                                          " WHERE ActorID=?");

    stmt->setInt(1, productsInActor.ActorID);

    sql::ResultSet* res = stmt->executeQuery();

    if(res->next()) {
        int id = res->getInt(1);

        delete res;
        delete stmt;

        return id;
    }

    return -1;
}

int DbConnector::FindProductsInContainer(ProductsInContainer &productsInContainer) {
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT PIID FROM ProductsInContainers"
                                                          " WHERE CIID=?"
                                                          " AND PIID=?");

    stmt->setInt(1, productsInContainer.CIID);
    stmt->setInt(2, productsInContainer.PIID);

    sql::ResultSet* res = stmt->executeQuery();

    if(res->next()) {
        int id = res->getInt(1);

        delete res;
        delete stmt;

        return id;
    }

    return -1;
}

int DbConnector::FindContainerInProduct(ContainerInProduct &containerInProduct) {
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT PIID FROM ContainersInProducts"
                                                          " WHERE CIID=?"
                                                          " AND PIID=?");

    stmt->setInt(1, containerInProduct.CIID);
    stmt->setInt(2, containerInProduct.PIID);

    sql::ResultSet* res = stmt->executeQuery();

    if(res->next()) {
        int id = res->getInt(1);

        delete res;
        delete stmt;

        return id;
    }

    return -1;
}

int DbConnector::FindSpaceActor(SpaceActor &spaceActor) {
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT ActorID FROM SpaceActors"
                                                          " WHERE PIID=?");
    stmt->setInt(1, spaceActor.PIID);

    sql::ResultSet* res = stmt->executeQuery();

    if(res->next()) {
        int id = res->getInt(1);

        delete res;
        delete stmt;

        return id;
    }

    return -1;
}

int DbConnector::FindSpaceArea(Space &spaceArea) {
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT SID FROM Spaces"
                                                          " WHERE SID=?");
    stmt->setInt(1, spaceArea.SID);

    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        int id = res->getInt(1);

        delete res;
        delete stmt;

        return id;
    }

    return -1;
}

int DbConnector::FindPlayerInventory(PlayerInventory &inventory) {
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT CharacterID FROM PlayerInventories"
                                                          " WHERE CIID=?");
    stmt->setInt(1, inventory.CIID);

    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        int id = res->getInt(1);

        delete res;
        delete stmt;

        return id;
    }

    return -1;
}

int DbConnector::FindClientData(User& user) {
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT UserID FROM Users"
                                                          " WHERE UserName=? AND CharacterID=?");
    stmt->setString(1, user.UserName);
    stmt->setInt(2, user.CharacterID);

    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        int id = res->getInt(1);

        delete res;
        delete stmt;

        return id;
    }

    return -1;
}

int DbConnector::FindCharacter(Character &character) {
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT CharacterID FROM Characters"
                                                          " WHERE Name=? AND InventoryID=?");
    stmt->setString(1, character.Name);
    stmt->setInt(2, character.InventoryID);

    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        int id = res->getInt(1);

        delete res;
        delete stmt;

        return id;
    }

    return -1;
}

int DbConnector::FindProductInCharacter(ProductInCharacter &productInCharacter) {
    sql::PreparedStatement* stmt = conn->prepareStatement("SELECT PIID FROM ProductsInCharacters"
                                                          " WHERE CharacterID=?");

    stmt->setInt(1, productInCharacter.CharacterID);

    sql::ResultSet* res = stmt->executeQuery();

    if(res->next())
    {
        int id = res->getInt(1);

        delete res;
        delete stmt;

        return id;
    }

    return -1;
}

bool DbConnector::Split(ProductInstance& instance, float deltaMass, float density) {
    if(deltaMass == instance.mass)
        return true;

    ProductInstance newInstance(-1,
                                instance.PAID,
                                deltaMass,
                                deltaMass / density,
                                instance.quality,
                                instance.CurrentGSID,
                                instance.meta,
                                instance.type);

    instance.mass -= deltaMass;
    instance.volume -= newInstance.volume;

    UpdateInstance(instance);

    int newPIID = UpdateInstance(newInstance);
    GetInstanceById(newPIID, instance);

    return true;
}

bool DbConnector::TransferProduct(int PIID, int CIID, float deltaMass) {
    ProductInstance productInstance;
    if(!GetInstanceById(PIID, productInstance))
        return false;

    ProductArchetype productArchetype;
    if(!GetArchetypeById(productInstance.PAID, productArchetype))
        return false;

    ContainerInstance containerInstance;
    if(!GetContainerInstanceById(CIID, containerInstance))
        return false;

    if(!ProductCanBeAddedToContainer(containerInstance, productInstance))
        return false;

    bool isAdded = false;

    if(!(productArchetype.type == ProductArchetypeType::Solid))
    {
        std::cout << "Transfer non solid product - <" << productArchetype.name << ">...";

        std::vector<ProductsInContainer> products;
        GetProductsInContainerById(containerInstance.CIID, products);

        if(deltaMass > 0.f)
            Split(productInstance, deltaMass, productArchetype.density);

        for(auto product : products)
        {
            ProductInstance instance;
            GetInstanceById(product.PIID, instance);

            if(productInstance.PAID == instance.PAID)
            {
                float totalQuality = (instance.quality * instance.mass + productInstance.quality * productInstance.mass) / (instance.mass + productInstance.mass);

                instance.quality = totalQuality;
                instance.mass += productInstance.mass;

                UpdateInstance(instance);

                isAdded = true;

                std::cout << "Merge non solid product with existing product in container <" << productInstance.PIID << "> + <" << product.PIID << ">.";
            }
        }
    }

    if(!isAdded)
    {
        ProductsInContainer newProduct(productInstance.PIID, CIID);
        UpdateProductsInContainer(newProduct);
    } else
    {
        DeleteInstance(productInstance.PIID);
    }

    ContainerInProduct parentProduct;
    if(GetContainerInProductByCIID(CIID, parentProduct))
    {
        ProductInstance instance;
        if(GetInstanceById(parentProduct.PIID, instance))
        {
            instance.mass += productInstance.mass;

            UpdateInstance(instance);
        }
    }

    return true;
}

bool DbConnector::TransferProductToHand(int PIID, int CharacterID) {
    ProductInstance productInstance;
    if(!GetInstanceById(PIID, productInstance))
        return false;

    ProductArchetype productArchetype;
    if(!GetArchetypeById(productInstance.PAID, productArchetype))
        return false;

    Character character;
    if(!GetCharacterByID(CharacterID, character))
        return false;

    if(!(productArchetype.type == ProductArchetypeType::Solid))
        return false;

    ProductInCharacter productInCharacter;
    if(GetSlotIsEmpty(character.CharacterID, "Hand", productInCharacter))
        return false;

    productInCharacter.SetData(productInstance.PIID,
                               character.CharacterID,
                               "Hand");

    SpaceActor actor(-1, 0, "0, 0, 0", "0, 0, 0", productInstance.PIID, -1);

    if(UpdateProductInCharacter(productInCharacter) != -1)
        if(UpdateSpaceActor(actor) != -1)
            return true;

    return false;
}

bool DbConnector::TransferProductToSpaceActor(SpaceActor& spaceActor) {
    ProductInstance instance;
    if(!GetInstanceById(spaceActor.PIID, instance))
        return false;

    ProductArchetype archetype;
    if(!GetArchetypeById(instance.PAID, archetype))
        return false;

    Space space;
    if(!GetSpaceAreaBySID(spaceActor.SID, space))
        return false;

    if(!(archetype.type == ProductArchetypeType::Solid))
        return false;

    SpaceActor newActor;
    if(GetSpaceActorByPIID(spaceActor.PIID, newActor))
    {
        spaceActor.ActorID = newActor.ActorID;
    } else
        spaceActor.ActorID = -1;

    spaceActor.ActorID = UpdateSpaceActor(spaceActor);

    if(spaceActor.ActorID > -1)
        return true;

    return false;
}


bool DbConnector::TransferProductContainerToHand(int PIID, int CharacterID) {
    if(DeleteProductsInContainer(PIID))
        if(TransferProductToHand(PIID, CharacterID))
            return true;

    return false;
}

bool DbConnector::TransferProductHandToContainer(int PIID, int CIID) {
    if(TransferProduct(PIID, CIID, 0.f))
    {
        DeleteSpaceActorByPIID(PIID);
        DeleteProductInCharacter(PIID);
    }

    return true;
}

bool DbConnector::TransferProductContainerToSpaceActor(SpaceActor& spaceActor) {
    if(GetSpaceActorByActorID(spaceActor.ActorID, spaceActor))
        return false;

    if(TransferProductToSpaceActor(spaceActor))
    {
        if(DeleteProductsInContainer(spaceActor.PIID))
            return true;
    }

    return false;
}

bool DbConnector::TransferProductSpaceActorToHand(int ActorID, int CharacterID) {
    SpaceActor spaceActor;
    if(!GetSpaceActorByActorID(ActorID, spaceActor))
        return false;

    Character character;
    if(!GetCharacterByID(CharacterID, character))
        return false;

    if(TransferProductToHand(spaceActor.PIID, CharacterID))
        if(DeleteSpaceActor(ActorID))
            return true;

    return false;
}

bool DbConnector::TransferProductHandToSpaceActor(SpaceActor& spaceActor) {
    if(TransferProductToSpaceActor(spaceActor))
        if(DeleteProductInCharacter(spaceActor.PIID))
            return true;

    return false;
}

float DbConnector::GetContainerMassHeld(int CIID) {
    float MassHeld = 0.f;

    std::vector<ProductsInContainer> products;
    GetProductsInContainerById(CIID, products);

    for(auto productID : products)
    {
        MassHeld += GetProductTotalMass(productID.PIID);
    }

    return MassHeld;
}

float DbConnector::GetContainerVolumeHeld(int CIID) {
    float VolumeHeld = 0.f;

    std::vector<ProductsInContainer> products;
    GetProductsInContainerById(CIID, products);

    for(auto productID : products)
    {
        VolumeHeld += GetProductTotalVolume(productID.PIID);
    }

    return VolumeHeld;
}

float DbConnector::GetProductTotalMass(int PIID) {
    float TotalMass = 0.f;

    ProductInstance instance;
    GetInstanceById(PIID, instance);

//    ContainerInProduct container;
//    if(GetContainerInProductById(PIID, container))
//        TotalMass += GetContainerMassHeld(container.CIID);

    TotalMass += instance.mass;

    return TotalMass;
}

float DbConnector::GetProductTotalVolume(int PIID) {
    float TotalVolume = 0.f;

    ProductInstance instance;
    GetInstanceById(PIID, instance);

    ContainerInProduct container;
    if(GetContainerInProductById(PIID, container))
    {
        // TODO ??? + Volume from Container in Product ???
    }

    TotalVolume += instance.volume;

    return TotalVolume;
}

bool DbConnector::ProductCanBeAddedToContainer(ContainerInstance containerInstance, ProductInstance productInstance) {
    ContainerArchetype containerArchetype;
    GetContainerArchetypeById(containerInstance.CAID, containerArchetype);

    ProductArchetype productArchetype;
    GetArchetypeById(productInstance.PAID, productArchetype);

    if(!HasFlag(containerArchetype.availableTypes, productArchetype.type))
    {
        std::cout << "~~~ Container <" << containerInstance.CIID << ">: Product <" << productInstance.PIID <<"> has wrong flags";
        return false;
    }

    if(containerArchetype.maxMass >= 0 && productInstance.mass > containerArchetype.maxMass - GetContainerMassHeld(containerInstance.CIID))
    {
        std::cout << "~~~ Container <" << containerInstance.CIID << ">: Product <" << productInstance.PIID <<"> has too big mass";
        return false;
    }

    if(containerArchetype.maxVolume >= 0 && productInstance.volume > containerArchetype.maxVolume - GetContainerVolumeHeld(containerInstance.CIID))
    {
        std::cout << "~~~ Container <" << containerInstance.CIID << ">: Product <" << productInstance.PIID <<"> has too big volume";
        return false;
    }

    return true;
}

void DbConnector::GenerateSpaceAreas(int Xpos, int Ypos, float Height, float Width, float Length) {
    for (int i = 0; i < 8; ++i) {
        int X = Xpos - 9600 * i;
        for (int j = 0; j < 12; ++j) {
            int Y = Ypos - 9600 * j;
            std::string Xstr = std::to_string(X);
            std::string Ystr = std::to_string(Y);

            std::string position = Xstr + ", " + Ystr + ", 0";

            std::string index_i = std::to_string(i);
            std::string index_j = std::to_string(j);
            std::string Name = "CityArea_" + index_i + "_" + index_j;

            std::string rotation = "0, 0, 0";

            Space newSpace(-1,
                           Name,
                           position,
                           rotation,
                           Height,
                           Width,
                           Length);

            UpdateSpaceArea(newSpace);
        }
    }
}

bool DbConnector::CheckTransferPossible(ProductInstance product, int CIID, float deltaMass, float& partialMass) {
    ProductArchetype productArch;
    if (!GetArchetypeById(product.PAID, productArch))
        return false;

    ContainerInstance container;
    if (!GetContainerInstanceById(CIID, container))
        return false;
    ContainerArchetype containerArch;
    if (!GetContainerArchetypeById(container.CAID, containerArch))
        return false;
    std::vector<ProductsInContainer> productsInside;
    if (!GetProductsInContainerById(CIID, productsInside))
        return false;

    // False if product type is wrong
    if (!HasFlag(containerArch.availableTypes, productArch.type))
        return false;

    float availableMass = containerArch.maxMass;
    float availableVolume = containerArch.maxVolume;

    for (auto& item : productsInside){
        ProductInstance inside;
        if (!GetInstanceById(item.PIID, inside))
            return false;

        availableMass -= inside.mass;
        availableVolume -= inside.volume;
    }

    if (productArch.type == ProductArchetypeType::Solid) {
        availableMass -= product.mass;
        availableVolume -= product.volume;

        // False if container doesn't have enough space
        if (availableMass <= 0.f || availableVolume <= 0.f)
            return false;

        partialMass = product.mass;
        return true;
    } else {
        partialMass = (deltaMass > 0.f && deltaMass <= product.mass)
                    ? deltaMass : product.mass;

        float deltaVolume;
        if (availableMass - partialMass >= 0.f) {
            deltaVolume = partialMass / productArch.density;
            if (availableVolume - deltaVolume >= 0.f) {
                return true;
            } else {
                deltaVolume = availableVolume;
                partialMass = deltaVolume * productArch.density;
                return true;
            }
        } else {
            partialMass = availableMass;
            deltaVolume = partialMass / productArch.density;
            if (availableVolume - deltaVolume >= 0.f) {
                return true;
            } else {
                deltaVolume = availableVolume;
                partialMass = deltaVolume * productArch.density;
                return true;
            }
        }
    }
}

bool DbConnector::CheckBreakOnRemoval(int PIID, bool bFullRemove, int WBID, int CIID, int& WBSID, bool& bIsCompleted) {
    std::map<int, std::list<WorkbenchSlotInfo>>::iterator status_it;
    status_it = WorkbenchSlotInfoMap.find(WBID);

    if (status_it == WorkbenchSlotInfoMap.end()){
        return false;
    }

    std::list<WorkbenchSlotInfo>::iterator slot_it;
    std::list<int>::iterator piids_it;
    for (slot_it = status_it->second.begin(); slot_it != status_it->second.end(); slot_it++) {
        if (slot_it->HasPIID(PIID, piids_it)) {
            WBSID = slot_it->WBSID;
            switch (slot_it->status) {
                case Ready:
                    std::cout << "ERROR: Product " << PIID << " marked in workbench " << WBID
                              << ":" << WBSID << " while slot is in Ready state!";
                    return false;
                case Process:
                    bIsCompleted = false;
                    status_it->second.erase(slot_it);
                    if (status_it->second.empty())
                        WorkbenchSlotInfoMap.erase(status_it);
                    return true;
                case Finish:
                    if (bFullRemove) {
                        slot_it->PIIDs.erase(piids_it);
                        if (slot_it->PIIDs.empty()) {
                            bIsCompleted = true;
                            status_it->second.erase(slot_it);
                            if (status_it->second.empty())
                                WorkbenchSlotInfoMap.erase(status_it);
                            return true;
                        }
                    }
                    return false;
            }
        }
    }

    return false;
}

bool DbConnector::CheckBreakOnAdded(int PIID, int WBID, int CIID) {
    std::map<int, std::list<WorkbenchSlotInfo>>::iterator status_it;
    status_it = WorkbenchSlotInfoMap.find(WBID);

    if (status_it == WorkbenchSlotInfoMap.end())
        return false;

    WorkbenchInstance instance;
    if (!GetWorkbenchInstanceByID(WBID, instance))
        return false;

    WorkbenchComponentInWorkbenchArchetype wbcArch;
    if (!GetWorkbenchComponentInWorkbenchArchetypeByID(instance.WBAID, wbcArch))
        return false;

    WorkbenchComponent workbenchComp;
    if (!GetWorkbenchComponentByID(wbcArch.WBCID, workbenchComp))
        return false;

    // On multi-slot atm not possible to find, which slot user wanted to target, so do nothing
    if (workbenchComp.SlotCount > 1) {
        if (workbenchComp.AutoStart) {
            // For multi-processing with autostart should find available slot
        }
        return false;
    } else {
        // Check for any LockPIIDs in this WBID. If any - we should break.
        // This is valid cause only one WBSID available and won't work for multi-slot
        switch (status_it->second.begin()->status) {
            case Ready:
                if (workbenchComp.AutoStart) {
                    // Here should be auto-start
                }
                return false;
            case Process:
                // Only one slot - can safely remove full entry
                WorkbenchSlotInfoMap.erase(status_it);
                return true;
            case Finish:
                return false;
        }
    }

    return false;
}

bool DbConnector::ContainerIsWorkbench(int CIID, int& WBID) {
    ContainerInstance container;
    if (GetContainerInstanceById(CIID, container)) {
        WorkbenchInstance workbench;
        if (GetWorkbenchInstanceByPIID(container.PIID, workbench)) {
            WBID = workbench.WBID;
            return true;
        }
    }

    return false;
}
