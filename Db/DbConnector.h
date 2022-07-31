#pragma once

#include <mutex>
#include "vector"
#include "DbCommon.h"
#include "../Models/ModelsCommon.h"
#include "../Models/ContainerArchetype.h"

class DbConnector {
public:
    DbConnector();
    virtual ~DbConnector();

    bool Connect();
    bool isConnect();

    /** Check if product transfer is possible.
     * @param deltaMass Desired mass to transfer.
     * @param partialMass Mass we CAN transfer (may be lower then deltaMass)
     * @return TRUE if product can be transferred at least partially.
     */
    bool CheckTransferPossible(ProductInstance product, int CIID, float deltaMass, float& partialMass);

    bool TransferProduct(int PIID, int CIID, float deltaMass);
    bool TransferProductToHand(int PIID, int CharacterID);
    bool TransferProductToSpaceActor(SpaceActor& spaceActor);

    bool TransferProductContainerToHand(int PIID, int CharacterID);
    bool TransferProductHandToContainer(int PIID, int CIID);

    bool TransferProductContainerToSpaceActor(SpaceActor& spaceActor);

    bool TransferProductSpaceActorToHand(int ActorID, int CharacterID);
    bool TransferProductHandToSpaceActor(SpaceActor& spaceActor);

    bool Split(ProductInstance& instance, float deltaMass, float density);

    float GetContainerMassHeld(int CIID);
    float GetProductTotalMass(int PIID);

    float GetContainerVolumeHeld(int CIID);
    float GetProductTotalVolume(int PIID);
    bool ProductCanBeAddedToContainer(ContainerInstance containerInstance, ProductInstance productInstance);

    uint8_t AsBitFlag(ProductArchetypeType Enum)
    {
        uint8_t A = static_cast<uint8_t>(Enum);
        uint8_t B = 1;
        while(A > 0)
        {
            B = B << 1;
            A--;
        }
        return B;
    }

    bool HasFlag(const uint8_t Bitmask, const ProductArchetypeType Type)
    {
        return (Bitmask & AsBitFlag(Type)) != 0;
    }

    // Methods for working with CharacterBody Structs
        // Body Type
    bool GetAllCharacterBodyTypes(std::vector<CharacterBodyType>& BodyTypes);
    bool GetCharacterBodyType(uint32_t id, CharacterBodyType& BodyType);
    int UpdateCharacterBodyType(CharacterBodyType& BodyType);
    bool DeleteCharacterBodyType(uint32_t id);
        // Body Cloth
    bool GetCharacterBodyCloth(uint32_t id, std::vector<CharacterBodyCloth>& BodyCloths);
    int UpdateCharacterBodyCloth(CharacterBodyCloth& BodyCloth);
    bool DeleteCharacterBodyCloth(uint32_t id);

    // Methods for working with Archetypes table
        // ProductArchetypes
    bool GetAllArchetypes(std::vector<ProductArchetype>& archetypes);
    bool GetArchetypeById(uint32_t id, ProductArchetype& archetype);
    bool GetArchetypeByName(std::string name, ProductArchetype& archetype);
    int UpdateArchetype(ProductArchetype& archetype);
    bool DeleteArchetype(uint32_t id);
        // ContainerArchetypes
    bool GetAllContainerArchetypes(std::vector<ContainerArchetype>& containerArchetypes);
    bool GetContainerArchetypeById(uint32_t id, ContainerArchetype& containerArchetype);
    int UpdateContainerArchetype(ContainerArchetype& containerArchetype);
    bool DeleteContainerArchetype(uint32_t id);
        // WorkbenchArchetype
    bool GetAllWorkbenchArchetypes(std::vector<WorkbenchArchetype>& workbenchArchetypes);
    bool GetWorkbenchArchetypeByPAID(uint32_t id, WorkbenchArchetype& workbenchArchetype);
    bool DeleteWorkbenchArchetype(uint32_t id);

    bool GetContainersInProductArchetypeByPAID(uint32_t id, ContainersInProductArchetype& productContainerArchetype);

