#pragma once
#include <string>
#include <vector>
#include <stdexcept>
using namespace std;
#include "Node.h"

class MinHeap {
public:
	void insert(Node* value) {
		data.push_back(value);

		heapify_up(data.size() - 1);
	}
	Node* removeMin() {
		if (data.empty()) {
			throw runtime_error("No. Empty heap.");
		}
		auto min = data[0];
		data[0] = data.back();
		data.pop_back();

		heapify_down(0);

		return min;
	}
	Node* peekMin() const {
		if (data.empty()) {
			throw runtime_error("No. Empty heap.");
		}
		return data[0];
	}
	size_t size() {
		return data.size();
	}
private:
	std::vector<Node*> data;

	void heapify_up(double i) {
		while (i != 0 && data[parent(i)]->operator>(data[i])) {
			swap(data[parent(i)], data[i]);
			i = parent(i);
		}
	}
	void heapify_down(double i) {
		double left = left_child(i);
		double right = right_child(i);
		double min_index = i;

		if (left < data.size() && data[left]->operator<( data[min_index])) {
			min_index = left;
		}
		if (right < data.size() && data[right]->operator<(data[min_index])) {
			min_index = right;
		}
		if (i != min_index) {
			swap(data[min_index], data[i]);

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
