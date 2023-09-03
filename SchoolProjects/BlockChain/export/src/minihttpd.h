#pragma once
#define MAX_MESSAGE_LENGTH 8096
#include "plibcpp.h"
#include "httperror.h"
#include <string>
#include <iostream>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <fstream>
#include <exception>
#include "webparts.h"

// minihttpd class which inherits from plib_user to avoid memory issues
// see the plibcpp.h file for details on why
class minihttpd : public plib_user
{
public:
	// Give the server an address and port to bind to on instantiation
	minihttpd(std::string addressToBindTo, int portToBindTo);
	// Run once and exit (respond to one request and die (admittedly not very useful))
	void run();
	// Keep running after that first request. More useful for just about everything
	void keepRunning();
	// Tell the server how to respond to requests to different routes
	bool registerRoute(const std::string& routeName, const WebDoc& doc, const std::vector<methods> acceptable, InstructionSet* instructions);
protected:
	bool isRunning;
	// This method is antiquated because it's inherited from another project which was muli-threaded and the .run() member method was spoken for
	int privateRun(std::string interfaceToBindTo, int portToBindTo, bool keepRunning = false);
	std::string interfaceToBindTo;
	int portToBindTo;
	std::string rootDirectory;
	std::unordered_map<std::string, Route> routes;
};
