#include<stdio.h> 
#include <iostream>
#include <vector>
#include <tuple>
#include <algorithm>
#include <map>
#include <ctime>
#include <chrono>
#include <unordered_map> 
#include "Piping.h"
#include "csvParse.h"

using namespace std;

double timeMakingPipes = 0;
double timeEvaluatingWeight = 0;
double timeGettingWeight = 0;
double timeStochasticStep = 0;
clock_t c_start = clock();
unordered_map <vector<Component>, double> groupToCost;

void printGroup(vector<Component>& group) {
	for (Component component : group) {
		cout << " ";
		component.print();
		cout << "\n";
	}
}

void printGroups(vector<vector<Component>>& groups) {
	for (int i = 0; i < groups.size(); i++) {
		cout << "Group " << i << ":\n";
		printGroup(groups[i]);
	}
}


bool compareComponentX(Component i1, Component i2)
{
	return (i1.getX() < i2.getX());
}
bool compareComponentY(Component i1, Component i2)
{
	return (i1.getY() < i2.getY());
}
bool compareComponentZ(Component i1, Component i2)
{
	return (i1.getZ() < i2.getZ());
}

int weighted_median(vector<Component> components) { //could do this in O(n) with unsorted as well... j more work.
	double sum = 0;
	for (Component component : components) {
		sum += weightPerMeter(kWattLoadToDiameter(component.getFlow()));
	}
	double cumsum = 0;
	for (int i = 0; i < components.size(); i++) {
		cumsum += weightPerMeter(kWattLoadToDiameter(components[i].getFlow()));
		if (cumsum > sum / 2) {
			return i;
		}
	}
}

bool groupsUnderLoad(vector<vector<Component>> groups, double maxLoad) {

	for (vector<Component> components : groups) {
		double load = 0;
		for (Component component : components) {
			load += component.getFlow();
		}
		if (load > maxLoad) {
			return false;
		}

	}
	return true;
}

vector<pipe> makePipesForGroup(vector<Component> components, vector<Header> headers) {
	//find the median of the group along x-axis - this is where the branch will connect to the group.
	vector<pipe> pipes;
	sort(components.begin(), components.end(), compareComponentZ);
	int median = weighted_median(components);
	double medianZ = components[median].getZ();
	sort(components.begin(), components.end(), compareComponentX);
	median = weighted_median(components);
	double medianX = components[median].getX();
	bool hasVital = false;
	double load = 0;
	for (Component component : components) {
		load += component.getFlow();
	}


	sort(components.begin(), components.end(), compareComponentY);
	double lastY = components[0].getY();
	for (Component component : components) {
		if (component.isVital()) {
			hasVital = true;
		}

		pipes.push_back(pipe(dTriple(medianX, component.getY(), medianZ), dTriple(medianX, component.getY(), component.getZ()), kWattLoadToDiameter(component.getFlow()), component.getFlow()));
		pipes.push_back(pipe(dTriple(medianX, component.getY(), component.getZ()), dTriple(component.getX(), component.getY(), component.getZ()), kWattLoadToDiameter(component.getFlow()), component.getFlow()));

		if (component.getY() != lastY) {
			pipes.push_back(pipe(dTriple(medianX, lastY,medianZ), dTriple(medianX, component.getY(), medianZ), kWattLoadToDiameter(load), load));
		}
		lastY = component.getY();
	}

	double minY = components[0].getY();
	double maxY = components[components.size() - 1].getY();

	double shortToAnyHeader = INFINITY;
	pipe shortPipeToAnyY;
	pipe shortPipeToAnyZ;
	//figure out what pipe to use to connect the branch to the header.
	//for the length of the branch figure out the part closest to any header - for two lists of pipes, find the points closest
	for (Header header : headers) {
		double shortToHeader = INFINITY;
		pipe shortPipeToHeaderY;
		pipe shortPipeToHeaderZ;
		

		if (header.Y >= minY && header.Y <= maxY) {
			shortPipeToHeaderY = pipe(dTriple(0, 0, 0), dTriple(0, 0, 0), 0, 0);
			shortPipeToHeaderZ = pipe(dTriple(medianX, header.Y, medianZ), dTriple(medianX, header.Y, header.Z), headerDiameter, headerLoad);
			shortToHeader = 0;
		}
		for (double Y : {minY, maxY}) {
			if (abs(header.Y - Y) < shortToHeader) {
				shortPipeToHeaderY = pipe(dTriple(medianX, header.Y, header.Z), dTriple(medianX, header.Y, medianZ), kWattLoadToDiameter(load), load);
				shortPipeToHeaderZ = pipe(dTriple(medianX, header.Y, medianZ), dTriple(medianX, Y, medianZ), kWattLoadToDiameter(load), load);
				shortToHeader = abs(header.Y - Y);
			}
		}
		if (hasVital) {
			pipes.push_back(shortPipeToHeaderY);
			pipes.push_back(shortPipeToHeaderZ);
		}
		if (shortToHeader < shortToAnyHeader) {
			shortToAnyHeader = shortToHeader;
			shortPipeToAnyY = shortPipeToHeaderY;
			shortPipeToAnyZ = shortPipeToHeaderZ;
		}
	}
	if (!hasVital) {
		pipes.push_back(shortPipeToAnyY);
		pipes.push_back(shortPipeToAnyZ);
	}


	vector<pipe> validPipes;
	for (pipe elt : pipes) {
		if (pipeLength(elt) > .02) {
			validPipes.push_back(elt);
		}
	}
	return validPipes;
}