    // Methods for working with GraphicsSets table
        // GraphicsSets
    bool GetAllGraphicsSets(std::vector<GraphicsSet>& graphicsSets);
    bool GetGraphicsSetById(uint32_t id, GraphicsSet& graphicsSet);
    int UpdateGraphicsSet(GraphicsSet& graphicsSet);
    bool DeleteGraphicsSet(uint32_t id);
    // GraphicsSets refs*
        // GraphicsSetsActors
    bool GetAllGraphicsSetsActors(std::vector<GraphicsSetsActor>& graphicsSetsActors);
    bool GetGraphicsSetsActorsByGSID(uint32_t id, std::vector<GraphicsSetsActor>& graphicsSetsActors);
    bool GetGraphicsSetsActorsByActorID(uint32_t id, std::vector<GraphicsSetsActor>& graphicsSetsActors);
    int UpdateGraphicsSetsActors(GraphicsSetsActor& graphicsSetsActor);
    bool DeleteGraphicsSetsActorsByGSID(uint32_t id);
    bool DeleteGraphicsSetsActorsByActorID(uint32_t id);
        // GraphicsSetsProducts
    bool GetAllGraphicsSetsProducts(std::vector<GraphicsSetsProduct>& graphicsSetsProducts);
    bool GetGraphicsSetsProductById(uint32_t id, GraphicsSetsProduct& graphicsSetsProduct);
    bool GetGraphicSetProductByPAID(uint32_t id, GraphicsSetsProduct& graphicSetsProduct);
    int UpdateGraphicsSetsProduct(GraphicsSetsProduct& graphicsSetsProduct);
    bool DeleteGraphicsSetsProduct(uint32_t id);

    // Methods for working with Instances table
        // ProductInstances
    bool GetAllInstances(std::vector<ProductInstance>& instances);
    bool GetProductInstancesBySID(uint32_t id, std::vector<ProductInstance>& instances);
    bool GetInstanceById(uint32_t id, ProductInstance& instance);
    int UpdateInstance(ProductInstance& instance);
    bool DeleteInstance(uint32_t id);
        // ContainerInstances
    bool GetAllContainerInstances(std::vector<ContainerInstance>& instances);
    bool GetContainerInstancesByPIIDs(std::vector<ProductInstance> Products, std::vector<ContainerInstance>& instances);
    bool GetContainerInstancesByPIID(uint32_t id, std::vector<ContainerInstance>& instances);
    bool GetContainerInstanceById(uint32_t id, ContainerInstance& instance);
    int UpdateContainerInstance(ContainerInstance& instance);
    bool DeleteContainerInstance(uint32_t id);

    bool GetAllWorkbenchInstances(std::vector<WorkbenchInstance>& instances);
    bool GetWorkbenchInstanceByPIID(uint32_t id, WorkbenchInstance& instance);
    bool GetWorkbenchInstanceByID(uint32_t id, WorkbenchInstance& instance);
    int UpdateWorkbenchInstance(WorkbenchInstance& instance);
    bool DeleteWorkbenchInstance(uint32_t id);

    bool GetAllWorkbenchSchemes(std::vector<WorkbenchScheme>& schemes);
    bool GetWorkbenchSchemeByID(uint32_t id, WorkbenchScheme& scheme);

    bool GetAllConversionProductsWorkbenchSchemes(std::vector<ConversionProductsWorkbenchScheme>& schemes);
    bool GetConversionProductsWorkbenchSchemesBySchemeID(uint32_t id, std::vector<ConversionProductsWorkbenchScheme>& schemes);
    bool GetConversionProductsWorkbenchSchemeByID(uint32_t id, ConversionProductsWorkbenchScheme& scheme);

    bool GetAllConversionSourcesWorkbenchSchemes(std::vector<ConversionSourcesWorkbenchScheme>& schemes);
    bool GetConversionSourcesWorkbenchSchemesBySchemeID(uint32_t id, std::vector<ConversionSourcesWorkbenchScheme>& schemes);
    bool GetConversionSourcesWorkbenchSchemeByID(uint32_t id, ConversionSourcesWorkbenchScheme& scheme);

    bool GetAllConversionWorkbenchSchemes(std::vector<ConversionWorkbenchScheme>& schemes);
    bool GetConversionWorkbenchSchemeByID(uint32_t id, ConversionWorkbenchScheme& scheme);

