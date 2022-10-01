#include "game.h"
#include <SFML/Graphics.hpp>
#include <array>
#include <iostream>
#include <format>
#include <optional>
#include "levels.h"

constexpr size_t SCREEN_WIDTH = 1400;
constexpr size_t SCREEN_HEIGHT = 600;

constexpr float SS_TO_WORLD = WORLD_HEIGHT / SCREEN_HEIGHT;
constexpr bool g_DEBUG_GRAPHICS = false;

// Ship model lines
std::vector<sf::Vertex> g_shipVerts
{
    // Top
    {sf::Vector2f(-10, 0), sf::Color::White},
    {sf::Vector2f(0, 10), sf::Color::White},

    {sf::Vector2f(0, 10), sf::Color::White},
    {sf::Vector2f(10, 0), sf::Color::White},

    {sf::Vector2f(10, 0), sf::Color::White},
    {sf::Vector2f(0, -10), sf::Color::White},

    {sf::Vector2f(0, -10), sf::Color::White},
    {sf::Vector2f(-10, 0), sf::Color::White},

    // Bottom
    {sf::Vector2f(-8, -12), sf::Color::White},
    {sf::Vector2f(8, -12), sf::Color::White},

    {sf::Vector2f(8, -12), sf::Color::White},
    {sf::Vector2f(8, -8), sf::Color::White},

    {sf::Vector2f(8, -8), sf::Color::White},
    {sf::Vector2f(2, -8), sf::Color::White},

    {sf::Vector2f(-2, -8), sf::Color::White},
    {sf::Vector2f(-8, -8), sf::Color::White},

    {sf::Vector2f(-8, -8), sf::Color::White},
    {sf::Vector2f(-8, -12), sf::Color::White},

    // Legs
    {sf::Vector2f(6, -12), sf::Color::White},
    {sf::Vector2f(12, -18), sf::Color::White},

    {sf::Vector2f(-6, -12), sf::Color::White},
    {sf::Vector2f(-12, -18), sf::Color::White},

    // Feet
    {sf::Vector2f(8, -18), sf::Color::White},
    {sf::Vector2f(16, -18), sf::Color::White},

    {sf::Vector2f(-8, -18), sf::Color::White},
    {sf::Vector2f(-16, -18), sf::Color::White}
};

std::vector<sf::Vector2f> g_shipColliders
{
    // Landing feet -- these can be landed on
    sf::Vector2f(12, -18) * SS_TO_WORLD,
    sf::Vector2f(-12, -18) * SS_TO_WORLD,

    // Chassis -- collision with any of these means INSTANT DEATH!
    // Top
    sf::Vector2f(0, 10) * SS_TO_WORLD,
    // Sides
    sf::Vector2f(10, 0) * SS_TO_WORLD,
    sf::Vector2f(-10, 0) * SS_TO_WORLD
};

// Global debug graphics data
std::vector<sf::Vertex> g_debugLines;
std::vector<sf::CircleShape> g_debugPoints;

struct State
{
    // User control function

    control_t m_update;
    ShipCommand m_shipCmd{};

    // Game state

    float m_time{0.0f};
    float m_realTime{0.0f};
    ShipState m_ship;
    TerrainData m_terrain;

    // Simulation state (suppresses win/lose message in physics)
    bool m_isSimulation{false};

    enum class Stage
    {
        Running,
        Lost,
        Won
    } m_stage{Stage::Running};
};

struct GraphicsState
{
    sf::RenderWindow m_window;
    sf::Font m_uMono;

    sf::Text m_winMsg;
    sf::Text m_loseMsg;

    struct
    {
        sf::Text time;
        sf::Text alt;
        sf::Text pos;
        sf::Text fuel;
        sf::Text horzSpeed;
        sf::Text vertSpeed;
    } m_textLabels;

    struct
    {
        sf::Text time;
        sf::Text alt;
        sf::Text pos;
        sf::Text fuel;
        sf::Text horzSpeed;
        sf::Text vertSpeed;
    } m_textValues;
};

