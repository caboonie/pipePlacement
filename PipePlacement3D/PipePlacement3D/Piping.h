#pragma once
#include <tuple>
#include <stdio.h> 
#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>

const double headerDiameter = .30;
const double headerLoad = 5000;
const double velocity = 2; //m/s

enum class Direction {
	X_AXIS,
	Y_AXIS
};

using namespace std;

//length and inner diameter in meters, weight in kg
double weight(double diameter, double length);

double weightPerMeter(double diameter);

double valveWeight(double diameter);

double kWattLoadToDiameter(double kW);

struct dTriple {
	double X, Y, Z;
	dTriple() { X = 0; Y = 0; Z = 0; };
	dTriple(double nX, double nY, double nZ) {
		X = nX; Y = nY; Z = nZ;
	}
	bool operator == (const dTriple& Ref) const
	{
		return((X == Ref.X) && (Y == Ref.Y) && (Z == Ref.Z));
	}
	bool operator <(const dTriple& Ref) const
	{
		if (this->X == Ref.X) {
			if (Y == Ref.Y) {
				return Z < Ref.Z;
			}
			return this->Y < Ref.Y;
		}
		return this->X < Ref.X;
	} 
	void print() {
		cout << "(" << X << ", " << Y << "," << Z << ")";
	}
};

struct pipe {
	dTriple start;
	dTriple end;
	double diameter;
	double load;
	pipe() { start = dTriple(); end = dTriple(); diameter = 0; load = 0; }
	pipe(dTriple nstart, dTriple nend, double ndiameter, double nload) {
		start = nstart; end = nend; diameter = ndiameter; load = nload;
	}
	void print() {
		start.print();
		cout << " - ";
		end.print();
		cout << " diameter (m): " << diameter << " flow: " << load << "\n";
	}
	bool operator == (const pipe& Ref) {
		if (start == Ref.start && end == Ref.end) {
			return true;
		}
		if (end == Ref.start && start == Ref.end) {
			return true;
		}
		return false;
	}
};

class Component {
private:
	double X, Y, Z, flow;
	bool vital;
	int vital_dupe_ind;

public:

	Component() {
		X = 0.0; Y = 0.0; Z = 0.0;  flow = 0.0; vital = false; vital_dupe_ind = -1;
	}
	Component(const double& nX, const double& nY, const double& nZ, const double& nflow, const bool& nvital = false) {
		X = nX; Y = nY; Z = nZ; flow = nflow; vital = nvital; vital_dupe_ind = -1;
	}
	double getX() const { return X; }
	double getY() const { return Y; }
	double getZ() const { return Z; }
	dTriple point() const { return dTriple(X, Y, Z); }
	double getFlow() const { return flow; }
	bool isVital() const { return vital; }
	int getVitalDupe() { return vital_dupe_ind; }
	void setVitalDupe(int index) { vital_dupe_ind = index; }
	void print() { cout << "(" << X << "," << Y << "," << Z << "):" << flow << " is vital: " << vital << " vital dupe:" << vital_dupe_ind; }
	bool operator <(const Component& Ref) const
	{
		if (this->X == Ref.X) {
			return this->Y < Ref.Y;
		}
		return this->X < Ref.X;
	} 
	bool operator==(const Component& other) const
	{
		return flow == other.flow && X == other.X && Y == other.Y && Z== other.Z && vital == other.vital;
	}
};

template <>
struct hash<Component>
{
	size_t operator()(const Component& k) const
	{
		// Compute individual hash values for two data members and combine them using XOR and bit shifting
		return ((((hash<double>()(k.getX()) ^ (hash<double>()(k.getY()) << 1)) >> 1) ^ (hash<bool>()(k.isVital()) << 1)) >> 1) ^ (hash<double>()(k.getZ()) << 1) >> 1;
	}
};


template <>
struct hash<vector<Component>>
{
	size_t operator()(const vector<Component>& k) const
	{
		size_t hashValue = 0;
		// Compute individual hash values for two data members and combine them using XOR and bit shifting
		for (int i = 0; i < k.size(); i++) {
			hashValue += hash<Component>()(k[i]); // (hashValue ^ (hash<Component>()(k[i]) << 1)) >> 1;
		}
		return hashValue;
	}
};

class Header
{
public:
	double X, Y, Z;
	Direction direction;
	Header() { X = 0; Y = 0; Z = 0; direction = Direction::X_AXIS; };
	Header(const double& nX, const double& nY, const double& nZ, Direction ndirection) {
		X = nX; Y = nY; Z = nZ; direction = ndirection;
	}
};

class HeaderLoop {
public:
	vector<pipe> pipes;
	HeaderLoop(const vector<pipe>& npipes) {
		pipes = npipes;
	}
	bool intersects(dTriple point) {
		for (pipe elt : pipes) {
			if (point.Y == elt.start.Y && point.Z == elt.start.Z) {
				if (elt.start.X <= point.X && elt.end.X >= point.X)
					return true;
				if (elt.start.X >= point.X && elt.end.X <= point.X)
					return true;
			}
		}
		return false;
	}
	pipe pipeInX(double X) {
		for (pipe elt : pipes) {
			if (elt.start.X <= X && elt.end.X >= X)
				return elt;
			if (elt.start.X >= X && elt.end.X <= X)
				return elt;
		}
		cout << "Error - could not find header\n";
		return pipes[0];
	}
};


//double pipeWeight(vector<pipe> pipes, vector<Header> headers);
//double pipeResistance(vector<pipe> pipes, vector<Header> headers);
void printPipes(vector<pipe> pipes);
bool samePipe(pipe pipe1, pipe pipe2);
double pipeLength(pipe elt);
bool compPipeDiameter(pipe pipe1, pipe pipe2);
double pipeCost(vector<pipe> pipes, vector<HeaderLoop> headers, double pressureCoeff = .001);
double colocationPentalty(vector<vector<Component>>& groups, vector<vector<Component>>& colocationGroups);