    bool GetAllWorkbenchComponents(std::vector<WorkbenchComponent>& components);
    bool GetWorkbenchComponentByID(uint32_t id, WorkbenchComponent& component);

    bool GetAllWorkbenchComponentSlots(std::vector<WorkbenchComponentSlot>& components);
    bool GetWorkbenchComponentSlotsByWBCID(uint32_t id, std::vector<WorkbenchComponentSlot>& components);
    bool GetWorkbenchComponentSlotByID(uint32_t id, WorkbenchComponentSlot& component);

    bool GetAllContainersInWorkbenchArchetype(std::vector<ContainersInWorkbenchArchetype>& containers);
    bool GetContainersInWorkbenchArchetypeByID(uint32_t id, std::vector<ContainersInWorkbenchArchetype>& containers);

    bool GetAllWorkbenchComponentsInWorkbenchArchetype(std::vector<WorkbenchComponentInWorkbenchArchetype>& components);
    bool GetWorkbenchComponentInWorkbenchArchetypeByID(uint32_t id, WorkbenchComponentInWorkbenchArchetype& component);

    // Methods for working with Containers table
        // ContainersInActors
    bool GetAllContainersInActors(std::vector<ContainersInActor>& containersInActors);
    bool GetContainersInActorById(uint32_t id, ContainersInActor& containersInActor);
    int UpdateContainersInActor(ContainersInActor& containersInActor);
    bool DeleteContainersInActor(uint32_t id);
        // ProductsInActors
    bool GetAllProductsInActors(std::vector<ProductsInActor>& productsInActors);
    bool GetProductsInActorById(uint32_t id, ProductsInActor& productsInActor);
    int UpdateProductsInActor(ProductsInActor& productsInActor);
    bool DeleteProductsInActor(uint32_t id);
        // ProductsInContainers
    bool GetAllProductsInContainers(std::vector<ProductsInContainer>& productsInContainers);
    bool GetProductsInContainerById(uint32_t id, std::vector<ProductsInContainer>& productsInContainer);
    bool GetProductInContainerByPIID(uint32_t id, ProductsInContainer& productInContainer);
    int UpdateProductsInContainer(ProductsInContainer& productsInContainer);
    bool DeleteProductsInContainer(uint32_t id);
        // ProductsInCharacters
    bool GetAllProductsInCharacters(std::vector<ProductInCharacter>& productsInCharacters);
    bool GetProductsInCharacterByCharacterId(uint32_t id, std::vector<ProductInCharacter>& productsInCharacter);
    bool GetProductInCharacterById(uint32_t id, ProductInCharacter& productInCharacter);

    bool GetSlotIsEmpty(uint32_t id, std::string slot, ProductInCharacter& productInCharacter);

    int UpdateProductInCharacter(ProductInCharacter& productInCharacter);
    bool DeleteProductInCharacter(uint32_t id);
        // ContainersInProducts
    bool GetAllContainersInProducts(std::vector<ContainerInProduct>& containersInProducts);
    bool GetContainerInProductById(uint32_t id, ContainerInProduct& containerInProduct);
    bool GetContainerInProductByCIID(uint32_t id, ContainerInProduct& containerInProduct);
    int UpdateContainerInProduct(ContainerInProduct& containerInProduct);
    bool DeleteContainerInProduct(uint32_t id);