std::ostream& operator<<(std::ostream& os, sf::Vector2f const& v)
{
    os << "(" << v.x << ", " << v.y << ")";
    return os;
}

void update_GUI_strings(State& world, GraphicsState& state)
{
    auto& values = state.m_textValues;

    int ss = static_cast<int>(world.m_time) % 60;
    int mm = static_cast<int>(world.m_time) / 60;
    values.time.setString(std::format("{:02d}:{:02d}", mm, ss));

    values.alt.setString(std::format("{:04d}", (int)world.m_ship.position.y));
    values.pos.setString(std::format("{:04d}", (int)world.m_ship.position.x));
    values.fuel.setString(std::format("{:04d}", world.m_ship.fuel));
    values.horzSpeed.setString(std::format("{:d}", (int)world.m_ship.velocity.x));
    values.vertSpeed.setString(std::format("{:d}", (int)world.m_ship.velocity.y));
}

void init(State& state, GraphicsState& gfxState, Level gameLevel)
{
    // placement-new to avoid implicitly deleted copy constructor due to sf::RenderWindow
    gfxState.~GraphicsState();
    new (&gfxState) GraphicsState
    {
        .m_window = sf::RenderWindow(sf::VideoMode({SCREEN_WIDTH, SCREEN_HEIGHT}),
            "Mars Lander", sf::Style::Titlebar | sf::Style::Close)
    };

    // Load level data
    levels::Level const& level = levels::get_level_data(gameLevel);
    state.m_ship = level.initState;
    state.m_terrain = level.terrain;

    // Load font
    if (!gfxState.m_uMono.loadFromFile("../resources/UbuntuMono-Regular.ttf"))
    {
        std::cout << "Couldn't find font file!\n";
    }
    // Create text objects
    auto create_text = [&gfxState](const char* str, sf::Vector2f pos)
    {
        sf::Text text;
        text.setFont(gfxState.m_uMono);
        text.setCharacterSize(16);
        text.setFillColor(sf::Color::White);
        text.setStyle(sf::Text::Regular);
        text.setPosition(pos.cwiseMul(sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT)));
        text.setString(str);
        return text;
    };
    // Labels
    gfxState.m_textLabels.time = create_text("TIME", {.2, .05});
    gfxState.m_textLabels.alt = create_text("ALTITUDE", {.2, .1});
    gfxState.m_textLabels.pos = create_text("POSITION", {.2, .15});
    gfxState.m_textLabels.fuel = create_text("FUEL", {.7, .05});
    gfxState.m_textLabels.horzSpeed = create_text("HORIZONTAL SPEED", {.7, .1});
    gfxState.m_textLabels.vertSpeed = create_text("VERTICAL SPEED", {.7, .15});
    // Values
    gfxState.m_textValues.time = create_text("00:00", {.3, .05});
    gfxState.m_textValues.alt = create_text("0000", {.3, .1});
    gfxState.m_textValues.pos = create_text("0000", {.3, .15});
    gfxState.m_textValues.fuel = create_text("0000", {.8, .05});
    gfxState.m_textValues.horzSpeed = create_text("0", {.8, .1});
    gfxState.m_textValues.vertSpeed = create_text("0", {.8, .15});
    // Update strings so that it looks correct on the first frame
    update_GUI_strings(state, gfxState);

    // Win/Loss messages
    gfxState.m_winMsg = create_text("SUCCESS!", {0.5, 0.3});
    gfxState.m_winMsg.setCharacterSize(64);
    sf::FloatRect bounds = gfxState.m_winMsg.getLocalBounds();
    gfxState.m_winMsg.setOrigin(bounds.getSize() / 2.0f);

    gfxState.m_loseMsg = create_text("BOOM", {0.5, 0.3});
    gfxState.m_loseMsg.setCharacterSize(64);
    bounds = gfxState.m_loseMsg.getLocalBounds();
    gfxState.m_loseMsg.setOrigin(bounds.getSize() / 2.0f);
}

struct State2ndOrder
{
    sf::Vector2f first;
    sf::Vector2f second;
};