vector<pipe> makePipesBendHelper(vector<Component> components, double load, bool verbose = false) {
	//find the median of the group along x-axis - this is where the branch will connect to the group.
	vector<pipe> pipes;
	sort(components.begin(), components.end(), compareComponentZ);
	int median = weighted_median(components);
	double medianZ = components[median].getZ();
	sort(components.begin(), components.end(), compareComponentX);
	//cout << "here7\n";
	median = weighted_median(components);
	//cout << "here8 median: "<<median<<" "<<components.size()<<"\n";
	double medianX = components[median].getX();
	//cout << "here9\n";
	if (verbose) {
		cout << "median: " << medianX << "\n";
	}


	sort(components.begin(), components.end(), compareComponentY);
	double lastY = components[0].getY();
	//cout << "here6\n";
	for (Component component : components) {

		pipes.push_back(pipe(dTriple(medianX, component.getY(), medianZ), dTriple(medianX, component.getY(), component.getZ()), kWattLoadToDiameter(component.getFlow()), component.getFlow()));
		pipes.push_back(pipe(dTriple(medianX, component.getY(), component.getZ()), dTriple(component.getX(), component.getY(), component.getZ()), kWattLoadToDiameter(component.getFlow()), component.getFlow()));
		if (component.getY() != lastY) {
			pipes.push_back(pipe(dTriple(medianX, lastY, medianZ), dTriple(medianX, component.getY(), medianZ), kWattLoadToDiameter(load), load));
		}
		lastY = component.getY();
	}



	vector<pipe> validPipes;
	for (pipe elt : pipes) {
		if (pipeLength(elt) > .02) {
			validPipes.push_back(elt);
		}
	}
	return validPipes;
}