    // Methods for working with Actors table
        // SpaceActors
    bool GetAllSpaceActors(std::vector<SpaceActor>& spaceActors);
    bool GetSpaceActorBySID(uint32_t id, std::vector<SpaceActor>& spaceActors);
    bool GetSpaceActorByActorID(uint32_t id, SpaceActor& spaceActor);
    bool GetSpaceActorByPIID(uint32_t id, SpaceActor& spaceActor);
    int UpdateSpaceActor(SpaceActor& spaceActor);
    bool DeleteSpaceActorByPIID(uint32_t id);
    bool DeleteSpaceActor(uint32_t id);
        // Space
    bool GetAllSpaceAreas(std::vector<Space>& spaceAreas);
    bool GetSpaceAreaBySID(uint32_t id, Space& spaceArea);
    int UpdateSpaceArea(Space& spaceArea);
    bool DeleteSpaceArea(uint32_t id);
        // ContainerComponent
    bool GetAllContainerComponents(std::vector<ContainerComponent>& spaceComponents);
    bool GetContainerComponentByCCID(uint32_t id, ContainerComponent& spaceComponent);
    bool DeleteContainerComponent(uint32_t id);
        // InteractiveObjectComponent
    bool GetAllInteractiveObjectComponents(std::vector<InteractiveObjectComponent>& interactComponents);
    bool GetInteractiveObjectComponentByIOCID(uint32_t id, InteractiveObjectComponent& interactComponent);
    bool DeleteInteractiveObjectComponent(uint32_t id);
        // ContainerInstanceComponent
    bool GetAllProductArchetypeComponent(std::vector<ProductArchetypeComponent>& spaceArchetypeComponents);
    bool GetProductArchetypeComponentsByPAID(uint32_t id, std::vector<ProductArchetypeComponent>& spaceArchetypeComponents);
    bool DeleteProductArchetypeComponentsByPAID(uint32_t id);

    // Player Inventory
    bool GetPlayerInventoryByID(uint32_t id, PlayerInventory& inventory);
    int UpdatePlayerInventory(PlayerInventory& inventory);
    bool DeletePlayerInventory(uint32_t id);

    int UpdateClientData(User& user, uint64_t hashedPassword);
    bool DeleteClientData(uint32_t id);
    int CheckAuthData(std::string userName, uint64_t hashedPassword);

    bool GetAllCharacters(std::vector<Character>& characters);
    bool GetCharacterByID(uint32_t id, Character& character);
    int UpdateCharacter(Character& character);
    bool DeleteCharacter(uint32_t id);

    int RegistrationClient(std::string userName, uint64_t hashedPassword);

    // Workbench and schemes

    enum WorkbenchSlotStatus {
        Ready,
        Process,
        Finish
    };

    struct WorkbenchSlotInfo {
        WorkbenchSlotInfo() {};

        WorkbenchSlotInfo(int _WBSID, WorkbenchSlotStatus _status, std::list<int> _PIIDs)
            : WBSID(_WBSID), status(_status), PIIDs(_PIIDs) {};

        int WBSID = 0;
        WorkbenchSlotStatus status = Ready;
        std::list<int> PIIDs = {};

        void Reset() {
            status = Ready;
            PIIDs.clear();
        }

        bool HasPIID(int PIID) {
            for (auto& elem : PIIDs) {
                if (elem == PIID)
                    return true;
            }
            return false;
        }

        bool HasPIID(int PIID, std::list<int>::iterator& iterator) {
            for (iterator = PIIDs.begin(); iterator != PIIDs.end(); iterator++) {
                if (*iterator == PIID)
                    return true;
            }
            return false;
        }
    };

    /** WBID : SlotInfo */
    std::map<int, std::list<WorkbenchSlotInfo>> WorkbenchSlotInfoMap;
    /** PIID : {WBID, WBSID} */
    //std::map<int, std::vector<int>> LockPIIDs;
    /** PIID : {WBID, WBSID} || Products that was created as a conversion result. */
    //std::map<int, std::vector<int>> CompletedPIIDs;
    bool CheckConversionProcess(ConversionInfo info);
    bool StartConversionProcess(ConversionInfo info);

    bool ContainerIsWorkbench(int CIID, int& WBID);
    /** @param bFullRemove Flag that all product was removed, not just part of it
     * @param WBSID Return valid value if function returned TRUE. */
    bool CheckBreakOnRemoval(int PIID, bool bFullRemove, int WBID, int CIID, int& WBSID, bool& bIsCompleted);
    bool CheckBreakOnAdded(int PIID, int WBID, int CIID);

private:
        // BodyType
    int AddCharacterBodyType(CharacterBodyType& BodyType);