State2ndOrder operator/(State2ndOrder const& a, float f)
{
    State2ndOrder result;
    result.first = a.first / f;
    result.second = a.second / f;
    return result;
}

State2ndOrder operator*(float f, State2ndOrder const& a)
{
    State2ndOrder result;
    result.first = f * a.first;
    result.second = f * a.second;
    return result;
}

State2ndOrder operator+(State2ndOrder const& a, State2ndOrder const& b)
{
    State2ndOrder result;
    result.first = a.first + b.first;
    result.second = a.second + b.second;
    return result;
}

template <std::floating_point T, typename Y, class FNC_T>
inline Y RK4(FNC_T dy_dt, const T initTime, const Y initVal, const T stepsize)
{
    const T h = stepsize;
    const T t0 = initTime;
    const Y y0 = initVal;
    const T f2 = static_cast<T>(2.0);
    Y k1 = dy_dt(t0, y0);
    Y k2 = dy_dt(t0 + h / f2, y0 + h * (k1 / f2));
    Y k3 = dy_dt(t0 + h / f2, y0 + h * (k2 / f2));
    Y k4 = dy_dt(t0 + h, y0 + h * k3);

    return y0 + static_cast<T>(1.0 / 6.0) * h * (k1 + f2 * k2 + f2 * k3 + k4);
}

// Rotate a point about the origin by an angle, then translate by a vector
sf::Vector2f transform(sf::Vector2f point, float rotate, sf::Vector2f transl)
{
    // Rotate
    sf::Vector2f result;
    result.x = point.x * cos(rotate) - point.y * sin(rotate);
    result.y = point.x * sin(rotate) + point.y * cos(rotate);
    // Translate
    result += transl;

    return result;
}