vector<pipe> makePipesForGroupBend(vector<Component> components, vector<Header> headers, bool verbose = false) {
	//find a reasonable point to bend the branch pipe
	if (components.size() == 1) { return makePipesForGroup(components, headers); }

	vector<pipe> pipes;
	sort(components.begin(), components.end(), compareComponentY);
	bool hasVital = false;
	double minY = components[0].getY();
	double maxY = components[components.size() - 1].getY();
	double sum = 0;
	double flowSum = 0;
	for (Component component : components) {
		sum += component.getFlow() * component.getX();
		flowSum += component.getFlow();
	}
	double avgXTop, avgXBottom, difference;
	double bottomSum = 0;
	double bottomFlow = 0;
	double maxDifference = 0;
	int bendInd = 0;
	for (int i = 0; i < components.size() - 1; i++) {
		if (components[i].isVital()) {
			hasVital = true;
		}
		bottomSum += components[i].getFlow() * components[i].getX();
		bottomFlow += components[i].getFlow();
		avgXTop = (sum - bottomSum) / (flowSum - bottomFlow);
		avgXBottom = bottomSum / bottomFlow;
		difference = abs(avgXTop - avgXBottom);
		if (difference > maxDifference) {
			maxDifference = difference;
			bendInd = i;
		}
	}
	//cout << "bendInd: " << bendInd<<"\n";

	double load = 0;
	for (Component component : components) {
		load += component.getFlow();
	}
	vector<Component> first(components.begin(), components.begin() + bendInd + 1);
	//cout << "here5\n";
	vector<pipe> subPipes = makePipesBendHelper(first, load, verbose);

	for (int i = 0; i < subPipes.size(); i++) pipes.push_back(subPipes[i]);
	sort(first.begin(), first.end(), compareComponentX);
	int median = weighted_median(first);
	double firstMedianX = first[median].getX();
	sort(first.begin(), first.end(), compareComponentZ);
	median = weighted_median(first);
	double firstMedianZ = first[median].getZ();
	//cout << "here3\n";
	vector<Component> last(components.begin() + bendInd + 1, components.end());

	subPipes = makePipesBendHelper(last, load, verbose);

	for (int i = 0; i < subPipes.size(); i++) pipes.push_back(subPipes[i]);

	sort(last.begin(), last.end(), compareComponentX);
	median = weighted_median(last);
	double lastMedianX = last[median].getX();
	sort(last.begin(), last.end(), compareComponentZ);
	median = weighted_median(last);
	double lastMedianZ = last[median].getZ();
	if (verbose) {
		cout << "medians: " << firstMedianX << " " << lastMedianX << "\n";
	}
	pipes.push_back(pipe(dTriple(firstMedianX, components[bendInd].getY(), firstMedianZ), dTriple(firstMedianX, components[bendInd].getY(), lastMedianZ), kWattLoadToDiameter(load), load));
	pipes.push_back(pipe(dTriple(firstMedianX, components[bendInd].getY(), lastMedianZ), dTriple(lastMedianX, components[bendInd].getY(), lastMedianZ), kWattLoadToDiameter(load), load));
	pipes.push_back(pipe(dTriple(lastMedianX, components[bendInd].getY(), lastMedianZ), dTriple(lastMedianX, components[bendInd + 1].getY(), lastMedianZ), kWattLoadToDiameter(load), load));



	double shortToAnyHeader = INFINITY;
	pipe shortPipeToAnyY;
	pipe shortPipeToAnyZ;
	for (Header header : headers) {

		double shortToHeader = INFINITY;
		pipe shortPipeToHeaderY;
		pipe shortPipeToHeaderZ;

		if (header.Y >= minY && header.Y <= maxY) {
			shortPipeToHeaderY = pipe(dTriple(0, 0, 0), dTriple(0,0,0), 0, 0);
			shortPipeToHeaderZ = pipe(dTriple(0, 0, 0), dTriple(0, 0, 0), 0, 0);
			shortToHeader = 0;
		}

		if (abs(header.Y - minY) < shortToHeader) {
			shortPipeToHeaderY = pipe(dTriple(firstMedianX, header.Y, header.Z), dTriple(firstMedianX, header.Y, firstMedianZ), kWattLoadToDiameter(load), load);
			shortPipeToHeaderZ = pipe(dTriple(firstMedianX, header.Y, firstMedianZ), dTriple(firstMedianX, minY, firstMedianZ), kWattLoadToDiameter(load), load);
			shortToHeader = abs(header.Y - minY);
		}
		if (abs(header.Y - maxY) < shortToHeader) {
			shortPipeToHeaderY = pipe(dTriple(lastMedianX, header.Y, header.Z), dTriple(lastMedianX, header.Y, lastMedianZ), kWattLoadToDiameter(load), load);
			shortPipeToHeaderZ = pipe(dTriple(lastMedianX, header.Y, lastMedianZ), dTriple(lastMedianX, maxY, lastMedianZ), kWattLoadToDiameter(load), load);
			shortToHeader = abs(header.Y - maxY);
		}
		if (hasVital) {
			pipes.push_back(shortPipeToHeaderY);
			pipes.push_back(shortPipeToHeaderZ);
		}
		if (shortToHeader < shortToAnyHeader) {
			shortToAnyHeader = shortToHeader;
			shortPipeToAnyY = shortPipeToHeaderY;
			shortPipeToAnyZ = shortPipeToHeaderZ;
		}
	}
	if (!hasVital) {
		pipes.push_back(shortPipeToAnyY);
		pipes.push_back(shortPipeToAnyZ);
	}


	vector<pipe> validPipes;
	for (pipe elt : pipes) {
		if (pipeLength(elt) > .02) {
			validPipes.push_back(elt);
		}
	}
	return validPipes;
}


