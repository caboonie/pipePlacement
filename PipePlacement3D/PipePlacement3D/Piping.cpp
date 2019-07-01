#include<stdio.h> 
#include <iostream>
#include <vector>
#include <tuple>
#include <stdio.h>
#include <algorithm>
#include <map>
#include <chrono>
#include "Piping.h"



using namespace std;

double kWattLoadToDiameter(double kW) {
	return sqrt(4 * kW * 1000 / (3.14 * velocity * 4.184 * 10 * 1000000)); //sqrt( 4* Q/(pi*V*Cp*deltaT)) - m = W/((m/s)*J/(Cg)*C*g/m^3)
}

double weight(double diameter, double length) {
	return length * (1152.9 * diameter * diameter + 20.278 * diameter + 1.1415);
}

double weightPerMeter(double diameter) {
	return (1152.9 * diameter * diameter + 20.278 * diameter + 1.1415);
}

double valveWeight(double diameter) {
	return 173241 * pow(diameter, 3) + 744.58 * pow(diameter, 2) + 229.85 * diameter;
}

double TWeight(double diameter) {
	return 326.32 * pow(diameter, 2) - 4.9133 * diameter;
}

double reducerWeight(double diameter) { //NOT ACCURATE yet
	return 638.18 * pow(diameter, 2) - 19.366 * diameter;
}


bool samePipe(pipe pipe1, pipe pipe2) {
	if (pipe1.start == pipe2.start && pipe1.end == pipe2.end) {
		return true;
	}
	if (pipe1.end == pipe2.start && pipe1.start == pipe2.end) {
		return true;
	}
	return false;
}



bool compPipeDiameter(pipe pipe1, pipe pipe2) {
	return pipe1.diameter < pipe2.diameter;
}

double pipeLength(pipe elt) {
	return abs(elt.start.X - elt.end.X) + abs(elt.start.Y - elt.end.Y) + abs(elt.start.Z - elt.end.Z);
}

double deltaP(double K) {
	//cout << "K: " << K << "\n";
	return  .5 * 1000 * velocity * velocity * K; //kg/m^3 * (m/s)^2 * K
}

double resistance(double diameter, double length) {
	if (diameter == 0 || length == 0) {
		return 0;
	}
	//cout << "diameter: " << diameter << " length: " << length << "\n";
	double nu = 0.00000144492;
	double eD = .0015 / diameter;
	double Re = diameter * velocity / nu; //DV/nu
	//cout << "Reynolds number: " << Re << "\n";
	double fD = .0055 * (1 + pow((20000 * eD + 1000000 / Re), 1 / 3));
	//cout << "fD: " << fD << "\n";
	double K = fD * length / diameter;
	/*if (K > 5) {
		cout << "K: " << K << "\n";
		cout << "diameter: " << diameter << " length: " << length << "\n";
	}*/
	double dp = deltaP(K);
	//cout << "dp: " << dp << "\n";
	return dp;
}

double bendResistance(double diameter) {
	return deltaP(.1); //coefficient for bends with 90 deg with R/D=5
}

//where flow ratio is flow through run over flow from source
double resistanceThroughRun(double flowRatio) {
	double K = .62 - .98 * pow(flowRatio, -1) + .36 * pow(flowRatio, -2) + .03 * pow(flowRatio, 6);
	//cout << "through run K: " << K << "\n";
	return deltaP(K);
}

//where flow ratio is flow through branch over flow from source
//diameter ratio is diameter of branch over diameter of source
double resistanceThroughBranch(double flowRatio, double diameterRatio) {
	if (flowRatio == 0 || diameterRatio == 0) {
		return 0;
	}
	double K9 = .57;// double ratioRToD = 0;
	double K = (.81 - 1.13 * pow(flowRatio, -1) + pow(flowRatio, -2)) * pow(diameterRatio, 4) + 1.12 * diameterRatio - 1.08 * pow(diameterRatio, 3) + K9;
	//cout << "through branch K: " << K << "\n";
	return deltaP(K);
}

double reducerResistance() {
	return 0;
}



void printPipes(vector<pipe> pipes) {
	for (pipe elt : pipes) {
		elt.print();
	}
}