void physics(State& state, float dt)
{
    state.m_time += dt;
    state.m_ship.time = state.m_time;

    // Rotational authority
    float dAngle = state.m_shipCmd.rotation - state.m_ship.rotation;
    if (fabs(dAngle) > 15.0f)
    {
        state.m_ship.rotation += copysignf(15.0f, dAngle);
    }
    else
    {
        state.m_ship.rotation = state.m_shipCmd.rotation;
    }

    // Thrust authority
    if (state.m_shipCmd.power > state.m_ship.power)
    {
        state.m_ship.power++;
    }
    else if (state.m_shipCmd.power < state.m_ship.power)
    {
        state.m_ship.power--;
    }

    // Fuel burn
    if (state.m_ship.power > state.m_ship.fuel)
    {
        state.m_ship.power = state.m_ship.fuel;
        state.m_ship.fuel = 0;
    }
    else
    {
        state.m_ship.fuel -= state.m_ship.power;
    }

    // Integrate
    float thrust = static_cast<float>(state.m_ship.power);
    float angle = sf::degrees(state.m_ship.rotation).asRadians();
    State2ndOrder shipState{state.m_ship.position, state.m_ship.velocity};
    auto phys = [thrust, angle](float t, State2ndOrder const& state)
    {
        sf::Vector2f vel = state.second;
        sf::Vector2f thrustA(thrust * sin(-angle), thrust * cos(-angle));
        sf::Vector2f accel = sf::Vector2f(0.0f, -3.711f) + thrustA;

        return State2ndOrder{vel, accel};
    };
    shipState = RK4(phys, state.m_time, shipState, dt);


    // Collision

    sf::Vector2f lastPos = state.m_ship.position;
    sf::Vector2f lastVel = state.m_ship.velocity;
    sf::Vector2f nextPos = shipState.first;
    sf::Vector2f nextVel = shipState.second;

    // Magic wikipedia formula for line intersections
    // https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection
    // "Given two points on each line"

    auto intersect = [](sf::Vector2f a1, sf::Vector2f a2, sf::Vector2f b1, sf::Vector2f b2)
        -> std::optional<float>
    {
        float x1 = a1.x;
        float x2 = a2.x;
        float x3 = b1.x;
        float x4 = b2.x;

        float y1 = a1.y;
        float y2 = a2.y;
        float y3 = b1.y;
        float y4 = b2.y;

        float t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4))
            / ((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4));
        float s = ((x1 - x3) * (y1 - y2) - (y1 - y3) * (x1 - x2))
            / ((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4));

        if (t > 0.0f && t < 1.0f && s > 0.0f && s < 1.0f)
        {
            return t;
        }
        return {};
    };
    auto interpolate = [](float t, auto a1, auto a2)
    {
        auto p = a1 + t * (a2 - a1);
        return p;
    };

    bool collision = false;
    bool hitFatalPart = false;
    bool validLandingSpot = true;
    sf::Vector2f collisionPos;
    sf::Vector2f shipColPos;
    sf::Vector2f shipColVel;
    float hitFactor = 1.0f;
    for (size_t i = 1; i < state.m_terrain.m_points.size(); i++)
    {
        // Grab a terrain segment
        sf::Vector2f a = state.m_terrain.m_points[i - 1];
        sf::Vector2f b = state.m_terrain.m_points[i];

        // Calculate slope (used to determine valid landing spot)
        sf::Vector2f n = sf::Vector2f(-(b - a).y, (b - a).x).normalized();
        sf::Angle slope = sf::radians(atan2(n.x, n.y));

        // Test all colliders
        for (size_t j = 0; j < g_shipColliders.size(); j++)
        {
            sf::Vector2f offset = g_shipColliders[j];
            sf::Vector2f p0 = transform(offset, angle, lastPos);
            sf::Vector2f p1 = transform(offset, angle, nextPos);

            if constexpr (g_DEBUG_GRAPHICS) { draw_line(p0, p1, sf::Color::Red); }

            // Calculate intersection
            auto factor = intersect(p0, p1, a, b);
            if (!factor.has_value()) { continue; }

            // Collision found
            collision = true;

            // Check whether terrain is suitable for landing via slope
            // Slope must be <5 deg
            if (fabs(slope.asDegrees()) > 5.0f) { validLandingSpot = false; }

            // Skip if other parts collided sooner
            if (*factor > hitFactor) { continue; }
            hitFactor = *factor;

            //std::cout << "Hit: " << j << "\n";

            // Collision happens sooner than previous ones, save it
            collisionPos = interpolate(*factor, p0, p1);

            // Compute final ship params if ship stops due to this collision
            shipColPos = interpolate(*factor, lastPos, nextPos);
            shipColVel = interpolate(*factor, lastVel, nextVel);

            // Set flag if nearest collision is with a fatal part
            // Colliders 0 and 1 are the feet, rest are fatal
            if (j > 1) { hitFatalPart = true; }
        }
    }

    // If the state doesn't have an associated window, we're running in a simulation
    bool isSimulation = state.m_isSimulation;

    if (collision)
    {
        // Debug message
        if (!isSimulation)
        {
            std::cout << "Collision:"
                << "\npos: " << collisionPos
                << "\nvel: " << shipColVel
                << "\nlast pos: " << lastPos
                << "\nlast vel: " << lastVel
                << "\nnext pos: " << nextPos
                << "\nnext vel: " << nextVel
                << "\n";
        }

        state.m_ship.position = shipColPos;
        state.m_ship.velocity = shipColVel;

        state.m_ship.power = 0;

        // Calculate win/lose conditions

        if (hitFatalPart)
        {
            if (!isSimulation)
            {
                std::cout << "Error: fatal component hit terrain (try landing on your feet)\n";
            }
            goto lose;
        }
        if (!validLandingSpot)
        {
            if (!isSimulation)
            {
                std::cout << "Error: terrain not flat enough\n";
            }
            goto lose;
        }
        if (shipColVel.length() > 40.0f)
        {
            if (!isSimulation)
            {
                std::cout << "Error: excessive velocity\n";
            }
            goto lose;
        }
        if (fabs(state.m_ship.rotation) > 0.1f)
        {
            if (!isSimulation)
            {
                std::cout << "Error: ship not upright\n";
            }
            goto lose;
        }

        // Win
        if (!isSimulation) { std::cout << "You win!\n"; }
        state.m_stage = State::Stage::Won;
        return;

        // Lose
    lose:
        if (!isSimulation) { std::cout << "You lose!\n"; }
        state.m_stage = State::Stage::Lost;
        return;
    }

    state.m_ship.position = shipState.first;
    state.m_ship.velocity = shipState.second;
}

