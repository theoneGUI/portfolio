#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <numeric>
#include <limits>
#include <cmath>


#include "Place.h"
#include "KeyedMinHeap.h"
#include "Graph.h"
#include "Path.h"

#include "rapidjson/document.h"
// You may not add additional imports!


using namespace std;
using namespace rapidjson;

vector<string> split(const string& s, char delimiter) {
	vector<string> places;
	string place;
	istringstream placeStream(s);
	while (getline(placeStream, place, delimiter)) {
		places.push_back(place);
	}
	return places;
}


// You may notice a distinct lack of Place right around here,
// That is because I had dependency issues with having classes in different files.
// Place has been relocated to its own header file.


// Simple distance in 2D grid, ignoring the curvature of the Earth
double distance(const Place& a, const Place& b) {
	double x = a.x - b.x;
	double y = a.y - b.y;
	return sqrt(x * x + y * y);
}

// goes through the Place vector and totals the distance within it.
// Returns a Path container object with length and path taken included
Path processPermutation(const Place& start, const vector<Place>& places) {
	vector<int> indexes = { start.index };
	double pathLength = distance(start, places[0]);
	for (int i = 0; i < places.size() - 1; i++) {
		pathLength += distance(places[i], places[i + 1]);
	}
	for (int i = 0; i < places.size(); i++) {
		indexes.push_back(places[i].index);
	}
	indexes.push_back(start.index);
	pathLength += distance(start, places[places.size() - 1]);
	return Path(indexes, pathLength);
}

// Modified from the original string permutation example in class to fit this assignment
void permuteVector(const Place& start, vector<Place>& a, int l, int r, KeyedMinHeap<Path>& possiblePaths) {
	//cout << "Function has been called!" << endl;
	// This is a permutation of your string, we print it
	if (l == r) {
		Path p = processPermutation(start, a);
		possiblePaths.insert(p.length(), p);
	}
	else {
		for (int i = l; i <= r; i++) {
			swap(a[l], a[i]);
			permuteVector(start, a, l + 1, r, possiblePaths);
			swap(a[l], a[i]);
		}
	}
}

/*
 * TODO: your brute force approach to solving TSP goes here
 * input is a vector of places, and a path
 * return the total distance traveled
 * update the vector path for the indices you travel
 */
double tsp_bruteforce(const vector<Place>& places, vector<int>& path) {
	auto& start = places[0];
	vector<Place> thingsToPermute;
	vector<Place> compare;
	size_t len = places.size() - 1;

	size_t thingsDone = 0;
	for (int i = 1; i < places.size(); i++) {
		thingsToPermute.push_back(places[i]);
	}

	KeyedMinHeap<Path> possiblePaths;
	permuteVector(start, thingsToPermute, 0, thingsToPermute.size() - 1, possiblePaths);

	auto shortest = possiblePaths.removeMin();
	for (const auto& i : shortest.second.path()) {
		path.push_back(i);
	}

	return shortest.first;
}

// Lookup a place object by its index field within a vector of Places
Place placeByIndex(const vector<Place>& places, int index) {
	for (const auto& i : places) {
		if (index == i.index) {
			return i;
		}
	}
	return Place();
}

// Find if a specified vector contains a thing
bool vecContains(const vector<Vertex>& x, const Vertex& y) {
	for (const auto& i : x) {
		if (i == y)
			return true;
	}
	return false;
}