    // Helper methods for creating add sql requests
        // Archetypes
    int AddArchetype(ProductArchetype& archetype);
    int AddContainerArchetype(ContainerArchetype& containerArchetype);
        // Instances
    int AddInstance(ProductInstance& instance);
    int AddContainerInstance(ContainerInstance& containerInstance);
    int AddWorkbenchInstance(WorkbenchInstance& workbenchInstance);
        // GraphicsSets
    int AddGraphicsSet(GraphicsSet& graphicsSet);
    int AddGraphicsSetsActor(GraphicsSetsActor& graphicsSetActor);
    int AddGraphicsSetsProduct(GraphicsSetsProduct& graphicsSetProduct);
        // Containers
    int AddContainersInActor(ContainersInActor& containersInActor);
    int AddProductsInActor(ProductsInActor& productsInActor);
    int AddProductsInContainer(ProductsInContainer& productsInContainer);
    int AddContainerInProduct(ContainerInProduct& containerInProduct);
        // Actors
    int AddSpaceActor(SpaceActor& spaceActor);
        // SpaceArea
    int AddSpaceArea(Space& spaceArea);
        // PlayerInventory
    int AddPlayerInventory(PlayerInventory& inventory);
        // ClientData
    int AddClientData(User& user, uint64_t hashedPassword);
        // Character
    int AddCharacter(Character& character);

    int AddProductInCharacter(ProductInCharacter& productInCharacter);

        // BodyType
    int FindCharacterBodyType(CharacterBodyType& BodyType);
    // Helper methods for checking thar object is unique
        // Archetypes
    int FindArchetypeByFields(ProductArchetype& archetype);
    int FindContainerArchetypeByFields(ContainerArchetype& containerArchetype);
        // Instances
    int FindInstanceByFields(ProductInstance& instance);
    int FindContainerInstanceByFields(ContainerInstance& containerInstance);
    int FindWorkbenchInstanceByFields(WorkbenchInstance& workbenchInstance);
    // GraphicsSets
    int FindGraphicsSetByFields(GraphicsSet& graphicsSet);
    int FindGraphicsSetsActorByFields(GraphicsSetsActor& graphicsSetsActor);
    int FindGraphicsSetsProductByFields(GraphicsSetsProduct& graphicsSetsProduct);
        // Containers
    int FindContainersInActorByFields(ContainersInActor& containersInActor);
    int FindProductsInActor(ProductsInActor& productsInActor);
    int FindProductsInContainer(ProductsInContainer& productsInContainer);
    int FindContainerInProduct(ContainerInProduct& containerInProduct);
        // Actors
    int FindSpaceActor(SpaceActor& spaceActor);
        // Space Areas
    int FindSpaceArea(Space& spaceArea);

    int FindPlayerInventory(PlayerInventory& inventory);

    int FindClientData(User& user);
    int FindCharacter(Character& character);

    int FindProductInCharacter(ProductInCharacter& productInCharacter);


    void GenerateSpaceAreas(int Xpos, int Ypos, float Height, float Width, float Length);

    template <typename vectorType>
    bool Find(const std::vector<vectorType>& vector, const vectorType& element) const {
        for(auto& value : vector)
        {
            if(value == element)
                return true;
        }

        return false;
    }

    template <typename keyType, typename valueType>
    void SortByValue(std::map<keyType, valueType>& Map)
    {
        std::vector<std::pair<keyType, valueType>> A;

        for(auto& it : Map)
        {
            A.push_back(it);
        }

        std::sort(A.begin(), A.end(), [](std::pair<keyType, valueType>& a, std::pair<keyType, valueType>& b){
            return a.second > b.second;
        });

        Map.clear();
        for(auto& it : A)
        {
            Map.insert(it);
        }

        return;
    }


    float RandomFloat(uint64_t seed, float a, float b) const
    {
        srand(seed);
        float random = ((float) rand()) / (float) RAND_MAX;
        float diff = b - a;
        float r = random * diff;
        return a + r;
    }

    bool RandomBool(uint64_t seed)
    {
        srand(seed);
        return 0 + (rand() % (1 - 0 + 1)) == 1;
    }

    uint64_t hashedData(std::string data)
    {
        uint64_t Hash = 12345;
        for(size_t i = 0; i < data.size(); ++i)
        {
            Hash = 33 * Hash + static_cast<unsigned char>(data[i]);
        }

        return Hash;
    }

    // Driver will handle connections
    sql::Driver* driver;

    // Connection to database
    sql::Connection* conn;

    // Mutex for thread-safe operations
    std::mutex dbMutex;
};