sf::Vector2f projection(sf::Vector2f const& v)
{
    constexpr float scaleX = (float)SCREEN_WIDTH / WORLD_WIDTH;
    constexpr float scaleY = (float)SCREEN_HEIGHT / WORLD_HEIGHT;
    sf::Vector2f result(scaleX * v.x, SCREEN_HEIGHT - scaleY * v.y);
    return result;
}

void draw_scene(sf::RenderWindow& win, State const& state, GraphicsState const& gfxState)
{
    win.clear();

    // Draw terrain
    auto& terrain = state.m_terrain.m_points;
    std::vector<sf::Vertex> terrainTransformed(terrain.size());
    for (size_t i = 0; i < terrain.size(); i++)
    {
        sf::Vertex& v = terrainTransformed[i];
        v.position = projection(terrain[i]);
        v.color = sf::Color(255, 0, 0);
    }
    win.draw(terrainTransformed.data(), terrainTransformed.size(), sf::LineStrip);

    // Draw ship
    sf::Vector2f shipPos = projection(state.m_ship.position);
    sf::Transform shipTF;
    shipTF.translate(shipPos);
    shipTF.rotate(sf::degrees(180.0f - state.m_ship.rotation));
    win.draw(g_shipVerts.data(), g_shipVerts.size(),
        sf::PrimitiveType::Lines, sf::RenderStates(shipTF));

    // Draw rocket exhaust
    std::array<sf::Vertex, 4> jetMesh
    {
        sf::Vertex{sf::Vector2f(-4, -12), sf::Color::Red},
        sf::Vertex{sf::Vector2f(0, -30), sf::Color::Red},
        sf::Vertex{sf::Vector2f(0, -30), sf::Color::Red},
        sf::Vertex{sf::Vector2f(4, -12), sf::Color::Red}
    };
    if (state.m_ship.power > 0)
    {
        float tip = -20 - 10 * state.m_ship.power;
        float t = 10.0f * state.m_realTime;
        tip += 3.0f * sin(t) * sin(4.0f * t);
        jetMesh[1].position.y = tip;
        jetMesh[2].position.y = tip;
        float wag = 0.5f * sin(t - 1.0f) * sin(5.0f * t);
        jetMesh[1].position.x = wag;
        jetMesh[2].position.x = wag;

        win.draw(jetMesh.data(), jetMesh.size(),
            sf::PrimitiveType::Lines, sf::RenderStates(shipTF));
    }

    // Draw debug graphics
    win.draw(g_debugLines.data(), g_debugLines.size(), sf::PrimitiveType::Lines);
    for (auto& point : g_debugPoints)
    {
        win.draw(point);
    }

    // Draw GUI
    auto& txtLabels = gfxState.m_textLabels;
    auto& txtValues = gfxState.m_textValues;
    win.draw(txtLabels.time);
    win.draw(txtLabels.alt);
    win.draw(txtLabels.pos);
    win.draw(txtLabels.fuel);
    win.draw(txtLabels.horzSpeed);
    win.draw(txtLabels.vertSpeed);

    win.draw(txtValues.time);
    win.draw(txtValues.alt);
    win.draw(txtValues.pos);
    win.draw(txtValues.fuel);
    win.draw(txtValues.horzSpeed);
    win.draw(txtValues.vertSpeed);

    // Display win/loss message, if applicable
    switch (state.m_stage)
    {
    case State::Stage::Won:
        win.draw(gfxState.m_winMsg);
        break;
    case State::Stage::Lost:
        win.draw(gfxState.m_loseMsg);
        break;
    case State::Stage::Running:
    default:
        break;
    }

    win.display();
}

