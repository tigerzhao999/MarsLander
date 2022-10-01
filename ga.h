#pragma once
#include <vector>
#include "game.h"

struct Individual
{
    std::vector<ShipCommand> gene;
    int fittness = 0;
};

struct Population
{
    std::vector<Individual> wholePopulation;
};

Individual iri();

Population pop();

//void fittness(ShipState const& ship, Individual& currIndividual);

void fittness(ShipState const& ship, Individual* currIndividual, TerrainData const* g_terrain);

void runGA(Level gameLevel, TerrainData const* terrain);
