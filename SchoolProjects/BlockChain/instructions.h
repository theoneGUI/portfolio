#pragma once
#include <string>
#include "httperror.h"
#include "webparts.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/error/error.h"
#include <ctime>
#include "BlockChain.h"
#include "worldending.h"

using namespace rapidjson;

using std::string;
using std::to_string;


// Base class from which all Instructions for dynamic routes derive
class InstructionSet {
public:
	// No contructor, only required method is what to execute when a request comes in.
	// @returns a Response object for httpd to serialize
	virtual Response execute(const Request& request) = 0;
};

// Gets the current UNIX timestamp and returns it to the user
class Instructions_Time : public InstructionSet {
public:
	Response execute(const Request& request) override {
		Response r(200, "OK", {}, WebDoc("time: " + to_string(time(nullptr))));
		return r;
	}
};

// Demonstration of how you can perform different actions on the same route with different request methods
class Instructions_Mult : public InstructionSet {
public:
	Response execute(const Request& request) override {
		string docContent;
		short code = 200;

		if (request.getVerb() == GET) {
			docContent = "You made a GET request";
		}
		else if (request.getVerb() == POST) {
			docContent = "You made a POST request";
		}

		Response r(code, "OK", {}, WebDoc(docContent));
		return r;
	}
};

// This instruction set controls what happens to the block chain
class Instructions_Actions : public InstructionSet {
public:
	Instructions_Actions(BlockChain* bc)
		: b{ bc } {
	}

	Response execute(const Request& request) override {
		string docContent;
		short code = 200;
		auto body = request.getBody();
		auto headers = request.getHeaders();

		if (headers["Content-Type"].find("application/json") == string::npos)
			throw HTTP400("JSON Required");

		Document json;
		json.Parse(body.c_str());
		Document::AllocatorType& alloc = json.GetAllocator();

		if (json.HasParseError())
			throw HTTP400("Valid JSON required");

		string actionStr;
		int actionInt;
		try {
			actionStr = json["action"].GetString();
			actionInt = stoi(actionStr);
		}
		catch (...) {
			throw HTTP400("Missing or invalid parameters");
		}

		Document jsonOut;
		Document::AllocatorType& allocOut = jsonOut.GetAllocator();
		jsonOut.SetObject();
		bool needsFurtherStudy = false;

		string strToRef;
		if (actionInt == 1) {
			if (!json.HasMember("send") || json["send"].IsNull() ||
				!json.HasMember("recv") || json["recv"].IsNull() ||
				!json.HasMember("data") || json["data"].IsNull()
				)
				throw HTTP400("Missing required data or malformed request");
			string sender = json["send"].GetString();
			string recv = json["recv"].GetString();
			string dat = json["data"].GetString();
			b->addBlock(sender, recv, b->getDifficulty(), dat);
			jsonOut.AddMember("message", Value("Added block to chain."), allocOut);
		}
		else if (actionInt == 2) {
			int status = b->verifyChain();
			if (status != -1) {
				strToRef = "The blockchain is invalid. The invalidity begins at block " + to_string(status) + " of the chain.";
				Value msgValue;
				msgValue.SetString(StringRef(strToRef.c_str()));
				jsonOut.AddMember("message", msgValue, allocOut);
			}
			else {
				Value msgValue = Value("Blockchain was found to be valid.");
				jsonOut.AddMember("message", msgValue, allocOut);
			}
		}
		else if (actionInt == 3) {
			jsonOut.Parse(b->toJson().c_str());
			jsonOut.RemoveMember("difficulty_list");
			jsonOut.RemoveMember("sender_map");
			jsonOut.RemoveMember("receiver_map");

			jsonOut.AddMember("message", Value("Displaying blockchain..."), allocOut);
		}
		else if (actionInt == 4) {
			/*cout << "You have chosen to corrupt the block chain. I'll need information for that.\n";
			cout << "You first need to choose a block to corrupt. There are " << b->chainLength() << " blocks to choose from.\n Enter a number between 0 and " << b->chainLength() - 1 << " to select the block.\n > ";
			while (true) {
				input = getIntFromUser();
				if (input >= 0 && input <= (b->chainLength() - 1))
					break;
				cout << "Enter a number within the specified bounds.\n";
			}         Inquiry  */
			if (!json.HasMember("newdata") || json["newdata"].IsNull() ||
				!json.HasMember("index") || json["index"].IsNull()
				)
				throw HTTP400("Missing required data or malformed request");
			string newdata = json["newdata"].GetString();
			int intDex = json["index"].GetInt();
			b->messUpChain(intDex, newdata);
			jsonOut.AddMember("message", Value("The chain is corrupt."), allocOut);
		}
		else if (actionInt == 5) {
			b->fixChain();
			jsonOut.AddMember("message", Value("Chain has been repaired."), allocOut);
		}
		else if (actionInt == 6) {
			strToRef = b->toJson();
			Value v = Value(StringRef(strToRef.c_str()));
			jsonOut.AddMember("data", v, allocOut);
		}
		else if (actionInt == 7) {
			// "You have chosen to change your difficulty. Enter new difficulty between 1 and 7 (please don't do 7)";
			if (!json.HasMember("newdiff") || json["newdiff"].IsNull()
				)
				throw HTTP400("Missing required data or malformed request");
			int newdiff = json["newdiff"].GetInt();
			b->setDifficulty(newdiff);
			Value v;
			strToRef = string("The chain difficulty is now " + to_string(newdiff));
			v.SetString(StringRef(strToRef.c_str()));
			jsonOut.AddMember("message", v, allocOut);
		}
		else if (actionInt == 8) {
			if (!json.HasMember("sender") || json["sender"].IsNull() || !json["sender"].IsString())
				throw HTTP400("Missing required data or malformed request");

			string sender = json["sender"].GetString();
			jsonOut.Parse(b->toJson().c_str());
			jsonOut.RemoveMember("difficulty_list");
			jsonOut.RemoveMember("blockchain");
			jsonOut.RemoveMember("receiver_map");
			jsonOut.RemoveMember("chainhash");

			if (jsonOut["sender_map"].HasMember(sender.c_str())) {
				Value obj;
				obj.SetObject();
				obj.AddMember("receivers", jsonOut["sender_map"][sender.c_str()], allocOut);
				obj.AddMember("sender", json["sender"], allocOut);
				jsonOut.AddMember("data", obj, allocOut);
			}
			else {
				jsonOut.AddMember("data", Value(kNullType), allocOut);
			}

			jsonOut.AddMember("message", Value("Displaying sender information..."), allocOut);
		}
		else if (actionInt == 9) {
			if (!json.HasMember("recip") || json["recip"].IsNull() || !json["recip"].IsString())
				throw HTTP400("Missing required data or malformed request");

			string recip = json["recip"].GetString();
			jsonOut.Parse(b->toJson().c_str());
			jsonOut.RemoveMember("difficulty_list");
			jsonOut.RemoveMember("blockchain");
			jsonOut.RemoveMember("sender_map");
			jsonOut.RemoveMember("chainhash");

			if (jsonOut["receiver_map"].HasMember(recip.c_str())) {
				Value obj;
				obj.SetObject();
				obj.AddMember("senders", jsonOut["receiver_map"][recip.c_str()], allocOut);
				obj.AddMember("receiver", json["recip"], allocOut);
				jsonOut.AddMember("data", obj, allocOut);
			}
			else {
				jsonOut.AddMember("data", Value(kNullType), allocOut);
			}

			jsonOut.AddMember("message", Value("Displaying sender information..."), allocOut);
		}
		else if (actionInt == 10) {
			throw SigTerm();
		}
		else if (actionInt == 11) {
			throw SigHalt();
		}
		else {
			throw HTTP400("Action does not exist");
		}

		StringBuffer buf;
		Writer<StringBuffer> writer(buf);
		jsonOut.Accept(writer);

		docContent = string(buf.GetString(), buf.GetSize());
		Response r(code, "OK", { {"Content-Type","application/json"} }, WebDoc(docContent));
		return r;
	}

private:
	BlockChain* b;
};

