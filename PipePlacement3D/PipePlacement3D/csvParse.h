#pragma once
#include <tuple>
#include<stdio.h> 
#include <iostream>
#include <vector>

using namespace std;

vector<vector<string>> read_csv_data(string filename);
void printTable(vector<vector<string>> table);
vector<Component> parseComponents(vector<vector<string>> table, int xInd = 2, int yInd = 3, int zInd = 4, int loadInd = 7, int firstRow = 1, int numVital = 100);
vector< vector<Component>> partitionComponents(vector<Component> components, vector<double> bulkheads);
void writePipeToCsv(string filename, vector<pipe> pipes, vector<Component> components);
void writeGroupsToCsv(string filename, vector<vector<Component>> groups, vector<HeaderLoop> headers, vector<Component> components);