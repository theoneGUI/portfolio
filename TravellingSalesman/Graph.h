#pragma once
#include <vector>
#include <iostream>
#include "Vertex.h"
#include "Place.h"
#include "Path.h"

using namespace std;

// Graph houses a link-based graph, not an adjacency matrix-based graph
class Graph
{
public:
	// Insert a vertex into the graph
	void insert(const Vertex& x) {
		verts.push_back(x);
		relations.push_back(vector<Vertex>());
		weights.push_back(vector<double>());
	}

	// Connect 2 vertices with a weight determined by the vertices' distance on Earth
	void connect(const Vertex& v1, const Vertex& v2) {
		size_t vertex1 = find(v1);
		size_t vertex2 = find(v2);
		if (vertex1 == -1 || vertex2 == -1)
			return;
		relations[vertex1].push_back(v2);
		relations[vertex2].push_back(v1);
		auto dist = distance(v1.place(), v2.place());
		weights[vertex1].push_back(dist);
		weights[vertex2].push_back(dist);
	}

	// Get all verticies adjacent to the given vertex if it exists
	vector<Vertex> adjacent(const Vertex& v) const {
		size_t vector1 = find(v);
		if (vector1 == -1) {
			return vector<Vertex>();
		}
		return relations[vector1];
	}

	// Make a complete graph, connecting all vertices to each other
	void connectAll() {
		for (int i = 0; i < verts.size(); i++) {
			for (int j = 0; j < verts.size(); j++) {
				if (i != j)
					connect(verts[i], verts[j]);
			}
		}
	}

	// Begin a depth-first search to result in a minimal spanning tree
	void minDfs(const Vertex& start) {
		auto index = find(start);
		if (index == -1)
			return;
		cout << start.index();
		vector<Vertex> visited = { start };

		double number = 0;
		double minDist = numeric_limits<double>::max();
		Vertex minIndex;

		for (int i = 0; i < relations[index].size(); i++) {
			auto w = weights[index][i];
			auto v = relations[index][i];
			if (!vecFind(relations[index][i], visited)) {
				if (w < minDist) {
					minDist = w;
					minIndex = v;
				}
			}
		}
		visited.push_back(minIndex);
		number += minDist;
		cout << ", " << minIndex.index();
		minDfs(minIndex, visited, number);

		// And then go back to the start...
		number += distance(start.place(), visited.back().place());
		cout << ", " << start.index() << endl;
		cout << "\nDone in: " << number << endl;
	}

private:
	// Version of minDfs written to be called recursively
	void minDfs(const Vertex& start, vector<Vertex>& visited, double& number) {
		auto index = find(start);
		if (index == -1)
			return;
		double minDist = numeric_limits<double>::max();
		Vertex minIndex;

		for (int i = 0; i < relations[index].size(); i++) {
			auto w = weights[index][i];
			auto& v = relations[index][i];
			if (!vecFind(relations[index][i], visited)) {
				if (w < minDist) {
					minDist = w;
					minIndex = v;
				}
			}
		}

		if (minDist == numeric_limits<double>::max()) {
			return;
		}
		visited.push_back(minIndex);
		number += minDist;
		cout << ", " << minIndex.index();
		minDfs(minIndex, visited, number);
	}

	// utility to determine if a vertex is contained in a vector of vertices
	bool vecFind(const Vertex& v, const vector<Vertex>& vec) {
		for (const auto& i : vec) {
			if (i == v)
				return true;
		}
		return false;
	}

	// Utility to find distance between Places
	double distance(const Place& a, const Place& b) {
		double x = a.x - b.x;
		double y = a.y - b.y;
		return sqrt(x * x + y * y);
	}

	// Find the index of a vertex in the verts vector, if it exists
	size_t find(const Vertex& x) const {
		for (int i = 0; i < verts.size(); i++) {
			if (x == verts[i]) {
				return i;
			}
		}
		return -1;
	}
	
	// Find the index of a vertex in the verts vector based on the Place object's index field
	size_t find(const int& x) const {
		for (int i = 0; i < verts.size(); i++) {
			if (verts[i] == x) {
				return i;
			}
		}
		return -1;
	}

	// Where we store the things
	vector<Vertex> verts;
	vector<vector<Vertex>> relations;
	vector<vector<double>> weights;
};

