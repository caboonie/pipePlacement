#pragma once
#include "Piping.h"
#include <vector>

using namespace std;

vector<pipe> makePipesGroup(vector<Component > components, vector<HeaderLoop> headers, double maxLoad, int maxEvals = 10, int maxIters = 10, double stayProb = .9);
vector<pipe> makePipesForGroup(vector<Component> components, vector<HeaderLoop> headers);
vector<pipe> makePipesForGroupBend(vector<Component> components, vector<HeaderLoop> headers, bool verbose = false);
vector<vector<Component>> makeGroupsGroup(vector<Component > components, vector<HeaderLoop> headers, double maxLoad, int maxEvals = 10, int maxIters = 10, double stayProb = .9);