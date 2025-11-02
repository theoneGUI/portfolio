#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <unordered_map>
#include <stdexcept>

#include "Node.h"
#include "KeyedMinHeap.h"

using namespace std;

// Do not modify
string file_to_string(string fileName) {
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Unable to open the file: " << fileName << endl;
        throw std::runtime_error("Issue with file");
    }
    else {
        stringstream buffer;
        buffer << file.rdbuf();

        string fileText = buffer.str();
        streampos size = file.tellg();
        cout << "Uncompressed file size is: " << size << " bytes" << endl;
        file.close();
        return fileText;
    }
}

// Traversing
//	0 is left
//	1 is right


// It's janky but it works, trust me
// :3
string traverse(Node* ofInterest, unordered_map<char, Node*>& mapper) {
    auto found = mapper[ofInterest->character()];
    string traversalBackwards = "";
    Node* currentNode = found;
    while (currentNode->parent != nullptr) {
        if (currentNode->isLeftChild())
            traversalBackwards += "0";
        else
            traversalBackwards += "1";
        currentNode = currentNode->parent;
    }
    string traversalForwards = "";
    for (int i = traversalBackwards.size() - 1; i >= 0; i--) {
        traversalForwards += traversalBackwards[i];
    }
    return traversalForwards;
}

void huffman(string fileText){
    unordered_map<char, int> counts;
    for (const auto& i : fileText) {
        if (counts.find(i) == counts.end()) {
            counts[i] = 1;
        }
        else {
            counts[i]++;
        }
    }

    MinHeap heap;
    for (const auto& i : counts) {
        Node* newNode = new Node(i.first, i.second);
        heap.insert(newNode);
    }

    unordered_map<char, Node*> mapper;

    Node* root = nullptr;
    while (heap.size() > 1) {
        Node* first = heap.removeMin();
        Node* second = heap.removeMin();

        mapper[first->character()] = first;
        mapper[second->character()] = second;

        Node* imposter = new Node('\0', first->occurrences() + second->occurrences());
        imposter->rightChild(first);
        imposter->leftChild(second);

        first->parent = imposter;
        second->parent = imposter;

        heap.insert(imposter);
        root = imposter;
    }
    // Traversing
//	0 is left
//	1 is right

    unordered_map<char, string> encodings;
    for (const auto& i : mapper) {
        encodings[i.first] = traverse(i.second, mapper);
    }

    size_t encodedLen = 0;
    string encodedWithPipes;
    string regularEncoded;
    for (int i = 0; i < fileText.length(); i++) {
        encodedLen += encodings[fileText[i]].length();
        regularEncoded += encodings[fileText[i]];
        encodedWithPipes += encodings[fileText[i]];

        if (i != fileText.length() - 1)
            encodedWithPipes += "|";
    }

    double compressed = ceil((double)encodedLen / 8.0);
    int regular = fileText.length();
    cout << "\nThe original text was " << regular << " bytes long" << endl;
    cout << "This encoding uses " << compressed << " bytes" << endl;
    cout << "Encoding for human eyes: " << endl << encodedWithPipes << endl;
    cout << "Encoding for machine eyes: " << endl << regularEncoded << endl << endl;
    cout << "The byte-for-byte compression ratio is " << floor((compressed / (double)regular)*100) << "%" << endl;
}


// Do not modify the main
int main(){
    cout << "What is the file name??" << endl;
    string fileName;
    cin >> fileName;
    string fileText = file_to_string(fileName);
    cout << "File text is: ";
    cout  << fileText << endl;
    cout << "Compression information below: " << endl;
    huffman(fileText);
}