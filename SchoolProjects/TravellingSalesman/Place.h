#pragma once
#include <string>

// Moved to its own file because of dependency issues with multiple classes being in their own files

// Stores all the information for each place
// latitude and longitude are for extra credit only, feel free to ignore otherwise
struct Place {
	int index;
	std::string name;
	double x, y;
	double latitude, longitude;

	// I also added comparators between Places for use elsewhere
	bool operator==(const Place& in) const {
		return index == in.index &&
			name == in.name &&
			x == in.x &&
			y == in.y &&
			latitude == in.latitude &&
			longitude == in.longitude;
	}
	bool operator!=(const Place& in) const {
		return !operator==(in);
	}
};