/*
double pipeCost(vector<pipe> pipes, vector<Header> headers, double pressureCoeff) {
	return pipeWeight(pipes, headers) + pressureCoeff * pipeResistance(pipes, headers);
}
*/

double pipeCost(vector<pipe> pipes, vector<HeaderLoop> headers, double pressureCoeff) {
	map<dTriple, vector<pipe>> intersectsPerNode;
	double pressureSum = 0;
	double weightSum = 0;

	for (pipe elt : pipes) {
		double Xlength = abs(elt.start.X - elt.end.X);
		double Ylength = abs(elt.start.Y - elt.end.Y);
		double Zlength = abs(elt.start.Z - elt.end.Z);
		for (double length : {Xlength, Ylength, Zlength}) {
			pressureSum += resistance(elt.diameter, length);
			weightSum += weight(elt.diameter, length);
		}

		if (Xlength != 0 && Ylength != 0) {
			pressureSum += bendResistance(elt.diameter);
		}
		for (dTriple point : {elt.start, elt.end}) {
			point = dTriple(round(point.X * 10) / 10, round(point.Y * 10) / 10, round(point.Z * 10) / 10); //removes decimal places to make nodes that are close the same
			if (intersectsPerNode.find(point) != intersectsPerNode.end()) {
				intersectsPerNode.at(point).push_back(elt);
			}
			else {
				intersectsPerNode.insert(pair < dTriple, vector<pipe>>(point, vector<pipe>{elt}));
				if (headers[0].intersects(point) || headers[1].intersects(point)) { //if it connects to the header, add the header as an intersection
					intersectsPerNode.at(point).push_back(pipe(elt.start, elt.end, headerDiameter, headerLoad));
				}
			}
		}
	}

	map<dTriple, vector<pipe>>::iterator itr;
	for (itr = intersectsPerNode.begin(); itr != intersectsPerNode.end(); ++itr) {
		if (itr->second.size() >= 2) { //at this point it's a component, so have the branch flow into the component
			pipe minPipe = itr->second[0];
			pipe maxPipe = itr->second[0];
			for (pipe elt : itr->second) {
				minPipe = min(minPipe, elt, compPipeDiameter);
				maxPipe = max(maxPipe, elt, compPipeDiameter);
			}

			if (maxPipe.load != 0 || maxPipe.diameter != 0) {

				double flowRatioRun;
				double flowRatioBranch;
				double diameterRatioBranch;
				if (minPipe.load / maxPipe.load > .5) {
					flowRatioRun = minPipe.load / maxPipe.load;
					diameterRatioBranch = kWattLoadToDiameter(maxPipe.load - minPipe.load) / maxPipe.diameter;
				}
				else {
					flowRatioRun = 1 - minPipe.load / maxPipe.load;
					diameterRatioBranch = minPipe.diameter / maxPipe.diameter;
				}
				flowRatioBranch = 1 - flowRatioRun;
				pressureSum += resistanceThroughRun(flowRatioRun);
				//cout << flowRatioBranch << diameterRatioBranch;
				pressureSum += resistanceThroughBranch(flowRatioBranch, diameterRatioBranch);
			}

			weightSum += valveWeight(minPipe.diameter);
			weightSum += TWeight(maxPipe.diameter); //also add on weight of T-intersections
		}
	}

	return pressureCoeff * pressureSum + weightSum;
}

double colocationPentalty(vector<vector<Component>>& groups, vector<vector<Component>>& colocationGroups) {
	//for every pair of components that should be colocated, but are not, add penalty of 1
	map<Component, int> componentToGroup;
	for (int i = 0; i < groups.size(); i++) {
		for (Component component : groups[i]) {
			componentToGroup.insert(pair<Component, int>(component, i));
		}
	}
	double penalties = 0;
	for (vector<Component> group : colocationGroups) {
		for (int i = 0; i < group.size(); i++) {
			for (int j = i + 1; j < group.size(); j++) {
				if (componentToGroup.at(group[i]) != componentToGroup.at(group[j])) {
					penalties += 1;
				}
			}
		}
	}
	return penalties;
}


