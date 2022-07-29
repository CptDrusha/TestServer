//
// Created by dml on 15.04.2022.
//

#include "../Json/json11.hpp"

class Config
{
public:
    static json11::Json ProductBackpackConfig(std::string meta)
    {
        return json11::Json::array{
            -1,
            84,
            20,
            40,
            1,
            37,
            meta,
            "Virtual"
        };
    }

    static json11::Json ContainerBackpackConfig(int PIID)
    {
        return json11::Json::array{
            -1,
            1,
            PIID
        };
    }

    static json11::Json CharacterConfig(std::string userName, int PIID)
    {
        return json11::Json::array{
            -1,
            userName,
            PIID
        };
    }

    static json11::Json UserConfig(std::string userName, int CharacterID)
    {
        return json11::Json::array{
            -1,
            userName,
            CharacterID
        };
    }
};