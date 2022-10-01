#include <iostream>
#include <vector>
#include <optional>
#include "game.h"
#include "ga.h"

// A handle on the terrain data we query from the application
TerrainData const* g_terrain = nullptr;

static int mainstep = 0;
Individual maincurrIndividual = iri();

struct State
{
    sf::Vector2f pos;
    sf::Vector2f vel;
};


// Simulate a trajectory starting at a certain state (position, velocity)
// and simualte the future trajectory for "period" seconds. Assumes a constant
// attitude angle and thrust power throughout trajectory simulation
std::vector<State> simulate_trajectory(
    State initState, float angle, float thrust,
    float period, float dt=1.0f)
{
    sf::Vector2f pos = initState.pos;
    sf::Vector2f vel = initState.vel;

    angle *= (3.14159265359 / 180.);
    const sf::Vector2f acceleration(
        0.0f + thrust * sin(-angle),
        -3.711f + thrust * cos(-angle));

    std::vector<State> result;
    result.reserve(2 + static_cast<size_t>(period / dt));

    result.push_back({pos, vel});

    for (float t = 0.0f; t < period; t += dt)
    {
        vel += acceleration * dt;
        pos += vel * dt;

        result.push_back({pos, vel});
    }

    return result;
}


// Draw a list of states as a trajectory, drawing a point at each position and
// connecting them with a continuous line
void draw_traj(std::vector<State> const& traj)
{
    for (size_t i = 1; i < traj.size(); i++)
    {
        draw_line(traj[i].pos, traj[i - 1].pos);
        sf::Color color = sf::Color::White;
        if (traj[i].vel.length() > 40.0f)
        {
            color = sf::Color::Red;
        }
        draw_point(traj[i].pos, 10.0f, color);
    }
}

ShipCommand cmd(ShipState const& state)
{
    // Simulate ship trajectory given current params
    std::vector<State> traj = simulate_trajectory(
        {state.position, state.velocity},
        state.rotation, state.power, 75.0f);
    // Plot the result
    draw_traj(traj);

    // Put a point on each terrain vertex
    for (int i = 0; i < g_terrain->m_points.size(); i++)
    {
        draw_point(g_terrain->m_points[i], 10.0f);
    }

    // Hand-picked altitude to start suicide burn
    float ignitionAltitude = 2600.0f;

    // Draw a horizontal line at that altitude just for fun and visualization
    //draw_line({0.0f, ignitionAltitude}, {WORLD_WIDTH, ignitionAltitude});

    //------------------------------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------------------------
    return maincurrIndividual.gene.at(mainstep++);   
}

void on_simulation_end(ShipState const& ship)
{
    //domath
    //fittness(ship, currIndividual, g_terrain);
}

int main()
{
    // Select level
    Level level = Level::Ep2T3;

    g_terrain = &get_terrain_data(level);
    // Analyze terrain...

    //learn(&cmd, level, GameSpeed::x16);

    //learn(&randtraj, level, GameSpeed::x16);
    
    //std::vector<ShipState> sim_result = run_simulation(&gaCMD, level);
    /*ShipState end = sim_result.at(sim_result.size() - 1);
    std::cout << end.position.x;
    std::cout << "\n \n";
    std::cout << end.position.y;
    */

    runGA(level, g_terrain);

    //play(&cmd, level, GameSpeed::x16);
    return 0;
}

