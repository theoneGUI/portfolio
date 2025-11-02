#pragma once
#include "BlockChain.h"

// Errors implementations
BlockChainReadError::BlockChainReadError(string x) : runtime_error(x) {}
JSONParseError::JSONParseError(string x) : runtime_error(x) {}

// BlockChain class implementation

BlockChain::BlockChain(Block genesis) {
	addBlock(genesis, true);
}
BlockChain::BlockChain(BlockChainFilePath fp) {
	ifstream file(fp);
	std::stringstream buffer;
	buffer << file.rdbuf();
	string contents = buffer.str();

	Document jsonDoc;
	jsonDoc.Parse(contents.c_str());
	if (jsonDoc.HasParseError())
		throw JSONParseError("Invalid JSON");

	if (!jsonDoc.HasMember("sender_map") ||
		!jsonDoc.HasMember("receiver_map") ||
		!jsonDoc.HasMember("difficulty_list") ||
		!jsonDoc.HasMember("blockchain") ||
		!jsonDoc.HasMember("chainhash")
		)
		throw BlockChainReadError("Missing data");

	auto obj = jsonDoc.GetObject();

	chainhash = obj["chainhash"].GetString();

	auto diffArr = obj["difficulty_list"].GetArray();
	auto blockchain = obj["blockchain"].GetArray();

	auto senders = obj["sender_map"].GetObject();
	for (auto i = senders.MemberBegin(); i != senders.MemberEnd(); i++) {
		auto sendname = i->name.GetString();
		vector<string> tmp;
		auto recips = i->value.GetArray();
		for (auto j = recips.Begin(); j != recips.End(); j++) {
			tmp.push_back(j->GetString());
		}
		managedSenders[sendname] = tmp;
	}

	auto receivers = obj["receiver_map"].GetObject();
	for (auto i = receivers.MemberBegin(); i != receivers.MemberEnd(); i++) {
		auto recvname = i->name.GetString();
		vector<string> tmp;
		auto sends = i->value.GetArray();
		for (auto j = sends.Begin(); j != sends.End(); j++) {
			tmp.push_back(j->GetString());
		}
		managedReceivers[recvname] = tmp;
	}

	auto blockChain = obj["blockchain"].GetArray();
	for (auto i = blockChain.Begin(); i != blockChain.End(); i++) {
		unordered_map<string, string> blockInChain;
		auto blockObj = i->GetObject();
		auto prev = blockObj["previoushash"].GetString();
		auto send = blockObj["sender"].GetString();
		auto recv = blockObj["recipient"].GetString();
		uint64_t nonc = blockObj["nonce"].GetUint64();
		auto data = blockObj["data"].GetString();
		auto diff = blockObj["difficulty"].GetInt();
		diffList.push_back(diff);
		chainDiff = diff;

		Block block(send, recv, nonc, prev, diff, data);
		chain.push_back(block);
	}


}
int BlockChain::verifyChain() {
	string previousHash = "";
	if (chain.size() == 1 || chain.size() == 0)
		return -1;
	for (size_t i = 0; i < chain.size(); i++) {
		if (i == 0) {
			previousHash = chain[i].getBlockHash();
			continue;
		}
		Block b = chain[i];
		if (i == chain.size() - 1) {
			if (b.getPreviousHash() != chain[i - 1].getBlockHash())
				return i - 1;
			if (b.getBlockHash() != chainhash)
				return i;
		}
		else {
			if (b.getPreviousHash() != previousHash)
				return i;
		}

		previousHash = b.getBlockHash();
	}
	return -1;
}
void BlockChain::fixChain() {
	string previousHash = "";
	vector<Block> fixed;
	for (auto& i : chain) {
		Block copy(i.getSender(), i.getReceiver(), previousHash, i.getDifficulty(), i.getData());
		copy.figureOutNonce();
		previousHash = copy.getBlockHash();
		fixed.push_back(copy);
		chainhash = copy.getBlockHash();
	}
	chain = fixed;
}
string BlockChain::toJson() {
	Document jsonDoc;
	jsonDoc.SetObject();
	Document::AllocatorType& alloc = jsonDoc.GetAllocator();

	Value difficultyList;
	difficultyList.SetArray();
	for (const auto& i : diffList) {
		Value v(i);
		difficultyList.PushBack(v, alloc);
	}

	rapidjson::Value sendMap(rapidjson::kObjectType);
	for (auto& i : managedSenders) {
		Value recvArr;
		recvArr.SetArray();
		for (auto& j : i.second) {
			Value v;
			v.SetString(StringRef(j.c_str(), j.length()));
			recvArr.PushBack(v, alloc);
		}
		Value name;
		name.SetString(StringRef(i.first.c_str(), i.first.length()));
		sendMap.AddMember(name, recvArr, alloc);
	}

	rapidjson::Value recvMap(rapidjson::kObjectType);
	for (auto& i : managedReceivers) {
		Value sendArr;
		sendArr.SetArray();
		for (auto& j : i.second) {
			Value v;
			v.SetString(StringRef(j.c_str(), j.length()));
			sendArr.PushBack(v, alloc);
		}
		Value name;
		name.SetString(StringRef(i.first.c_str(), i.first.length()));
		recvMap.AddMember(name, sendArr, alloc);
	}

	Value blockChain;
	blockChain.SetArray();
	for (auto& i : chain) {
		Value j;
		j.SetObject();

		Value d;
		d.SetString(StringRef(i.data.c_str(), i.data.length()));
		Value n(i.nonce);

		Value s;
		s.SetString(StringRef(i.sender.c_str(), i.sender.length()));

		Value r;
		r.SetString(StringRef(i.receiver.c_str(), i.receiver.length()));

		Value p;
		p.SetString(StringRef(i.previousHash.c_str(), i.previousHash.length()));

		Value diff(i.difficulty);

		j.AddMember("data", d, alloc);
		j.AddMember("sender", s, alloc);
		j.AddMember("recipient", r, alloc);
		j.AddMember("nonce", n, alloc);
		j.AddMember("difficulty", diff, alloc);
		j.AddMember("previoushash", p, alloc);
		blockChain.PushBack(j, alloc);
	}

	Value hash;
	hash.SetString(StringRef(chainhash.c_str(), chainhash.length()));

	jsonDoc.AddMember("difficulty_list", difficultyList, alloc);
	jsonDoc.AddMember("sender_map", sendMap, alloc);
	jsonDoc.AddMember("receiver_map", recvMap, alloc);
	jsonDoc.AddMember("chainhash", hash, alloc);
	jsonDoc.AddMember("blockchain", blockChain, alloc);


	StringBuffer buf;
	Writer<StringBuffer> writer(buf);
	jsonDoc.Accept(writer);

	return string(buf.GetString(), buf.GetSize());
}
void BlockChain::messUpChain(int index, string newData) {
	if (index >= chainLength())
		throw runtime_error("Index out of range");
	BlockMesserUpper::messUpData(chain[index], newData);
}
size_t BlockChain::chainLength() {
	return chain.size();
}
string BlockChain::getChainHash() {
	return chainhash;
}
void BlockChain::addBlock(Block b) {
	addBlock(b, false);
}
void BlockChain::addBlock(const string& sender, const string& receiver, int difficulty, const string& data) {
	addBlock(Block(sender, receiver, chainhash, difficulty, data));
}
void BlockChain::toFile(string filePath) {
	ofstream out(filePath);
	string data = toJson();
	out << data;
	out.close();
}
void BlockChain::printAllBlocks() {
	for (int i = 0; i < chain.size(); i++) {
		Block me = chain[i];
		cout << "Block number " << i << ": \n";
		cout << "\tSender: " << me.sender << endl;
		cout << "\tReceiver: " << me.receiver << endl;
		cout << "\tNonce: " << me.nonce << endl;
		cout << "\tPrevious hash: " << me.previousHash << endl;
		cout << "\tDifficulty: " << me.difficulty << endl;
		cout << "\tData: " << me.data << endl;
		cout << "----------------------------------------------------------------------------------------------" << endl << endl;
	}
	cout << "Current chain hash: " << chainhash << endl << endl;
}
int BlockChain::getDifficulty() {
	return chainDiff;
}
void BlockChain::setDifficulty(int newdiff) {
	chainDiff = newdiff;
}
void BlockChain::printSenders(string sender) {
	for (const auto& i : managedSenders) {
		if (i.first == sender)
			;
		else
			continue;
		cout << i.first << " -> ";
		unordered_set<string> namesAlreadyDone;
		for (const auto& j : i.second) {
			if (namesAlreadyDone.find(j) == namesAlreadyDone.end()) {
				cout << j << ", ";
				namesAlreadyDone.insert(j);
			}
		}
		cout << "\b\b" << "  " << endl;
		return;
	}
	cout << "I couldn't find that sender.\n";
}
void BlockChain::printReceivers(string receiver) {
	for (const auto& i : managedReceivers) {
		if (i.first == receiver)
			;
		else
			continue;
		cout << i.first << " <- ";
		unordered_set<string> namesAlreadyDone;
		for (const auto& j : i.second) {
			if (namesAlreadyDone.find(j) == namesAlreadyDone.end()) {
				cout << j << ", ";
				namesAlreadyDone.insert(j);
			}
		}
		cout << "\b\b" << "  " << endl;
		return;
	}
	cout << "I couldn't find that receiver.\n";
}
void BlockChain::addBlock(Block b, bool isGenesis) {
		if (!isGenesis) {
			managedSenders[b.getSender()].push_back(b.getReceiver());
			managedReceivers[b.getReceiver()].push_back(b.getSender());
		}
		Block blockToAdd(b.getSender(), b.getReceiver(), b.getNonce(), chainhash, b.getDifficulty(), b.getData());
		chainhash = blockToAdd.getBlockHash();
		chain.push_back(blockToAdd);
		diffList.push_back(blockToAdd.getDifficulty());
		chainDiff = blockToAdd.getDifficulty();
	}
