#include<stdio.h> 
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <ctime>
#include <string>
#include <sstream>
#include <algorithm>
#include "Piping.h"
#include "csvParse.h"
#include "GroupAndMedian.h"

void writePipeToCsv(string filename, vector<pipe> pipes, vector<Component> components) {
	// File pointer 
	ofstream file;

	// Open an existing file 
	file.open(filename);
	file << "start X,start Y,start Z,end X,end Y,end Z,diameter\n";
	for (pipe elt : pipes) {
		file << elt.start.X << "," << elt.start.Y << "," << elt.start.Z << "," << elt.end.X << "," << elt.end.Y << "," << elt.end.Z << "," << elt.diameter << "\n";
	}
	file.close();

	// Open an existing file 
	file.open("components_" + filename);
	file << "X,Y,Z,flow,vital\n";
	for (Component elt : components) {
		file << elt.getX() << "," << elt.getY() << "," << elt.getZ() << "," << elt.getFlow() << "," << elt.isVital() << "\n";
	}
	file.close();
}

void writeGroupsToCsv(string filename, vector<vector<Component>> groups, vector<HeaderLoop> headers, vector<Component> components) {
	// File pointer 
	ofstream file;

	// Open an existing file 
	file.open(filename);
	file << "start X,start Y,start Z,end X,end Y,end Z,diameter\n";

	for (vector<Component> group : groups) {
		file << "group\n";
		vector<pipe> groupPipes = makePipesForGroup(group, headers);
		vector<pipe> groupPipesBend = makePipesForGroupBend(group, headers);
		//printPipes(groupPipesBend);
		//cout << "\n";
		if (pipeCost(groupPipes, headers) < pipeCost(groupPipesBend, headers)) {
			for (pipe elt : groupPipes) {
				file << elt.start.X << "," << elt.start.Y << "," << elt.start.Z << "," << elt.end.X << "," << elt.end.Y << "," << elt.end.Z << "," << elt.diameter << "\n";
			}
		}
		else {
			for (pipe elt : groupPipesBend) {
				file << elt.start.X << "," << elt.start.Y << "," << elt.start.Z << "," << elt.end.X << "," << elt.end.Y << "," << elt.end.Z << "," << elt.diameter << "\n";
			}
		}
	}

	file.close();

	// Open an existing file 
	file.open("components_" + filename);
	file << "X,Y,Z,flow,vital\n";
	for (Component elt : components) {
		file << elt.getX() << "," << elt.getY() << "," << elt.getZ() << "," << elt.getFlow() << "," << elt.isVital() << "\n";
	}
	file.close();
}

vector<vector<string>> read_csv_data(string filename)
{

	// File pointer 
	fstream fin;

	// Open an existing file 
	//cout << filename << "\n";
	fin.open(filename, ios::in);

	// Read the Data from the file 
	// as String Vector 
	vector<string> row;
	vector<vector<string>> table;
	string line, word, temp;
	int i = 0;
	getline(fin, line);
	//cout << "here\n";
	while (i < 1000 && line != "") {
		row.clear();
		//cout << line;
		stringstream ss(line);
		while (ss.good()) {
			getline(ss, word, ',');
			row.push_back(word);
			//cout << word;
		}
		table.push_back(row);
		getline(fin, line);
	}
	return table;
}

void printRow(vector<string> row) {
	for (string word : row) {
		cout << word << "\t";
	}
}

void printTable(vector<vector<string>> table) {
	for (vector<string> row : table) {
		printRow(row);
		cout << "\n";
	}
}

vector<Component> parseComponents(vector<vector<string>> table, int xInd, int yInd, int zInd, int loadInd, int firstRow, int numVital) {
	vector<Component> components;
	for (int i = firstRow; i < table.size(); i++) {
		if (stod(table[i][loadInd]) > 0) {
			if (i < numVital) {
				components.push_back(Component(stod(table[i][xInd]), stod(table[i][yInd]), stod(table[i][zInd]), stod(table[i][loadInd]), true));
			}
			else {
				components.push_back(Component(stod(table[i][xInd]), stod(table[i][yInd]), stod(table[i][zInd]), stod(table[i][loadInd])));
			}
		}
	}
	return components;
}

vector< vector<Component>> partitionComponents(vector<Component> components, vector<double> bulkheadXs) {
	vector< vector<Component>> partitions;
	for (int i = 0; i <= bulkheadXs.size(); i++) {
		partitions.push_back(vector<Component>{});
	}
	cout << "here1\n";
	sort(bulkheadXs.begin(), bulkheadXs.end());
	bool placed;
	for (Component component : components) {
		placed = false;
		for (int j = 0; j < bulkheadXs.size(); j++) {
			if (component.getX() < bulkheadXs[j]) {
				//cout<<bulkhead
				partitions[j].push_back(component);
				placed = true;
				break;
			}
		}
		if (!placed) {
			partitions[bulkheadXs.size()].push_back(component);
		}
	}
	return partitions;
}