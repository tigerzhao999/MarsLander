#include "game.h"
#include <array>
#include <valarray>
#include <iostream>
#include <format>
#include <optional>
#include "ga.h"
#include "time.h"
#include <random>


int numberofpeople = 40;
int numberofgenes = 90;
static int step = 0;
Individual* currIndividual;
Population totalPopulation = pop();

Individual iri()
{
    Individual initialrandomindividual;
    //int finalNum = rand() % (max - min + 1) + min; // Generate the number, assign to variable.
    
    srand(time(NULL));
    for (int i = 0; i < numberofgenes; i++)
    {
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist4(0, 4); // distribution in range [1, 6]
        std::uniform_int_distribution<std::mt19937::result_type> distrot(0, 90);
        
        ShipCommand initial;
        //int randNum = rand()%(max-min + 1) + min;
        initial.power = dist4(rng);
        initial.rotation = distrot(rng) - 45;
        initialrandomindividual.gene.push_back(initial);
    }
    return initialrandomindividual;
}


Population pop()
{
    Population population;
    for (int i = 0; i < numberofpeople; i++)
    {
        Individual individual = iri();
        population.wholePopulation.push_back(individual);
    }
    return population;
}

void selection()
{
    std::sort(totalPopulation.wholePopulation.begin(), totalPopulation.wholePopulation.end(), 
        [](Individual& a, Individual& b) { return a.fittness > b.fittness; });
}



ShipCommand gaCMD(ShipState const& state)
{

    if (step < currIndividual->gene.size())
    {
        return currIndividual->gene.at(step++);
    }
    else
    {
        std::cout << "steps outside the gene on : ";
        std::cout << step;
        exit(EXIT_FAILURE);
    }   
}

void runGA(Level gameLevel, TerrainData const* g_terrain)
{
    for (int i = 0; i < numberofpeople; i++)
    {
        currIndividual = &totalPopulation.wholePopulation.at(i);
        std::vector<ShipState> sim_result = run_simulation(&gaCMD, gameLevel);
        ShipState end = sim_result.at(sim_result.size() - 1);
        std::cout << end.position.x;
        std::cout << "\n \n";
        //std::cout << end.position.y;
        std::cout << "*flight time " << sim_result.size() << std::endl;
        step = 0;
        fittness(end, currIndividual, g_terrain);
    }
    selection();
}

void fittness(ShipState const& ship, Individual* currIndividual, TerrainData const* g_terrain)
{
    //domath
    float returnfittness = 0;

    //float rotate = ship.rotation;
    std::cout << "fittness calc\n";
    //currIndividual->fittness = rotate;
    //std::cout << currIndividual->fittness;

    // find find flat___________________________________________
    float previous = g_terrain->m_points[0].y;
    sf::Vector2f flatvector = g_terrain->m_points[0];
    sf::Vector2f leftflat = g_terrain->m_points[0];
    sf::Vector2f rightflat = g_terrain->m_points[0];
    float middle = 0.69f;
    float middleheight = 0.69f;
    for (int i = 1; i < g_terrain->m_points.size(); i++)
    {
        if (flatvector.y == g_terrain->m_points[i].y)
        {
            middle = (flatvector.x + g_terrain->m_points[i].x) / 2;
            middleheight = flatvector.y;
            leftflat = flatvector;
            rightflat = g_terrain->m_points[i];
            std::cout << "* Debug " << middle << std::endl;
            float leftx = leftflat.x;
            float rightx = rightflat.x;
            std::cout << "* Debug left " << leftx << std::endl;
            std::cout << "* Debug " << rightx << std::endl;
        }
        else
        {
            previous = g_terrain->m_points[i].y;
            flatvector = g_terrain->m_points[i];
        }
    }

    if (ship.position.x >= leftflat.x && ship.position.x <= rightflat.x)
    {
        returnfittness = returnfittness + 10000; 
    }
    if (ship.position.x <= leftflat.x)
    {
        returnfittness = middle - (leftflat.x - ship.position.x);
    }
    if (ship.position.x >= rightflat.x)
    {
        returnfittness = middle - (ship.position.x - rightflat.x);
    }
    else
    {
        returnfittness = returnfittness;
    }

    float angle = ship.rotation;
    if (angle > 20)
    {
        returnfittness = returnfittness * 0.33;
    }
    else if (angle > 0)
    {
        returnfittness = returnfittness * 0.66;
    }

    if (abs(ship.velocity.x) > 15)
    {
        returnfittness = returnfittness * 0.75;
    }
    if (abs(ship.velocity.y) > 40)
    {
        returnfittness = returnfittness * 0.75;
    }
    std::cout << "* speed x " << ship.velocity.x << std::endl;
    std::cout << "* speed y " << ship.velocity.y << std::endl;
    std::cout << "* angle " << angle << std::endl;
    std::cout << "* returnfittness " << returnfittness << std::endl;
    currIndividual->fittness = returnfittness;
}


