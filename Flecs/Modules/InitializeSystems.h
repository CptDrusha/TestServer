#pragma once

#include "../Flecs/flecs.h"
#include "../../Server/ENetServer.h"


// Register systems in flecs world
void InitializeSystems(flecs::world& ecsWorld, ENetServer* server, QuadTreeNode* quadTreeRoot);