vector<double> groupMedians(vector<vector<Component>>& groups) {
	vector<double> medians;
	for (vector < Component > components : groups) {
		sort(components.begin(), components.end(), compareComponentX);
		int median = weighted_median(components);
		medians.push_back(median);
	}

	return medians;
}

int closestGroup(vector<double> medians, Component component) {
	double bestDist = INFINITY;
	int bestMedian = -1;
	for (int i = 0; i < medians.size(); i++) {
		//cout << abs(medians[i] - component.getX()) << "\n";
		if (abs(medians[i] - component.getX()) < bestDist) {
			bestDist = abs(medians[i] - component.getX());
			bestMedian = i;
		}
	}
	return bestMedian;
}


vector<vector<Component>> stochasticStep(vector<vector<Component>>& groups, double stayProb = .9) {
	clock_t c_start = clock();
	vector<vector<Component>> newGroups;
	//double stayProb = 9; //consider a more complicated function/momentum

	vector<double> medians;
	for (vector < Component > components : groups) {
		sort(components.begin(), components.end(), compareComponentX);
		int median = weighted_median(components);
		medians.push_back(median);
	}
	//cout << "medians size: " << medians.size() << "\n";

	for (int i = 0; i < groups.size() + 1; i++) {
		newGroups.push_back(vector<Component>{});
	}

	for (int i = 0; i < groups.size(); i++) {
		for (Component component : groups[i]) {
			if ((rand() % 1000) / 1000.0 < stayProb) {
				newGroups[i].push_back(component);
			}
			else {
				int newGroup = rand() % (newGroups.size());
				newGroups[newGroup].push_back(component);
			}
		}
	}
	//cout << "here0\n";
	for (int i = newGroups.size() - 1; i >= 0; i--) {
		//cout << "int i: " << i << "\n";
		if (newGroups[i].size() == 0) {
			newGroups.erase(newGroups.begin() + i);
		}
	}
	//timeStochasticStep += (clock() - c_start);
	return newGroups;
}



double groupWeight(vector<vector<Component>>& groups, vector<Header>& headers) { //consider memoizing for speed?
	Header header1 = headers[0]; Header header2 = headers[1];
	double cost = 0;
	//cout << "start groupWeight\n";
	for (vector<Component> group : groups) {
		if (group.size() == 0) {
			//do nothing
		}
		else if (groupToCost.find(group) != groupToCost.end()) {
			//cout << "hit"<<hash<vector<Component>>()(group)<<"\n";
			//printGroup(group);
			cost += groupToCost.at(group);
		}
		else {
			//cout << "miss\n";
			//cout << hash<vector<Component>>()(group) << "\n";
			//printGroup(group);
			c_start = clock();
			//cout << "here8\n";
			vector<pipe> pipes = makePipesForGroup(group, headers);
			//cout << "here9\n";
			vector<pipe> pipesBend = makePipesForGroupBend(group, headers);
			//timeMakingPipes += (clock() - c_start);
			c_start = clock();
			//cout << "here5\n";
			double groupCostNoBend = pipeCost(pipes, headers);
			//cout << "here6\n";
			double groupCostBend = pipeCost(pipesBend, headers);
			//cout << "here7\n";
			double groupCost = min(groupCostBend, groupCostNoBend);
			//timeEvaluatingWeight += (clock() - c_start);
			groupToCost.insert(pair< vector<Component>, double>(group, groupCost));
			cost += groupCost;
		}
	}

	vector<vector<Component>> colocatedGroups;
	return cost + colocationPentalty(groups, colocatedGroups);
}



