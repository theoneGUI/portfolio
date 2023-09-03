#pragma once
#include <vector>
#include "Path.h"
using std::vector;

// A container for a path
// Includes path length and index fields of Places visited - in order
class Path {
public:
	Path(vector<int> indexes, double pathLength)
		: indexVec{ indexes }, pathLen{ pathLength }
	{    }
	vector<int> path() const {
		return indexVec;
	}
	double length() const {
		return pathLen;
	}
	bool operator==(const Path& comp) const {
		if (pathLen != comp.pathLen)
			return false;
		if (indexVec.size() != comp.indexVec.size())
			return false;
		for (int i = 0; i < indexVec.size(); i++) {
			if (comp.indexVec[i] != indexVec[i])
				return false;
		}
		return true;
	}

private:

	vector<int> indexVec;
	double pathLen;
};
