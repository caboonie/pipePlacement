#include<stdio.h> 
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <ctime>
#include <string>
#include <unordered_map>
#include <sstream>
#include "Piping.h"
#include "GroupAndMedian.h"
#include "csvParse.h"

using namespace std;

int main() {
	//Header header1 = Header(0, 10, 0, Direction::X_AXIS);
	//Header header2 = Header(0, 0, 0, Direction::X_AXIS);
	//Component a(3, 4, 10);
	Component a(3, 4, 4, 10);
	Component b(4, 5, 5, 4);
	Component c(5, 6, 0, 5);
	Component d(50, 6, 0, 6);
	Component e(2, 3, 7, 3);
	Component f(6, 3.5, 0, 3.5);

	vector<Component> components = { a,b,c,d,e,f };
	//vector<Header> headers = { header1, header2 };
	clock_t c_start = clock();

	pipe header1 = pipe(dTriple(0, 5, 5), dTriple(20, 5, 5), headerDiameter, headerLoad);
	pipe header2 = pipe(dTriple(20, 8, 5), dTriple(40, 8, 5), headerDiameter, headerLoad);
	pipe header3 = pipe(dTriple(40, 10, 5), dTriple(100, 10, 5), headerDiameter, headerLoad);
	HeaderLoop headerLoop1(vector<pipe>{header1, header2, header3});

	pipe header4 = pipe(dTriple(0, -5, 5), dTriple(20, -5, 5), headerDiameter, headerLoad);
	pipe header5 = pipe(dTriple(20, -8, 5), dTriple(40, -8, 5), headerDiameter, headerLoad);
	pipe header6 = pipe(dTriple(40, -10, 5), dTriple(100, -10, 5), headerDiameter, headerLoad);
	HeaderLoop headerLoop2(vector<pipe>{header4, header5, header6});

	vector<HeaderLoop> headers = { headerLoop1, headerLoop2 };

	vector<pipe> pipesGroup = makePipesGroup(components, headers, headerLoad, 40, 40);
	cout << "CPU time used: " << 1000.0 * (clock() - c_start) / CLOCKS_PER_SEC << " ms\n";
	//printPipes(pipesGroup);
	cout << "Total pipe cost: " << pipeCost(pipesGroup, headers) << "\n";

	writePipeToCsv("testPipesSmall.csv", pipesGroup, components);


	//real data example:
	vector<vector<string>> table = read_csv_data("data.csv");
	//printTable(table);
	components = parseComponents(table); // , 6, 7, 8, 9, 3);

	//header1 = Header(0, 8, 2, Direction::X_AXIS);
	//header2 = Header(0, -8, 8, Direction::X_AXIS);
	//headers = { header1, header2 };

	vector < vector <Component>> zones = partitionComponents(components, vector<double>{42.5, 75, 107.5});
	vector<pipe> allPipes;
	vector<vector <Component>> groups;
	vector<vector <Component>> allGroups;


	for (vector<Component> zone : zones) {

		cout << "zone size: " << zone.size() << "\n";

		c_start = clock();
		groups = makeGroupsGroup(zone, headers, headerLoad, 150, 75, .95);
		//pipesGroup = makePipesGroup(zone, headers, headerLoad, 150, 75, .95);
		//cout << "CPU time used: " << 1000.0 * (clock() - c_start) / CLOCKS_PER_SEC << " ms\n";
		//printPipes(pipesGroup);
		//cout << "Total pipe weight: " << pipeWeight(pipesGroup, headers) << "\n";
		//cout << "Total pipe pressure change: " << pipeResistance(pipesGroup, headers) << "\n";
		//cout << "Total pipe cost: " << pipeCost(pipesGroup, headers) << "\n";

		//for (pipe elt : pipesGroup) {
		//	allPipes.push_back(elt);
		//}
		for (vector<Component> elt : groups) {
			allGroups.push_back(elt);
		}
	}

	//writePipeToCsv("testPipes.csv", allPipes, components);
	writeGroupsToCsv("testPipes.csv", allGroups, headers, components);
	

	/* Experiment MedianFix or no
	// File pointer
	ofstream file;

	// Open an existing file
	file.open("results2_noMedian.csv");
	file<<"maxNeighbors,maxIters,zone 1, zone 2, zone 3, zone 4,time\n";
	for (int i : {1, 10, 20, 50, 100}) {
		for (int j : {1, 10, 20, 50, 100}) {
			double timeUsed = 0;
			file << i << "," << j << ",";
			for (vector<Component> zone : zones) {

				cout << "zone size: " << zone.size() << "\n";

				c_start = clock();
				pipesGroup = makePipesGroup(zone, headers, headerLoad, i, j);
				//cout << "CPU time used: " << 1000.0 * (clock() - c_start) / CLOCKS_PER_SEC << " ms\n";
				timeUsed += 1000.0 * (clock() - c_start) / CLOCKS_PER_SEC;
				//printPipes(pipesGroup);
				//cout << "Total pipe weight: " << pipeWeight(pipesGroup, headers) << "\n";
				//cout << "Total pipe pressure change: " << pipeResistance(pipesGroup, headers) << "\n";
				cout << "Total pipe cost: " << pipeCost(pipesGroup, headers) << "\n";


				file << pipeCost(pipesGroup, headers)<<",";

			}
			file << timeUsed <<"\n";
		}
	}

	file.close();

	*/

	/*
	//Experiment different stay probs
	// File pointer
	ofstream file;

	// Open an existing file
	file.open("stayProbResults2.csv");
	file<<"stayProb,zone 1, zone 2, zone 3, zone 4,time\n";
	for (double i : {.25,.5,.6,.7,.8,.9,.95,.99,.999}) {
			double timeUsed = 0;
			file << i << ",";
			for (vector<Component> zone : zones) {

				cout << "zone size: " << zone.size() << "\n";

				c_start = clock();
				pipesGroup = makePipesGroup(zone, headers, headerLoad, 100,20,i);
				//cout << "CPU time used: " << 1000.0 * (clock() - c_start) / CLOCKS_PER_SEC << " ms\n";
				timeUsed += 1000.0 * (clock() - c_start) / CLOCKS_PER_SEC;
				//printPipes(pipesGroup);
				//cout << "Total pipe weight: " << pipeWeight(pipesGroup, headers) << "\n";
				//cout << "Total pipe pressure change: " << pipeResistance(pipesGroup, headers) << "\n";
				cout << "Total pipe cost: " << pipeCost(pipesGroup, headers) << "\n";


				file << pipeCost(pipesGroup, headers)<<",";

			}
			file << timeUsed <<"\n";

	}

	file.close();
	*/


	/*
	c_start = clock();
	pipes = makePipesMST(components, headers, 100);
	cout << "CPU time used: " << 1000.0 * (clock() - c_start) / CLOCKS_PER_SEC << " ms\n";
	//printPipes(pipes);
	cout << "Total pipe weight: " << pipeWeight(pipes, headers) << "\n";
	cout << "Total pipe pressure change: " << pipeResistance(pipes, headers) << "\n";
	cout << "Total pipe cost: " << pipeCost(pipes, headers) << "\n";

	c_start = clock();
	pipesDirect = makePipesDirect(components, headers);
	cout << "CPU time used: " << 1000.0 * (clock() - c_start) / CLOCKS_PER_SEC << " ms\n";
	//printPipes(pipesDirect);
	cout << "Total pipe weight: " << pipeWeight(pipesDirect, headers) << "\n";
	cout << "Total pipe pressure change: " << pipeResistance(pipesDirect, headers) << "\n";
	cout << "Total pipe cost: " << pipeCost(pipesDirect, headers) << "\n";


	c_start = clock();
	pipesGroup = makePipesGroup(components, headers, 5000, 40, 40);
	cout << "CPU time used: " << 1000.0 * (clock() - c_start) / CLOCKS_PER_SEC << " ms\n";
	//printPipes(pipesGroup);
	cout << "Total pipe weight: " << pipeWeight(pipesGroup, headers) << "\n";
	cout << "Total pipe pressure change: " << pipeResistance(pipesGroup, headers) << "\n";
	cout << "Total pipe cost: " << pipeCost(pipesGroup, headers) << "\n";
	*/
}