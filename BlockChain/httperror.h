#pragma once
#include <stdexcept>
#include <string>
#include "webparts.h"

using std::string;

// Base class for HTTP Errors
// HTTP Errors are to be thrown and caught as their own entity so that they don't cause crashes; instead they give a specialized response to the requesting browser about what went wrong
// Each error class has 2 options for instantiation; a general error and an error with a message
class HTTPError : public std::runtime_error {
public:
	HTTPError() : std::runtime_error("HTTP Error") {

	}
	virtual Response getResponse() {
		return Response(500, "INTERNAL SERVER ERROR", {}, WebDoc("<html><h1>5XX Internal Server Error</h1></html>"));
	}
};

// Standard HTTP 404 not found error
class HTTP404 : public HTTPError {
public:
	HTTP404()
		: HTTPError(), document{ WebDoc("<html><h1>404 Not Found</h1></html>") }
	{}
	HTTP404(string message) 
		: HTTPError(), msg{message}, document{ WebDoc("<html><h1>404 Not Found</h1><p>" + message + "</p></html>") }
	{
	}
	Response getResponse() override {
		return Response(404, "NOT FOUND", {}, document);
	}
private:
	WebDoc document;
	string msg;
};

// Standard HTTP 406 invalid credentials error (not useful for this project, but possibly others)
class HTTP406 : public HTTPError {
public:
	HTTP406() 
		: HTTPError(), document{ WebDoc("<html><h1>406 Unauthorized</h1></html>") }
	{
	}
	HTTP406(string message)
		: HTTPError(), msg{ message }, document{ WebDoc("<html><h1>406 Unauthorized</h1><p>" + message + "</p></html>") }
	{
	}
	Response getResponse() override {
		return Response(406, "NOT AUTORIZED", {}, document);
	}
private:
	WebDoc document;
	string msg;
};

// Standard HTTP 400 bad request error
class HTTP400 : public HTTPError {
public:
	HTTP400()
		: HTTPError(), document{ WebDoc("<html><h1>400 Bad Request</h1></html>") }
	{
	}
	HTTP400(string message)
		: HTTPError(), msg{ message }, document{ WebDoc("<html><h1>400 Bad Request</h1><p>" + message + "</p></html>") }
	{
	}
	Response getResponse() override {
		return Response(400, "BAD REQUEST", {}, document);
	}
private:
	WebDoc document;
	string msg;

};

// Standard HTTP 500 internal server error
class HTTP500 : public HTTPError {
public:
	HTTP500()
		: HTTPError(), document{ WebDoc("<html><h1>500 Internal Server Error</h1></html>") }
	{
	}
	HTTP500(string message)
		: HTTPError(), msg{ message }, document{ WebDoc("<html><h1>500 Internal Server Error</h1><p>" + message + "</p></html>") }
	{
	}
	Response getResponse() override {
		return Response(500, "INTERNAL SERVER ERROR", {}, document);
	}
private:
	WebDoc document;
	string msg;
};