void BlockChain::demo() {
	cout << "Setting chain difficulty to 3..." << endl;
	setDifficulty(3);

	cout << "Adding block...\t";
	addBlock("me", "you", getDifficulty(), "a lot of money");
	cout << "New chain hash: " << chainhash << endl;

	cout << "Setting chain difficulty to 2...\n";
	setDifficulty(2);

	cout << "Adding block...\t";
	addBlock("you", "me", getDifficulty(), "a really good grade");
	cout << "New chain hash: " << chainhash << endl;

	cout << "Setting chain difficulty to 4...\n";
	setDifficulty(4);

	cout << "Adding block...\t";
	addBlock("bradley", "me", getDifficulty(), "an eviction notice");
	cout << "New chain hash: " << chainhash << endl;

	cout << "Setting chain difficulty to 2...\n";
	setDifficulty(2);

	cout << "Adding block...\t";
	addBlock("me", "bradley", getDifficulty(), "even more money");
	cout << "New chain hash: " << chainhash << endl;

	cout << "Setting chain difficulty to 4...\n";
	setDifficulty(4);

	cout << "Adding block...\t";
	addBlock("bradley", "me", getDifficulty(), "a bachelor's");
	cout << "New chain hash: " << chainhash << endl;

	cout << "Chain length: " << chainLength() << endl;
	cout << "Chain is valid: " << (verifyChain() == -1 ? "Yes" : "No") << endl;

	cout << "Whoopsy daisy, it looks like a block just got corrupted..." << endl;

	BlockMesserUpper::messUpData(chain[2], "a bear attack");

	cout << "Chain is valid: " << (verifyChain() == -1 ? "Yes" : "No -> at " + to_string(verifyChain())) << endl;

	cout << "I should fix that...";
	fixChain();
	cout << "Done\n";

	cout << "I wonder who sent something to me? Let's see...\n";
	printSenders("me");

	cout << "That's pretty good, now who was sent something BY me?\n";
	printReceivers("me");

	cout << "Wow, me the BlockChain class can do just about everything. But what if I want to save my current block chain to load later? What on earth shall I do then?\n";

	cout << "\n" << toJson() << endl << endl;
	
	cout << "Wow, look at that.\n";
}