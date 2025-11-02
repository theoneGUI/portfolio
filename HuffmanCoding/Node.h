#pragma once

class BinTree;

class Node {
public:
	Node(char c, unsigned count)
	: c{c}, count{count}
	{

	}
	Node() {
	}
	auto character() const{
		return c;
	}
	auto occurrences() const {
		return count;
	}

	bool isRightChild() const {
		if (parent == nullptr)
			return false;
		return this == parent->rchild;
	}
	bool isLeftChild()const {
		if (parent == nullptr)
			return false;
		return this == parent->lchild;
	}

	void rightChild(Node* rc) {
		rchild = rc;
	}
	void leftChild(Node* lc) {
		lchild = lc;
	}

	Node* rightChild() const {
		return rchild;
	}
	Node* leftChild() const {
		return lchild;
	}

	bool operator<(const Node& rhs) const {
		return this->count < rhs.count;
	}
	bool operator>(const Node& rhs) const {
		return this->count > rhs.count;
	}

	bool operator<(const Node* rhs) const {
		return this->count < rhs->count;
	}
	bool operator>(const Node* rhs) const {
		return this->count > rhs->count;
	}

	bool operator==(const Node rhs) const {
		return (this->c == rhs.c) && (this->count == rhs.count);
	}
	bool operator!=(const Node rhs) const {
		return operator==(rhs);
	}

	bool operator==(Node* rhs) const {
		return (this->c == rhs->c) && (this->count == rhs->count);

	}
	bool operator!=(Node* rhs) const {
		return operator==(rhs);
	}

public:
	// this is bad but I'm getting tired of playing encapuslation games with pointers
	Node* parent = nullptr;
	Node* rchild = nullptr;
	Node* lchild = nullptr;
private:
	char c = '\0';
	unsigned count = -1;
};