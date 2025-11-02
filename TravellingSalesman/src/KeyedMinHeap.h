#pragma once
#include <string>
#include <vector>
using namespace std;

// Similar to the traditional MinHeap, except it has a key associated with a value
// In this case, the key is the distance between a pair of Places.
// This has the effect of spitting out the lowest distance between 2 Places first, as well as those 2 places
template<typename T>
class KeyedMinHeap {
public:
	void insert(const double& key, const T& value) {
		mappedData.push_back(value);
		data.push_back(key);

		heapify_up(data.size() - 1);
	}
	std::pair<double, T> removeMin() {
		if (data.empty()) {
			throw runtime_error("No. Empty heap.");
		}
		double min = data[0];
		// reset the root node to the last added item to my heap
		data[0] = data.back();
		data.pop_back();

		T item = mappedData[0];
		mappedData[0] = mappedData.back();
		mappedData.pop_back();

		// now root node is the most recent added value to our heap
		// min contains the value that was once the root node
		heapify_down(0);

		// we want to have quick access to the children and the parent
		return pair<double, T>(min, item);
	}
	std::pair<double, T> peekMin() const {
		if (data.empty()) {
			throw runtime_error("No. Empty heap.");
		}
		double min = data[0];
		T item = mappedData[0];
		return pair<double, T>(min, item);
	}
	size_t size() {
		return data.size();
	}
private:
	// keys
	std::vector<double> data;
	// things associated with the keys
	std::vector<T> mappedData;

	// The idea between heapify_up and down both is to do to the mapped data exactly what you do to the keys.
	// This keeps indexing consistent between keys and mapped data, so that referencing both vectors at the same index
	// result in the same paired data that was inserted.
	void heapify_up(double i) {
		while (i != 0 && data[parent(i)] > data[i]) {
			swap(data[parent(i)], data[i]);
			swap(mappedData[parent(i)], mappedData[i]);
			i = parent(i);
		}
	}
	void heapify_down(double i) {
		double left = left_child(i);
		double right = right_child(i);
		double min_index = i;

		if (left < data.size() && data[left] < data[min_index]) {
			min_index = left;
		}
		if (right < data.size() && data[right] < data[min_index]) {
			min_index = right;
		}
		if (i != min_index) {
			swap(data[min_index], data[i]);
			swap(mappedData[min_index], mappedData[i]);

			heapify_down(min_index);
		}
	}
	double parent(double i) {
		return ((i - 1) / 2);
	}
	double left_child(double i) {
		return (2 * i) + 1;
	}
	double right_child(double i) {
		return (2 * i) + 2;
	}
};
