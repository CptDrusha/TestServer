//
// Created by dml on 19.07.22.
//
#include "../Json/json11.hpp"

enum class ContainerType
{
    Input,
    Output,
    Both
};

struct ContainersInWorkbenchArchetype
{
    ContainersInWorkbenchArchetype() : WBAID(-1), CLIID(-1) {}

    ContainersInWorkbenchArchetype(int wbaid, int cliid, ContainerType type)
            : WBAID(wbaid), CLIID(cliid), Type(type) {}

    ContainersInWorkbenchArchetype(int wbaid, int cliid, std::string type)
        : WBAID(wbaid), CLIID(cliid)
    {
        StringToType(type);
    }

    ContainersInWorkbenchArchetype(json11::Json json)
    {
        WBAID = json.array_items()[0].int_value();
        CLIID = json.array_items()[1].int_value();
        StringToType(json.array_items()[2].string_value());
    }

    void SetData(int wbaid, int cliid, std::string type)
    {
        this->WBAID = wbaid;
        this->CLIID = cliid;
        StringToType(type);
    }

    json11::Json to_json() const
    {
        return json11::Json::array {
            WBAID,
            CLIID,
            TypeToString()
        };
    }

    void StringToType(std::string type)
    {
        if(type == "Input")
            Type = ContainerType::Input;
        else if(type == "Output")
            Type = ContainerType::Output;

        Type = ContainerType::Both;
    }

    std::string TypeToString() const
    {
        if(Type == ContainerType::Input)
            return "Input";
        else if(Type == ContainerType::Output)
            return "Output";

        return "Both";
    }

    int WBAID;
    int CLIID;
    ContainerType Type;
};