#pragma once
#include "game.h"
#include <vector>
#include <SFML/System/Vector2.hpp>

namespace levels
{
struct Level
{
    ShipState initState;
    TerrainData terrain;
};

Level const& get_level_data(::Level gameLevel);
}