// This instruction set returns required information to the browser when it needs to inform the user about something in the block chain.
class Instructions_Inquiry : public InstructionSet {
public:
	Instructions_Inquiry(BlockChain* bc)
		: b{ bc } {
	}

	Response execute(const Request& request) override {
		string docContent;
		short code = 200;
		auto body = request.getBody();
		auto headers = request.getHeaders();

		if (headers["Content-Type"].find("application/json") == string::npos)
			throw HTTP400("JSON Required");

		Document json;
		json.Parse(body.c_str());
		Document::AllocatorType& alloc = json.GetAllocator();

		if (json.HasParseError())
			throw HTTP400("Valid JSON required");

		string actionStr;
		int actionInt;
		try {
			actionStr = json["action"].GetString();
			actionInt = stoi(actionStr);
		}
		catch (...) {
			throw HTTP400("Missing or invalid parameters");
		}

		Document jsonOut;
		Document::AllocatorType& allocOut = jsonOut.GetAllocator();
		jsonOut.SetObject();

		if (actionInt == 4) {
			jsonOut.AddMember("indexUpperBound", Value(b->chainLength() - 1), allocOut);
			jsonOut.AddMember("chainLength", Value(b->chainLength()), allocOut);
			jsonOut.AddMember("indexLowerBound", Value(0), allocOut);
		}
		else if (actionInt == 7) {
			jsonOut.AddMember("diffUpperBound", Value(7), allocOut);
			jsonOut.AddMember("diffLowerBound", Value(1), allocOut);
		}
		else {
			throw HTTP400("No inquiry information exists for this action.");
		}

		StringBuffer buf;
		Writer<StringBuffer> writer(buf);
		jsonOut.Accept(writer);

		docContent = string(buf.GetString(), buf.GetSize());
		Response r(code, "OK", { {"Content-Type","application/json"} }, WebDoc(docContent));
		return r;
	}
private:
	BlockChain* b;
};

// This instruction set returns the length of the block chain
class Instructions_Diff : public InstructionSet {
public:
	Instructions_Diff(BlockChain* bc)
		: b{ bc } {
	}

	Response execute(const Request& request) override {
		string docContent;
		short code = 200;
		auto body = request.getBody();
		auto headers = request.getHeaders();

		Document jsonOut;
		jsonOut.SetObject();
		jsonOut.AddMember("difficulty", Value(b->getDifficulty()), jsonOut.GetAllocator());

		StringBuffer buf;
		Writer<StringBuffer> writer(buf);
		jsonOut.Accept(writer);

		docContent = string(buf.GetString(), buf.GetSize());
		Response r(code, "OK", { {"Content-Type","application/json"} }, WebDoc(docContent));
		return r;
	}
private:
	BlockChain* b;
};