vector<vector<Component>> randomInit(vector<Component> components) {
	vector<vector<Component>> newGroups;
	for (int i = 0; i < components.size() + 1; i++) {
		newGroups.push_back(vector<Component>{});
	}
	for (Component component : components) {
		int newGroup = rand() % (newGroups.size());
		newGroups[newGroup].push_back(component);
	}
	for (int i = newGroups.size() - 1; i >= 0; i--) {
		//cout << "int i: " << i << "\n";
		if (newGroups[i].size() == 0) {
			newGroups.erase(newGroups.begin() + i);
		}
	}
	return newGroups;
}

vector<vector<Component>> medianFix(vector<vector<Component>>& groups, vector<Component> components, vector<Header> headers) {
	vector<double> medians;
	vector<pair<double, double>> pipeEnds;
	vector<vector<Component>> newGroups;
	for (vector<Component> group : groups) {
		newGroups.push_back(vector<Component>{});
		sort(group.begin(), group.end(), compareComponentX);
		int medianInd = weighted_median(group);
		double medianX = group[medianInd].getX();
		medians.push_back(medianX);
		sort(group.begin(), group.end(), compareComponentY);

		bool hasVital = false;
		for (Component component : components) {
			if (component.isVital()) {
				hasVital = true;
			}
		}


		double minY = group[0].getY();
		double maxY = group[group.size() - 1].getY();

		double shortToAnyHeader = INFINITY;
		Header headerUsed;
		for (Header header : headers) {
			for (double Y : {minY, maxY}) {
				if (abs(header.Y - Y) < shortToAnyHeader) {
					headerUsed = header;
					shortToAnyHeader = abs(header.Y - Y);
				}
			}
		}

		vector<double> possibleEnds = { headerUsed.Y,minY,maxY };
		if (hasVital) {
			possibleEnds = { headers[0].Y,headers[1].Y,minY,maxY };
		}
		sort(possibleEnds.begin(), possibleEnds.end());
		pipeEnds.push_back(pair<double, double>(possibleEnds[0], possibleEnds[possibleEnds.size() - 1]));

	}

	//cout << "finshed medians\n";
	for (Component component : components) {
		double minDist = INFINITY;
		int bestInd = -1;
		//component.print();
		//cout << "\n";
		for (int i = 0; i < medians.size(); i++) {
			if (abs(component.getX() - medians[i]) < minDist) {
				//cout << i << " "<< abs(component.getX() - medians[i])<<" "<< pipeEnds[i].first <<" "<< pipeEnds[i].second<<"\n";
				if (component.getY() + .5 >= pipeEnds[i].first && component.getY() - .5 <= pipeEnds[i].second) { //component is in pipe's Y range
					minDist = abs(component.getX() - medians[i]);
					bestInd = i;
				}
			}
		}

		//cout << " has bestInd: " << bestInd << "\n";
		newGroups[bestInd].push_back(component);
		//cout << "here\n";
		//pipeEnds[bestInd] = pair<double, double>(min(pipeEnds[bestInd].first, component.getY()), max(pipeEnds[bestInd].second, component.getY()));
	}
	for (int i = newGroups.size() - 1; i >= 0; i--) {
		if (newGroups[i].size() == 0) {
			newGroups.erase(newGroups.begin() + i);
		}
	}
	//cout << "finished\n";
	return newGroups;
}