void run(State& state, GraphicsState& gfxState, GameSpeed speed)
{
    //double t = 0.0;
    sf::Clock clock;
    sf::Time accumulator = sf::milliseconds(0);

    // Time acceleration
    int accelFactor = 1;
    switch (speed)
    {
    case GameSpeed::x2:
        accelFactor = 2;
        break;
    case GameSpeed::x4:
        accelFactor = 4;
        break;
    case GameSpeed::x8:
        accelFactor = 8;
        break;
    case GameSpeed::x16:
        accelFactor = 16;
        break;
    case GameSpeed::x1:
    default:
        break;
    }
    int stepTime = 1000 / accelFactor;

    while (gfxState.m_window.isOpen())
    {
        sf::Event event;
        while (gfxState.m_window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                gfxState.m_window.close();
            }
        }

        sf::Time elapsed;
        if constexpr (false) // fixed timestep
        {
            elapsed = sf::milliseconds(1);
        }
        else
        {
            elapsed = clock.restart();
        }
        accumulator += elapsed;
        state.m_realTime += elapsed.asSeconds();

        if (accumulator.asMilliseconds() > stepTime && state.m_stage == State::Stage::Running)
        {
            // Clear debug graphics for fresh drawing by physics/user
            g_debugLines.clear();
            g_debugPoints.clear();

            physics(state, 1.0f);

            // Get and sanitize player inputs
            ShipCommand cmd = state.m_update(state.m_ship);
            if (cmd.power > 4) { cmd.power = 4; }
            if (cmd.power < 0) { cmd.power = 0; }
            if (cmd.rotation > 90.0f) { cmd.rotation = 90.0f; }
            if (cmd.rotation < -90.0f) { cmd.rotation = -90.0f; }
            state.m_shipCmd = cmd;

            accumulator -= sf::milliseconds(stepTime);
        }

        update_GUI_strings(state, gfxState);
        draw_scene(gfxState.m_window, state, gfxState);
    }
}

void draw_line(sf::Vector2f a, sf::Vector2f b, sf::Color color)
{
    g_debugLines.push_back(sf::Vertex(projection(a), color));
    g_debugLines.push_back(sf::Vertex(projection(b), color));
}

void draw_point(sf::Vector2f pos, float radius, sf::Color color)
{
    sf::CircleShape circle(radius * ((float)SCREEN_HEIGHT / WORLD_HEIGHT));
    circle.setFillColor(color);
    circle.setPosition(projection(pos));
    circle.setOrigin(circle.getLocalBounds().getSize() / 2.0f);
    g_debugPoints.push_back(circle);
}

TerrainData const& get_terrain_data(Level gameLevel)
{
    levels::Level const& level = levels::get_level_data(gameLevel);
    return level.terrain;
}

void play(control_t fnc, Level gameLevel, GameSpeed speed)
{
    State state;
    GraphicsState gfxState;
    init(state, gfxState, gameLevel);

    state.m_update = fnc;

    run(state, gfxState, speed);
}

std::vector<ShipState> run_simulation(control_t fnc, Level gameLevel)
{
    State state;
    state.m_isSimulation = true;
    levels::Level const& level = levels::get_level_data(gameLevel);
    state.m_ship = level.initState;
    state.m_terrain = level.terrain;
    state.m_update = fnc;

    std::vector<ShipState> traj;
    traj.reserve(64);

    traj.push_back(state.m_ship);

    while (state.m_stage == State::Stage::Running)
    {
        // Clear debug graphics just to make sure they don't overflow since we
        // aren't going to use them here
        g_debugLines.clear();
        g_debugPoints.clear();

        physics(state, 1.0f);

        traj.push_back(state.m_ship);

        // Get and sanitize player inputs
        ShipCommand cmd = state.m_update(state.m_ship);
        if (cmd.power > 4) { cmd.power = 4; }
        if (cmd.power < 0) { cmd.power = 0; }
        if (cmd.rotation > 90.0f) { cmd.rotation = 90.0f; }
        if (cmd.rotation < -90.0f) { cmd.rotation = -90.0f; }
        state.m_shipCmd = cmd;
    }

    return traj;
}

