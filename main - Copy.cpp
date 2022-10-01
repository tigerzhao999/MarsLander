#include <iostream>
#include <vector>
#include <optional>
#include "game.h"

// A handle on the terrain data we query from the application
TerrainData const* g_terrain = nullptr;

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
    draw_line({0.0f, ignitionAltitude}, {WORLD_WIDTH, ignitionAltitude});

    // find find flat___________________________________________
    float previous = g_terrain->m_points[0].y;
    sf::Vector2f niggvector = g_terrain->m_points[0];
    float middle = 0.69f;
    float middleheight = 0.69f;
    for (int i = 1; i < g_terrain->m_points.size(); i++)
    {
        if (niggvector.y == g_terrain->m_points[i].y)
        {
            middle = (niggvector.x + g_terrain->m_points[i].x) / 2;
            middleheight = niggvector.y;
            std::cout << "* Debug " << middle << std::endl;
        }
        else
        {
            previous = g_terrain->m_points[i].y;
            niggvector = g_terrain->m_points[i];
        }
    }
    //found middle now look for tallest between pos and middle 
    float leftsearchwindow = 0;
    float rightsearchwindow = 0;
    float distance = 0;
    if (state.position.x < middle)
    {
        leftsearchwindow = state.position.x;
        rightsearchwindow = middle;
        distance = rightsearchwindow - leftsearchwindow;
    }
    else
    {
        rightsearchwindow = state.position.x;
        leftsearchwindow = middle;
        distance = rightsearchwindow - leftsearchwindow;
    }

    float tallest = 0;
    for (int i = 0; i < g_terrain->m_points.size(); i++)
    {
        if (g_terrain->m_points[i].x >= leftsearchwindow && g_terrain->m_points[i].x <= rightsearchwindow && g_terrain->m_points[i].y > tallest)
        {
            tallest = g_terrain->m_points[i].y;
            
        }
    }
    std::cout << "* tallest " << tallest << std::endl;
    std::cout << "* distance " << distance << std::endl;

    //calculate stopping distance
    float dt = 1.0f;
    //if (state.position.y > (tallest + 50))
    float sdDist = 0.0;
    if(true)
    {
        float sdvel = abs(state.velocity.x); 
        while (sdvel > 0)
        {
            sdvel += -(4* sin(0.785398) * dt);
            sdDist += sdvel * dt;
        }
    }
    std::cout << "* Stopping distance " << sdDist << std::endl;

    if (state.position.x < middle && state.velocity.x >= 0)
    {
        if (sdDist > distance - 250)
        {
            return {+45.0f, 4};
        }
        else
        {
            return {-45.0f, 4};
        }
    }
    if (state.position.x > middle && state.velocity.x <= 0)
    {
        if (sdDist > distance + 30)
        {
            return {-45.0f, 4};
        }
        else
        {
            return {+45.0f, 4};
        }
    }
    else
    {
        float angle = 0.0f;
        int power = 0; 
        if (state.velocity.y < -35)
        {
            power = 4;
        }
        else
        {
            power = 3; 
        }
        if (state.velocity.x < 0.0f)
        {
            angle = -45.0f;
        }
        if (state.velocity.x > 0.0f)
        {
            angle = +45.0f;
        }
        if (state.position.y < middleheight + 300)
        {
            angle = 0.0f;
        }
        

        return {angle, power};
    }


}

int main()
{
    // Select level
    Level level = Level::Ep2T4;

    g_terrain = &get_terrain_data(level);
    // Analyze terrain...

    play(&cmd, level, GameSpeed::x16);
    return 0;
}