vector<vector<Component>> medianConsolidate(vector<vector<Component>>& groups) {
	//cout << "consolidating medians\n";
	vector<double> medians;
	vector<pair<double, double>> pipeEnds;
	vector<vector<Component>> newGroups;
	double closestMedianDist;
	int closestMedian;
	for (int j = 0; j < groups.size(); j++) {
		//cout << "j: " << j << "\n";
		newGroups.push_back(vector<Component>{});
		sort(groups[j].begin(), groups[j].end(), compareComponentX);
		int medianInd = weighted_median(groups[j]);
		double medianX = groups[j][medianInd].getX();


		closestMedianDist = INFINITY;

		for (int i = 0; i < medians.size(); i++) {
			//cout << "i: " << i << "\n";
			if (abs(medians[i] - medianX) < closestMedianDist) {
				closestMedian = i;
				closestMedianDist = abs(medians[i] - medianX);
			}
		}
		if (closestMedianDist < .5) {
			for (Component c : groups[closestMedian]) { newGroups[closestMedian].push_back(c); }
			medians.push_back(INFINITY);
		}
		else {
			medians.push_back(medianX);
			for (Component c : groups[j]) { newGroups[j].push_back(c); }
		}
	}
	for (int i = newGroups.size() - 1; i >= 0; i--) {
		if (newGroups[i].size() == 0) {
			newGroups.erase(newGroups.begin() + i);
		}
	}
	return newGroups;
}



vector<vector<Component>> climb(vector<vector<Component>>& groups, vector<Component> components, vector<Header> headers, double maxLoad, int maxEvals = 10, int maxIters = 10, double stayProb = .9) {
	clock_t climb_start = clock();

	double bestScore = groupWeight(groups, headers);
	cout << "initial best weight: " << bestScore << "\n";
	vector<vector<Component>> bestGroups = groups;
	for (int j = 0; j < maxIters; j++) {
		vector<vector<Component>> currGroups = bestGroups;
		double oldBestScore = bestScore;
		//cout << "outer iteration: " << j << " best score: " << bestScore << "\n";
		//printGroups(bestGroups);
		for (int i = 0; i < maxEvals; i++) {
			//cout << "here1\n";
			vector<vector<Component>> neighb = stochasticStep(currGroups, stayProb);
			//cout << "here1Fin\n";
			if (groupsUnderLoad(neighb, maxLoad)) {
				clock_t c_start = clock();
				//cout << "here2\n";
				double weight = groupWeight(neighb, headers);
				//cout << "here3\n";
				//timeGettingWeight += (clock() - c_start);
				//cout << weight << "\n";
				//printGroups(neighb);
				if (weight < bestScore) {
					bestScore = weight;
					bestGroups = neighb;
				}
			}
		}
		/*
		if (j % 10 == 0) {
			vector<vector<Component>> neighb = medianFix(bestGroups,components,headers);
			double weight = groupWeight(neighb, headers);
			if (weight < bestScore || rand()%(j/5+1)==1) {
				//cout << "MEDIANFIX BETTER\n";
				bestScore = weight;
				bestGroups = neighb;
			}
		}
		/*
		if (oldBestScore == bestScore) {
			bestGroups = randomInit(components);
			bestScore = groupWeight(bestGroups, header1, header2);
		}
		*/
	}
	cout << "time spend in climb: " << (clock() - climb_start) << "\n";
	return bestGroups;
}


vector<vector<Component>> singletonInit(vector<Component> components) {
	vector<vector<Component>> newGroups;
	for (int i = 0; i < components.size(); i++) {
		newGroups.push_back(vector<Component>{components[i]});
	}
	return newGroups;
}

// To represent Disjoint Sets 
struct DisjointSets
{
	int* parent, * rnk;
	int n;

	// Constructor. 
	DisjointSets(int n)
	{
		// Allocate memory 
		this->n = n;
		parent = new int[n];
		rnk = new int[n];

		// Initially, all vertices are in 
		// different sets and have rank 0. 
		for (int i = 0; i < n; i++)
		{
			rnk[i] = 0;

			//every element is parent of itself 
			parent[i] = i;

		}
	}

	// Find the parent of a node 'u' 
	// Path Compression 
	int find(int u)
	{
		/* Make the parent of the nodes in the path
		   from u--> parent[u] point to parent[u] */
		if (u != parent[u])
			parent[u] = find(parent[u]);
		return parent[u];
	}




