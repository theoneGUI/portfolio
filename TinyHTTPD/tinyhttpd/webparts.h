#pragma once
#include <unordered_map>
#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>

// Utility function to perform python-like slicing on a string
std::string slice(const std::string&, size_t, size_t);

// Forward declarations for friend class directives
class InstructionSet;
class Route;
class Response;
class WebDoc;
class Request;

// A custom C-style string type to tell a C-style string from someone intentionally trying to pass in a file path to load a WebDoc from
typedef const char* webfile;
typedef const char* webdir;

// The different HTTP methods represented as chars
// A design choice I'm not sure I agree with anymore
enum methods {
	GET = 'G',
	POST = 'P',
	PUT = 'U',
	DEL = 'D',
	UNKNOWN = '?'
};

// This class houses a web document stored as a string or is able to be loaded from a file on disk
// When instantiating from a file, you can either choose to load it from the disk every time or cache in memory
class WebDoc {
public:
	WebDoc();
	WebDoc(std::string content);
	WebDoc(webfile path, bool loadIntoMemory, const std::string&);
	virtual std::string getContent() const;
	virtual const char* c_str() const;
	virtual size_t length() const;
	virtual std::string mime() const;
	bool operator==(const WebDoc& comp) const;
	bool operator!=(const WebDoc& comp) const;
	friend class Response;

protected:
	std::string documentContent;
	bool loadsFromFile;
	bool loadsFileIntoMemory;
	std::string filePath;
	std::string mimeType;
};

class WebDir {
public:
	WebDir();
	WebDir(webdir path);
	virtual std::string getContent(std::string relative);
	virtual const char* c_str() const;
	virtual size_t length() const;
	bool operator==(const WebDir& comp) const;
	bool operator!=(const WebDir& comp) const;
	friend class Response;

protected:
	std::string documentContent;
	std::string dirPath;
};

// Reponse takes different information and serializes it into a full HTTP response to return to the requesting browser
// @param statusCode is the number representation of the HTTP status you want to return (e.g. 200)
// @param statusMsg is the text representation of the HTTP status you want to return (e.g. OK)
// @param headers is a map of headers to put into the request. You don't have to specify content type if it's HTML; default content type is HTML
// @param doc is a WebDoc containing the text or instruction set required to make a sensible thing for the user to see or interact with
class Response {
public:
	Response(short statusCode, std::string statusMsg, const std::unordered_map<std::string, std::string>& headers, WebDoc doc);
	static std::string buildResponse(short statusCode, const std::unordered_map<std::string, std::string>& headers, WebDoc doc);
	void addHeader(std::string key, std::string val);
	std::string getResponse();
private:
	short statusCode;
	std::string statusMsg;
	std::unordered_map<std::string, std::string> headers;
	WebDoc document;
	std::string response;
	std::string buildResponse();
	bool buildComplete = false;
};

// Request takes a raw string representation of an HTTP request from a browser and parses it into something the program can process reliably and quickly
class Request {
public:
	// Instantiate from raw HTTP request string and process it accordingly
	Request(const char* raw);
	// giving getter methods for relevant parts of the request (no, cookies are not supported)
	std::unordered_map<std::string, std::string> getHeaders() const;
	std::string getBody() const;
	std::pair<std::string, std::string> getHttp() const;
	std::string getRoute() const;
	methods getVerb() const;
private:
	methods verb;
	std::unordered_map<std::string, std::string> headers;
	std::string body;
	std::pair<std::string, std::string> httpVersion;
	std::string route;
	friend class Route;

	std::vector<std::string> getLines(const std::string&);
	std::unordered_map<std::string, std::string> parseHeaders(const std::string&);
	std::pair<std::string, std::string> parseHTTP(const std::string&);
	std::string parseRoute(const std::string&);
	methods parseVerb(const std::string&);
	std::string parseBody(const std::string& input);
};

const WebDoc nullDoc = WebDoc("");

// Route gives minihttpd ways to direct the incoming requests
// Each Route has:
// @param r: a route name (e.g. /about)
// @param accept: vector of methods to accept (else throw a 400 to the browser) (e.g. GET)
// @param returnable: a WebDoc to return to the user on request (static; not returned if instructions is not nullptr)
// @param instructions: a pointer to an InstructionSet object (dynamic content) (nullptr to return the WebDoc)
class Route {
public:
	Route(std::string r, std::vector<methods> accept, WebDoc returnable, InstructionSet* instructions);
	Route();
	Route operator=(const Route& in);
	bool operator==(const Route& in);
	bool operator!=(const Route& in);
	WebDoc getDoc() const;
	bool isAcceptableMethod(methods m);
	bool hasInstructionSet();
	InstructionSet* instructionSet();
private:
	std::string route;
	std::vector<methods> acceptableMethods;
	WebDoc toReturn;
	InstructionSet* instructions;
};