// Follows along Prim's algorithm to traverse the Graph
// Returns the shortest Path found
Path prims(const Place& start, const Graph& g, const vector<Place>& places) {
	Vertex startingVertex = Vertex(start.index, start);
	KeyedMinHeap<pair<Place, Place>> heap;

	vector<Vertex> visited;
	double length = 0;

	// start off our minheap with the place we're starting at
	heap.insert(0, pair<Place, Place>(Place(), start));

	// Set all min distances to "infinity"
	vector<int> D(20);
	for (const auto& i : places) {
		D[i.index] = numeric_limits<int>::max();
	}

	D[start.index] = 0;

	Vertex root(start.index, start);
	// Go through our heap until it's empty
	while (heap.size() != 0) {
		auto min = heap.removeMin();
		if (!vecContains(visited, Vertex(min.second.second.index, min.second.second))) {
			// if we haven't visited this vertex, consider it visited
			// and add the distance from this vertex to the total
			visited.push_back(Vertex(min.second.second.index, min.second.second));
			length += min.first;
		}
		// find the minimum distance to an adjacent vertex and add to the heap
		auto adjacency = g.adjacent(Vertex(min.second.second.index, min.second.second));
		for (const auto& i : adjacency) {
			double w = distance(min.second.second, placeByIndex(places, i.index()));
			if (w < D[i.index()]) {
				D[i.index()] = w;
				heap.insert(w, pair<Place, Place>(min.second.second, placeByIndex(places, i.index())));
			}
		}
	}
	// Go back to the starting vertex
	visited.push_back(startingVertex);
	length += distance(start, placeByIndex(places, visited.back().index()));

	// Geta vector of Places equivalent to the vector of ints referring to their indexes
	vector<Place> visitedPlaces;
	for (int i = 1; i < visited.size()-1; i++) {
		visitedPlaces.push_back(placeByIndex(places, visited[i].index()));
	}

	// Get the path traversal and length of the path
	Path p = processPermutation(start, visitedPlaces);
	return p;
}

/*
 * TODO: your approximation approach to solving TSP goes here
 * input is a vector of places, and a path
 * return the total distance traveled
 * update the vector path for the indices you travel
 */
double tsp_approx(const vector<Place>& places, vector<int>& path) {
	auto& start = places[0];
	Graph g;
	// Make a complete graph on the places we're given
	for (int i = 0; i < places.size(); i++) {
		g.insert(Vertex(places[i].index, places[i]));
	}
	g.connectAll();

	// Alternate solution - using a minimum depth first search
	//Vertex startV(start.index, start);
	//g.minDfs(startV);

	// Primary solution using Prim's
	auto mst = prims(start, g, places);
	// Load the path given by Prim's into the path variable
	for (const auto& i : mst.path()) {
		path.push_back(i);
	}
	return mst.length();
}

/* TODO: write out a KML file containing the paths of your travel
 * inputs include the places you go (which will get you name, lat, long)
 * the brute force path found
 * the approx path found
 * filename which is defaulted to HW4.KML in main
 */
void output_kml(const vector<Place>& places, const vector<int>& brute_path, const vector<int>& approx_path, const string& filename) {
	// Create a KML file with the coordinates of each path in order 
	// Separate the different solutions both verically and by color
	string output = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	output += "<kml xmlns = \"http://www.opengis.net/kml/2.2\">";
	output += "  <Document>";
	output += "    <name>TSP Brute Force &amp; Approximation</name>";
	output += "    <description>Shortest path available and approximation</description>";
	output += "    <Style id=\"line1\">";
	output += "      <LineStyle>";
	output += "        <color>ff00ffff</color>";
	output += "       <width>4</width>";
	output += "      </LineStyle>";
	output += "    </Style>";
	output += "    <Style id=\"line2\">";
	output += "      <LineStyle>";
	output += "        <color>ff0000ff</color>";
	output += "       <width>4</width>";
	output += "      </LineStyle>";
	output += "    </Style>";
	output += "    <Placemark>";
	output += "      <name>Brute Force</name>";
	output += "      <description>Brute force's solution to the TSP</description>";
	output += "      <styleUrl>#line1</styleUrl>";
	output += "      <LineString>";
	output += "        <altitudeMode>absolute</altitudeMode>";
	output += "        <coordinates>";
	for (int i = 0; i < brute_path.size(); i++) {
		auto thisPlace = placeByIndex(places, brute_path[i]);
		output += to_string(thisPlace.longitude);
		output += ",";
		output += to_string(thisPlace.latitude);
		output += ",275";

		output += "\n";
	}
	output += "        </coordinates>";
	output += "      </LineString>";
	output += "    </Placemark>";
	output += "    <Placemark>";
	output += "      <name>Prim's Solution</name>";
	output += "      <description>Efficient way to solve the TSP with Prim's algorithm</description>";
	output += "      <styleUrl>#line2</styleUrl>";
	output += "      <LineString>";
	output += "        <altitudeMode>absolute</altitudeMode>";
	output += "        <coordinates>";
	for (int i = 0; i < approx_path.size(); i++) {
		auto thisPlace = placeByIndex(places, approx_path[i]);
		output += to_string(thisPlace.longitude);
		output += ",";
		output += to_string(thisPlace.latitude);
		output += ",222";

		output += "\n";
	}
	output += "        </coordinates>";
	output += "      </LineString>";
	output += "    </Placemark>";
	output += "  </Document>";
	output += "</kml>";

	ofstream out(filename);
	out << output;
	out.close();
}