	// Union by rank 
	void merge(int x, int y, bool combineLoad = false)
	{

		x = find(x), y = find(y);

		/* Make tree with smaller height
		   a subtree of the other tree  */
		if (rnk[x] > rnk[y])
			parent[y] = x;
		else // If rnk[x] <= rnk[y] 
			parent[x] = y;

		if (rnk[x] == rnk[y])
			rnk[y]++;
	}

};

bool samePoint(dTriple point1, dTriple point2) {
	//point1.print();
	//point2.print();
	//cout << "\n";
	if (abs(point1.X - point2.X) < .05) {
		if (abs(point1.Y - point2.Y) < .05) {
			if (abs(point1.Z - point2.Z) < .05) {
				return true;
			}
		}
	}
	return false;
}

vector<vector<Component>> medianInit(vector<Component> components) {
	vector<vector<Component>> groups;
	double medianThreshold = 5;
	for (Component component : components) {
		int bestMedian = -1;
		int bestDist = INFINITY;
		for (int i = 0; i < groups.size(); i++) {
			sort(groups[i].begin(), groups[i].end(), compareComponentX);
			int median = weighted_median(groups[i]);
			double medianX = groups[i][median].getX();
			if (abs(medianX - component.getX()) < bestDist) {
				bestMedian = i;
				bestDist = abs(medianX - component.getX()) < bestDist;
			}
		}
		if (bestMedian = -1) {
			groups.push_back(vector<Component>{component});
		}
		else {
			groups[bestMedian].push_back(component);
		}
	}
	return groups;
}



vector<pipe> makePipesGroup(vector<Component > components, vector<Header> headers, double maxLoad, int maxEvals = 10, int maxIters = 10, double stayProb = .9) {
	srand(time(0));
	timeMakingPipes = 0;
	timeStochasticStep = 0;
	timeEvaluatingWeight = 0;
	timeGettingWeight = 0;
	vector<vector<Component>> groups = singletonInit(components);// , headers, maxLoad); // vector<vector<Component>>{ components };// , headers, maxLoad); // , headers, maxLoad);//{ components };//
	groups = climb(groups, components, headers, maxLoad, maxEvals, maxIters, stayProb);
	//cout << "\nFINAL GROUPS:\n";
	//printGroups(groups);
	//groups = medianFix(groups, components, headers);
	//groups = medianConsolidate(groups);
	//printGroups(groups);

	
	vector<pipe> pipes;
	for (vector<Component> group : groups) {
		vector<pipe> groupPipes = makePipesForGroup(group, headers);
		vector<pipe> groupPipesBend = makePipesForGroupBend(group, headers);
		//printPipes(groupPipesBend);
		//cout << "\n";
		if (pipeCost(groupPipes, headers) < pipeCost(groupPipesBend, headers)) {
			for (pipe elt : groupPipes) {
				pipes.push_back(elt);
			}
		}
		else {
			for (pipe elt : groupPipesBend) {
				pipes.push_back(elt);
			}
		}

	}
	/*
	cout << "time making pipes: " << timeMakingPipes<<"\n";
	cout << "time calculating weight : " << timeEvaluatingWeight << "\n";
	cout << "time getting weight : " << timeGettingWeight << "\n";
	cout << "time stochastic stepping : " << timeStochasticStep << "\n";
	*/
	return pipes;
}

vector<vector<Component>> makeGroupsGroup(vector<Component > components, vector<Header> headers, double maxLoad, int maxEvals = 10, int maxIters = 10, double stayProb = .9) {
	srand(time(0));
	timeMakingPipes = 0;
	timeStochasticStep = 0;
	timeEvaluatingWeight = 0;
	timeGettingWeight = 0;
	vector<vector<Component>> groups = singletonInit(components);// , headers, maxLoad); // vector<vector<Component>>{ components };// , headers, maxLoad); // , headers, maxLoad);//{ components };//
	groups = climb(groups, components, headers, maxLoad, maxEvals, maxIters, stayProb);
	return groups;
}