#pragma once
#include "Place.h"

// Container class that stores a Place and the index field of that Place
class Vertex
{
public:
	Vertex() {
		data = -1;
	}
	Vertex(int d, Place place) {
		data = d;
		p = place;
	}
	bool operator==(const Vertex& in) const {
		return (data == in.data) && p == in.p;
	}
	bool operator!=(const Vertex& in) const {
		return !this->operator==(in);
	}
	bool operator==(const int& in) const {
		return data == in;
	}
	bool operator!=(const int& in) const {
		return !this->operator==(in);
	}
	int index() const {
		return data;
	}
	Place place() const {
		return p;
	}
private:
	int data;
	Place p;
};

