//
// Created by dml on 12.07.22.
//

#include "../Json/json11.hpp"

enum class MenuType
{
    None,
    Container,
    Market
};

struct InteractiveObjectComponent
{
    InteractiveObjectComponent() : IOCID(-1), MeshID(-1) {}

    InteractiveObjectComponent(int iocid, int meshid, std::string transform, std::string hoverTooltip,
                               bool isCollectable, std::string collectTooltip, bool isPickable,
                               std::string pickupTooltip, bool isUsable, std::string useTooltip,
                               MenuType menu, std::string openMenuTooltip)
            : IOCID(iocid), MeshID(meshid), Transform(transform), HoverTooltip(hoverTooltip), CollectTooltip(collectTooltip), IsCollectable(isCollectable), IsPickable(isPickable),
              PickupTooltip(pickupTooltip), IsUsable(isUsable), UseTooltip(useTooltip), Menu(menu), OpenMenuTooltip(openMenuTooltip) {}

    InteractiveObjectComponent(int iocid, int meshid, std::string transform, std::string hoverTooltip,
                               bool isCollectable, std::string collectTooltip, bool isPickable,
                               std::string pickupTooltip, bool isUsable, std::string useTooltip,
                               std::string menu, std::string openMenuTooltip)
                               : IOCID(iocid), MeshID(meshid), Transform(transform), HoverTooltip(hoverTooltip), CollectTooltip(collectTooltip), IsCollectable(isCollectable), IsPickable(isPickable),
                                 PickupTooltip(pickupTooltip), IsUsable(isUsable), UseTooltip(useTooltip), OpenMenuTooltip(openMenuTooltip)
    {
        Menu = MenuFromString(menu);
    }

    InteractiveObjectComponent(json11::Json json)
    {
        IOCID = json.array_items()[0].int_value();
        MeshID = json.array_items()[1].int_value();
        Transform = json.array_items()[2].string_value();
        HoverTooltip = json.array_items()[3].string_value();
        IsCollectable = json.array_items()[4].bool_value();
        CollectTooltip = json.array_items()[5].string_value();
        IsPickable = json.array_items()[6].bool_value();
        PickupTooltip = json.array_items()[7].string_value();
        IsUsable = json.array_items()[8].bool_value();
        UseTooltip = json.array_items()[9].string_value();
        MenuFromString(json.array_items()[10].string_value());
        OpenMenuTooltip = json.array_items()[11].string_value();
    }

    MenuType MenuFromString(std::string type)
    {
        if(type == "None")
            return MenuType::None;
        else if(type == "Container")
            return MenuType::Container;
        else if(type == "Market")
            return MenuType::Market;

        return MenuType::None;
    }

    std::string MenuToString() const
    {
        if(Menu == MenuType::None)
            return "None";
        else if(Menu == MenuType::Container)
            return "Container";
        else if(Menu == MenuType::Market)
            return "Market";

        return "None";
    }

    void SetData(int iocid, int meshid, std::string transform, std::string hoverTooltip,
                 bool isCollectable, std::string collectTooltip, bool isPickable,
                 std::string pickupTooltip, bool isUsable, std::string useTooltip,
                 std::string menu, std::string openMenuTooltip)
    {
        this->IOCID = iocid;
        this->MeshID = meshid;
        this->Transform = transform;
        this->HoverTooltip = hoverTooltip;
        this->IsCollectable = isCollectable;
        this->CollectTooltip = collectTooltip;
        this->IsPickable = isPickable;
        this->PickupTooltip = pickupTooltip;
        this->IsUsable = isUsable;
        this->UseTooltip = useTooltip;
        this->Menu = MenuFromString(menu);
        this->OpenMenuTooltip = openMenuTooltip;
    }

    json11::Json to_json() const
    {
        return json11::Json::array {
            IOCID,
            MeshID,
            Transform,
            HoverTooltip,
            IsCollectable,
            CollectTooltip,
            IsPickable,
            PickupTooltip,
            IsUsable,
            UseTooltip,
            MenuToString(),
            OpenMenuTooltip
        };
    }

    int IOCID;
    int MeshID;
    std::string Transform;
    std::string HoverTooltip;
    bool IsCollectable;
    std::string CollectTooltip;
    bool IsPickable;
    std::string PickupTooltip;
    bool IsUsable;
    std::string UseTooltip;
    MenuType Menu;
    std::string OpenMenuTooltip;
};