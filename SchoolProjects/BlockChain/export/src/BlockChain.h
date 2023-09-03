#pragma once
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "Block.h"

using namespace rapidjson;
using std::string;
using std::unordered_map;
using std::cout;
using std::endl;
using std::vector;

typedef const char* BlockChainFilePath;

// Quick and dirty way to tell a read error from a parse error in a catch statement
class JSONParseError : public runtime_error {
public:
	JSONParseError(string x);
};
class BlockChainReadError : public runtime_error {
public:
	BlockChainReadError(string x);
};

class BlockChain {
public:
	// Make a new chain beginning with the genesis block given
	BlockChain(Block genesis);
	// Grab a JSON representation of a block chain out of the given file if possible
	BlockChain(BlockChainFilePath fp);

	// return a JSON representation of this block chain object
	string toJson();
	// send the JSON representation of this block chain object to the specified file path
	void toFile(string filePath);
	void messUpChain(int index, string newData);

	void addBlock(Block b);
	void addBlock(const string& sender, const string& receiver, int difficulty, const string& data);
	int verifyChain();
	void fixChain();

	// Run through a quick demonstration of the things the block chain can make itself do
	void demo();

	void setDifficulty(int newdiff);
	int getDifficulty();
	size_t chainLength();
	string getChainHash();

	void printAllBlocks();
	void printSenders(string receiver);
	void printReceivers(string sender);
private:
	void addBlock(Block b, bool isGenesis);
	int chainDiff;
	vector<Block> chain;
	vector<int> diffList;
	string chainhash;
	unordered_map<string, vector<string>> managedSenders;
	unordered_map<string, vector<string>> managedReceivers;
};

// Declare the standard genesis block
const Block genesis(string(""), string(""), string(""), 2, "Leeroy Jenkins");