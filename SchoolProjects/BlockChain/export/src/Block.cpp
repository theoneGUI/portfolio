#include "Block.h"

// Block class implementation

Block::Block(const string& sender, const string& receiver, size_t nonce, const string& previousHash, int difficulty, const string& data)
	: sender{ sender }, receiver{ receiver }, nonce{ nonce }, previousHash{ previousHash }, difficulty{ difficulty }, data{ data }
{}
Block::Block(const string& sender, const string& receiver, const string& previousHash, int difficulty, const string& data)
	: sender{ sender }, receiver{ receiver }, nonce{ 0 }, previousHash{ previousHash }, difficulty{ difficulty }, data{ data }
{
	nonceProvided = false;
}
string Block::getSender() {
	return sender;
}
string Block::getReceiver() {
	return receiver;
}
string Block::getPreviousHash() {
	return previousHash;
}
string Block::getData() {
	return data;
}
size_t Block::getNonce() {
	if (!nonceProvided && !nonceFigured)
		figureOutNonce();
	return nonce;
}
int Block::getDifficulty() {
	return difficulty;
}
string Block::figureOutNonce() {
	if (!nonceProvided)
		nonceFigured = true;
	return hashBlock(difficulty);
}
string Block::getBlockHash() {
	string messMeUp = getHashable(getNonce());
	//  yes this was stolen from the example file on canvas
	vector<unsigned char> hash(picosha2::k_digest_size);
	picosha2::hash256(messMeUp.begin(), messMeUp.end(), hash.begin(),
		hash.end());
	string hex_str = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
	messMeUp = getHashable(nonce);
	std::vector<unsigned char> hash3(picosha2::k_digest_size);
	picosha2::hash256(messMeUp.begin(), messMeUp.end(), hash3.begin(),
		hash3.end());
	hex_str = picosha2::bytes_to_hex_string(hash3.begin(), hash3.end());
	return hex_str;
}

string Block::hashBlock(int difficulty) {
	size_t nonce = 0;
	string messMeUp = getHashable(nonce);

	//  yes this was stolen from the example file on canvas
	vector<unsigned char> hash(picosha2::k_digest_size);
	picosha2::hash256(messMeUp.begin(), messMeUp.end(), hash.begin(),
		hash.end());
	string hex_str = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
	while (starts_with_n_zeros(hex_str, difficulty) == false) {
		nonce += 1;
		messMeUp = getHashable(nonce);
		std::vector<unsigned char> hash3(picosha2::k_digest_size);
		picosha2::hash256(messMeUp.begin(), messMeUp.end(), hash3.begin(),
			hash3.end());
		hex_str = picosha2::bytes_to_hex_string(hash3.begin(), hash3.end());
	}
	this->nonce = nonce;
	return hex_str;
}
bool Block::starts_with_n_zeros(string s, int n) {
	if (s.length() < n) {
		return false;
	}
	string zeros(n, '0');
	return s.substr(0, n) == zeros;
}
string Block::getHashable(size_t nonce) {
	return data + previousHash + to_string(nonce) + sender + receiver;
}
string Block::getHashable() {
	return getHashable(this->nonce);
}

// BlockMesserUpper class implementation

void BlockMesserUpper::messUpData(Block& x, const string& newData) {
	x.data = newData;
	// force hash to recalculate on next request of block hash
	x.nonceFigured = false;
	x.nonceProvided = false;
}