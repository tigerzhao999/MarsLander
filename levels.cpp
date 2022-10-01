#include "game.h"
#include "levels.h"
#include <iostream>

namespace levels
{
Level ep1t1
{
    .initState =
    {
        .position = sf::Vector2f(2500, 2500),
        .velocity = sf::Vector2f(0, 0),
        .fuel = 500,
        .rotation = 0.0f,
        .power = 0
    },
    .terrain = {{
        {0, 100},
        {1000, 500},
        {1500, 100},
        {3000, 100},
        {5000, 1500},
        {6999, 1000}
    }}
};

Level ep2t1
{
    .initState =
    {
        .position = sf::Vector2f(2500.0f, 2700.0f),
        .velocity = sf::Vector2f(0.0f, 0.0f),
        .fuel = 550,
        .rotation = 0.0f,
        .power = 0
    },
    .terrain = {{
        {0, 100},
        {1000, 500},
        {1500, 1500},
        {3000, 1000},
        {4000, 150},
        {5500, 150},
        {6999, 800}
    }}
};

Level ep2t2
{
    .initState =
    {
        .position = sf::Vector2f(6500.0f, 2800.0f),
        .velocity = sf::Vector2f(-100.0f, 0.0f),
        .fuel = 600,
        .rotation = 90.0f,
        .power = 0
    },
    .terrain = {{
        {0, 100},
        {1000, 500},
        {1500, 100},
        {3000, 100},
        {3500, 500},
        {3700, 200},
        {5000, 1500},
        {5800, 300},
        {6000, 1000},
        {6999, 2000}
    }}
};

Level ep2t3
{
    .initState =
    {
        .position = sf::Vector2f(6500.0f, 2800.0f),
        .velocity = sf::Vector2f(-90.0f, 0.0f),
        .fuel = 750,
        .rotation = 90.0f,
        .power = 0
    },
    .terrain = {{
        {0, 100},
        {1000, 500},
        {1500, 1500},
        {3000, 1000},
        {4000, 150},
        {5500, 150},
        {6999, 800}
    }}
};

Level ep2t4
{
    .initState =
    {
        .position = sf::Vector2f(500.0f, 2700.0f),
        .velocity = sf::Vector2f(100.0f, 0.0f),
        .fuel = 800,
        .rotation = -90.0f,
        .power = 0
    },
    .terrain = {{
        {0, 1000},
        {300, 1500},
        {350, 1400},
        {500, 2000},
        {800, 1800},
        {1000, 2500},
        {1200, 2100},
        {1500, 2400},
        {2000, 1000},
        {2200, 500},
        {2500, 100},
        {2900, 800},
        {3000, 500},
        {3200, 1000},
        {3500, 2000},
        {3800, 800},
        {4000, 200},
        {5000, 200},
        {5500, 1500},
        {6999, 2800},
    }}
};

Level ep2t5
{
    .initState =
    {
        .position = sf::Vector2f(6500.0f, 2700.0f),
        .velocity = sf::Vector2f(-50.0f, 0.0f),
        .fuel = 1000,
        .rotation = 90.0f,
        .power = 0
    },
    .terrain = {{
        {0, 1000},
        {300, 1500},
        {350, 1400},
        {500, 2100},
        {1500, 2100},
        {2000, 200},
        {2500, 500},
        {2900, 300},
        {3000, 200},
        {3200, 1000},
        {3500, 500},
        {3800, 800},
        {4000, 200},
        {4200, 800},
        {4800, 600},
        {5000, 1200},
        {5500, 900},
        {6000, 500},
        {6500, 300},
        {6999, 500},
    }}
};

Level ep3t1
{
    .initState =
    {
        .position = sf::Vector2f(6500.0f, 2600.0f),
        .velocity = sf::Vector2f(-20.0f, 0.0f),
        .fuel = 1000,
        .rotation = 45.0f,
        .power = 0
    },
    .terrain = {{
        {0, 450},
        {300, 750},
        {1000, 450},
        {1500, 650},
        {1800, 850},
        {2000, 1950},
        {2200, 1850},
        {2400, 2000},
        {3100, 1800},
        {3150, 1550},
        {2500, 1600},
        {2200, 1550},
        {2100, 750},
        {2200, 150},
        {3200, 150},
        {3500, 450},
        {4000, 950},
        {4500, 1450},
        {5000, 1550},
        {5500, 1500},
        {6000, 950},
        {6999, 1750},
    }}
};

Level ep3t2
{
    .initState =
    {
        .position = sf::Vector2f(6500.0f, 2000.0f),
        .velocity = sf::Vector2f(0.0f, 0.0f),
        .fuel = 1200,
        .rotation = 45.0f,
        .power = 0
    },
    .terrain = {{
        {0, 1800},
        {300, 1200},
        {1000, 1550},
        {2000, 1200},
        {2500, 1650},
        {3700, 220},
        {4700, 220},
        {4750, 1000},
        {4700, 1650},
        {4000, 1700},
        {3700, 1600},
        {3750, 1900},
        {4000, 2100},
        {4900, 2050},
        {5100, 1000},
        {5500, 500},
        {6200, 800},
        {6999, 600}
    }}
};

Level const& get_level_data(::Level gameLevel)
{
    switch (gameLevel)
    {
    case ::Level::Ep1T1: return ep1t1;
    case ::Level::Ep2T1: return ep2t1;
    case ::Level::Ep2T2: return ep2t2;
    case ::Level::Ep2T3: return ep2t3;
    case ::Level::Ep2T4: return ep2t4;
    case ::Level::Ep2T5: return ep2t5;
    case ::Level::Ep3T1: return ep3t1;
    case ::Level::Ep3T2: return ep3t2;
    default:
        std::cerr << "ERROR: Bad level ID\n";
        return ep1t1;
    }
}
} // namespace levels

