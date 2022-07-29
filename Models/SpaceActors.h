//
// Created by dml on 16.03.2022.
//

#include <iostream>
#include <algorithm>
#include "../Json/json11.hpp"

struct vector3 {
    float x, y, z;

    inline vector3( void ) {}
    inline vector3( const float _x, const float _y, const float _z )
        : x(_x), y(_y), z(_z) {}

    inline vector3( std::vector<float> vector )
    {
        for(int i = 0; i < vector.size(); i++)
        {
            if(i == 0)
                x = vector[i];
            else if(i == 1)
                y = vector[i];
            else
                z = vector[i];
        }
    }

    inline vector3 operator + (const vector3& a) const
    {
        return vector3( x + a.x, y + a.y, z + a.z );
    }

    inline vector3 operator + (const float a) const
    {
        return vector3( x + a, y + a, z + a );
    }

    inline float Dot(const vector3& a) const
    {
        return a.x * x + a.y * y + a.z * z;
    }

    inline std::string to_string() const
    {
        return std::string(std::to_string(x) + ","
                + std::to_string(y) + ","
                + std::to_string(z));
    }
};

struct SpaceActor {
    SpaceActor() : ActorID(-1), SID(-1), PIID(-1), Pos("0, 0, 0"), Rot("0, 0, 0") {}

    SpaceActor(int aid, int sid, std::string pos, std::string rot, int piid)
        : ActorID(aid), SID(sid), PIID(piid), Pos(pos), Rot(rot) {}

    SpaceActor(int aid, int sid, std::string pos, std::string rot, int piid, int wbiid)
            : ActorID(aid), SID(sid), PIID(piid), WBIID(wbiid), Pos(pos), Rot(rot) {}

    vector3 StringParse(std::string str, vector3& vec)
    {
        std::string delimiter = ",";
        size_t pos = 0;
        std::string token;
        std::vector<float> parseVec;
        while((pos = str.find(delimiter)) != std::string ::npos)
        {
            token = str.substr(0, pos);
            remove(token.begin(), token.end(), ' ');

            parseVec.push_back(std::stof(token));
        }

        return vector3(parseVec);
    }

    SpaceActor(json11::Json json)
    {
        ActorID = json.array_items()[0].int_value();
        SID = json.array_items()[1].int_value();
        Pos = json.array_items()[2].string_value();
        Rot = json.array_items()[3].string_value();
        PIID = json.array_items()[4].int_value();
        WBIID = json.array_items()[5].int_value();
    }

    void SetData(int aid, int sid, std::string pos, std::string rot, int piid)
    {
        this->ActorID = aid;
        this->SID = sid;
        this->Pos = pos;
        this->Rot = rot;
        this->PIID = piid;
    }

    void SetData(int aid, int sid, std::string pos, std::string rot, int piid, int wbiid)
    {
        this->ActorID = aid;
        this->SID = sid;
        this->Pos = pos;
        this->Rot = rot;
        this->PIID = piid;
        this->WBIID = wbiid;
    }

    json11::Json to_json() const {
        return json11::Json::array {
            ActorID,
            SID,
            Pos,
            Rot,
            PIID,
            WBIID
        };
    }

    int ActorID;
    int SID;
    std::string Pos;
    std::string Rot;
    int PIID;
    int WBIID;
};

struct TransferContainerToSpaceActor
{
    TransferContainerToSpaceActor() {
        actor = SpaceActor();
    }

    TransferContainerToSpaceActor(SpaceActor spaceActor) : actor(spaceActor) {}

    TransferContainerToSpaceActor(json11::Json json)
    {
        actor = SpaceActor(json.array_items()[0].array_items());
    }

    json11::Json to_json() const
    {
        return json11::Json::array
                {
                        actor.to_json()
                };
    }

    SpaceActor actor;
};

struct TransferHandToSpaceActor
{
    TransferHandToSpaceActor() {}

    TransferHandToSpaceActor(SpaceActor spaceActor, int charID) : actor(spaceActor), CharacterID(charID) {}

    TransferHandToSpaceActor(json11::Json json)
    {
        actor = SpaceActor(json.array_items()[0].array_items());
        CharacterID = json.array_items()[1].int_value();
    }

    json11::Json to_json() const
    {
        return json11::Json::array
                {
                        actor.to_json(),
                        CharacterID
                };
    }

    SpaceActor actor;
    int CharacterID;
};