// Do not touch the main code!

// the main code has been touched slightly.
// I changed the way the Place indexes are printed out to match my implementation
// Nothing functionally has changed other than that
int main() {
    ifstream file("coolplaces2.txt");
    string str((istreambuf_iterator<char>(file)),istreambuf_iterator<char>());
    Document doc;
    doc.Parse(str.c_str());
    vector<Place> places;
    const Value &placesArray = doc["places"];

    for (const auto &placeValue : placesArray.GetArray()) {
        Place place;
        place.index = placeValue["index"].GetInt();
        place.name = placeValue["name"].GetString();
        place.x = placeValue["x"].GetDouble();
        place.y = placeValue["y"].GetDouble();
        place.latitude = placeValue["latitude"].GetDouble();
        place.longitude = placeValue["longitude"].GetDouble();
        places.push_back(place);
    }

    double min_x = numeric_limits<double>::max();
    double min_y = numeric_limits<double>::max();

    // Find smallest x,y values present in an arbitrary JSON file
    // You could theoretically reuse this code on any place you wish!
    for (const auto& place : places) {
        if (place.x < min_x) {
            min_x = place.x;
        }
        if (place.y < min_y) {
            min_y = place.y;
        }
    }

    // Normalize the x and y values for readability
    for (auto& place : places) {
        place.x -= min_x;
        place.y -= min_y;
    }

    string input;
    for(int i = 0; i < places.size(); i++){
        cout << "Index: " << places[i].index << " Location: " << places[i].name << endl;
    }
    cout << "Enter comma-separated indices of the places to visit the first number is your starting point: ";
    getline(cin, input);

    vector<string> selectedLocations = split(input, ',');
    vector<int> indices(selectedLocations.size());

    for (size_t i = 0; i < selectedLocations.size(); ++i) {
        indices[i] = stoi(selectedLocations[i]) - 1;
    }

    cout << "You will begin your tour at: " << selectedLocations[0] << endl;
    cout << "You wish to visit: ";
    for(int i = 1; i < selectedLocations.size(); i++){
        cout << selectedLocations[i] << " ";
    }
    cout << endl << "Before finally returning to: " << selectedLocations[0] << endl;

    vector<Place> selected_places(indices.size());
    for (int i = 0; i < indices.size(); ++i) {
        selected_places[i] = places[indices[i]];
    }

    vector<int> path;
    double min_dist = tsp_bruteforce(selected_places, path);

    cout << "Minimum distance (brute): " << min_dist << " meters" << endl;
    cout << "Path: ";
    for (int index : path) {
        cout << index << "->";
    }
	cout << "\b\b  " << endl;

    vector<int> path2;
    double approx_dist = tsp_approx(selected_places,path2);

    cout << "Approx distance (TSP-CLRS): " << approx_dist << " meters" << endl;
    cout << "Path: ";
    for (int index : path2) {
        cout << index << "->";
    }
    cout << "\b\b  " << endl;

    // Uncomment the line below when you are ready to work on the extra credit
    output_kml(places, path, path2,"hw4.kml");

    return 0;
}