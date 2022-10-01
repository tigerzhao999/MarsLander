#pragma once
#include <vector>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

constexpr float WORLD_WIDTH = 7000.0f;
constexpr float WORLD_HEIGHT = 3000.0f;

struct TerrainData
{
    std::vector<sf::Vector2f> m_points;
};

struct ShipState
{
    sf::Vector2f position;
    sf::Vector2f velocity;
    int fuel;
    float rotation;
    int power;
    float time;
};

/* Issue a command to the lander
 *
 * Rotation is an angle in degrees relative to upright. +90 corresponds to the
 * ship pointing directly to the left, and -90 corresponds to the ship pointing
 * directly to the right
 * 
 * Power is an integer ranging from 0 to 4, corresponding to the engine thrust.
 * Thrust values correspond directly to acceleration in m/s^2
 * 
 * Note that rotation and power can only change by +/- 15deg and +/- 1 per 
 * timestep, respectively.
 */
struct ShipCommand
{
    float rotation;
    int power;
};

enum class Level
{
    Ep1T1,
    Ep2T1,
    Ep2T2,
    Ep2T3,
    Ep2T4,
    Ep2T5,
    Ep3T1,
    Ep3T2
};

enum class GameSpeed
{
    x1,
    x2,
    x4,
    x8,
    x16
};

// user control function pointer type alias
using control_t = ShipCommand(*)(ShipState const&);

/* Debug drawing functions
 * 
 * These are "immediate mode" drawing functions that will draw graphics that last
 * for one frame and then be cleared. If you want visuals to persist, you'll have
 * to draw them every frame.
 */

// Draw a line connecting two points
void draw_line(sf::Vector2f a, sf::Vector2f b, sf::Color color=sf::Color::White);
// Draw a circle at a given position
void draw_point(sf::Vector2f pos, float radius, sf::Color color=sf::Color::White);

// Fetch the terrain data for the specified level
TerrainData const& get_terrain_data(Level gameLevel);

// Play the game
// Pass the user's update function and specify level and time acceleration
void play(control_t fnc, Level gameLevel, GameSpeed speed=GameSpeed::x1);

// Run the game through to completion all at once
std::vector<ShipState> run_simulation(control_t fnc, Level gameLevel);

