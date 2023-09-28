#pragma once
#include <string>
#include "httperror.h"
#include "webparts.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/error/error.h"
#include <ctime>
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

class Instructions_NotReallyIntensiveTask : public InstructionSet {
public:
	Response execute(const Request& request) override {
		string docContent;
		short code = 200;

		Sleep(5000);
		docContent = "You made a GET request";

		Response r(code, "OK", {}, WebDoc(docContent));
		return r;
	}
};