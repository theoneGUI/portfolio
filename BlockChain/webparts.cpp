#pragma once
#include "webparts.h"
using namespace std;

string slice(const string& s, size_t begin, size_t end) {
	auto len = s.size();
	string out = "";
	for (int i = begin; i < len; i++) {
		if (i > end)
			break;
		if (i >= begin && i <= end)
			out += s[i];
	}
	return out;
}

// Route class implementation

Route::Route()
	: route{ "" }, acceptableMethods{ {UNKNOWN} }, toReturn{ WebDoc() }, instructions{ nullptr }
{

}

Route::Route(std::string r, std::vector<methods> accept, WebDoc returnable, InstructionSet* instructions)
	: route{ r }, acceptableMethods{ accept }, toReturn{ returnable }, instructions{ instructions }
{
}

Route Route::operator=(const Route& in) {
	this->acceptableMethods = in.acceptableMethods;
	this->route = in.route;
	this->toReturn = in.toReturn;
	this->instructions = in.instructions;
	return Route(in.route, in.acceptableMethods, in.toReturn, in.instructions);
}

bool Route::operator==(const Route& in) {
	return (in.acceptableMethods == this->acceptableMethods && in.route == this->route && in.toReturn == this->toReturn);
}

bool Route::operator!=(const Route& in) {
	return !operator==(in);
}

WebDoc Route::getDoc() const {
	return toReturn;
}

bool Route::isAcceptableMethod(methods m) {
	for (const auto& i : acceptableMethods)
		if (i == m)
			return true;
	return false;
}

bool Route::hasInstructionSet() {
	return instructions != nullptr;
}

InstructionSet* Route::instructionSet() {
	return instructions;
}

// Document class implementation

WebDoc::WebDoc()
	: documentContent{ "" }, loadsFromFile{ false }, loadsFileIntoMemory{ false }
{}

WebDoc::WebDoc(std::string content)
	: documentContent{ content }, loadsFromFile{ false }, loadsFileIntoMemory{ false }
{
}

WebDoc::WebDoc(webpath path, bool loadIntoMemory) {
	filePath = path;
	loadsFromFile = true;
	loadsFileIntoMemory = loadIntoMemory;
	if (loadIntoMemory) {
		ifstream file(filePath);
		while (!file.eof()) {
			auto gotten = file.get();
			if (gotten == 'ÿ')
				break;
			this->documentContent += (char)gotten;
		}
		file.close();
	}
}

std::string WebDoc::getContent() const {
	if (loadsFromFile && !loadsFileIntoMemory) {
		ifstream file(filePath);
		string fileContents;
		while (!file.eof()) {
			auto gotten = file.get();
			if (gotten == 'ÿ')
				break;
			fileContents += (char)gotten;
		}
		file.close();
		return fileContents;
	}
	return documentContent;
}

const char* WebDoc::c_str() const {
	return this->documentContent.c_str();
}

size_t WebDoc::length() const {
	return this->documentContent.length();
}

bool WebDoc::operator==(const WebDoc& comp) const {
	return comp.documentContent == this->documentContent;
}

bool WebDoc::operator!=(const WebDoc& comp) const {
	return !operator==(comp);
}


// Request class implementation

Request::Request(const char* raw) {
	string strRep = raw;
	auto lines = getLines(strRep);
	this->headers = parseHeaders(strRep);
	this->httpVersion = parseHTTP(strRep);
	this->verb = parseVerb(strRep);
	this->route = parseRoute(strRep);
	this->body = parseBody(strRep);
}

string Request::getRoute() const {
	return this->route;
}

unordered_map<string, string> Request::getHeaders() const {
	return this->headers;
}

string Request::getBody() const {
	return this->body;
}

pair<string, string> Request::getHttp() const {
	return this->httpVersion;
}

vector<string> Request::getLines(const string& input) {
	vector<string> out;
	size_t offset = 0;
	while (true) {
		size_t foundAt = input.find_first_of("\r\n", offset);
		if (foundAt == string::npos)
			break;
		out.push_back(slice(input, offset, foundAt));
		offset = foundAt + 1;
	}
	return out;
}

unordered_map<string, string> Request::parseHeaders(const string& lines) {
	vector<string> linesVec;
	size_t offset = 0;
	size_t bodyBreak = lines.find("\r\n\r\n");
	while (true) {
		size_t foundAt = lines.find_first_of("\r\n", offset);
		if (foundAt == string::npos || foundAt >= bodyBreak)
			break;
		string sliced = slice(lines, offset, foundAt);
		linesVec.push_back(sliced);
		offset = foundAt + 1;
	}


	unordered_map<string, string> out;
	for (const auto& i : linesVec) {
		size_t foundAt = i.find(": ");
		if (foundAt != string::npos) {
			string key = slice(i, 0, foundAt - 1);
			string val = slice(i, foundAt + 2, i.length());
			out.insert(pair<string, string>(key, val));
		}
	}
	return out;
}

pair<string, string> Request::parseHTTP(const string& input) {
	pair<string, string> out;
	size_t newline = input.find("\r\n");
	size_t httpAt = input.find("HTTP");
	bool isHttp = httpAt != string::npos;
	if (!isHttp) return out;
	if (newline == string::npos) return out;
	string firstLine = slice(input, 0, newline);
	out.first = slice(firstLine, httpAt, firstLine.find("/", httpAt) - 1);
	string toFloat = slice(firstLine, firstLine.find("/", httpAt) + 1, firstLine.length());
	out.second = toFloat;
	return out;
}

methods Request::parseVerb(const string& input) {
	size_t newline = input.find("\r\n");
	bool isHttp = input.find("HTTP") != string::npos;
	if (!isHttp) return methods::UNKNOWN;
	if (newline == string::npos) return methods::UNKNOWN;
	string firstLine = slice(input, 0, newline);
	string method = slice(firstLine, 0, firstLine.find(" ") - 1);
	if (method == "GET")
		return methods::GET;
	else if (method == "POST")
		return methods::POST;
	else if (method == "PUT")
		return methods::PUT;
	else if (method == "DELETE")
		return methods::DEL;
	else
		return methods::UNKNOWN;
}

string Request::parseRoute(const string& input) {
	string out;
	size_t newline = input.find("\r\n");
	size_t httpAt = input.find("HTTP");
	bool isHttp = httpAt != string::npos;
	if (!isHttp) return out;
	if (newline == string::npos) return out;
	string firstLine = slice(input, 0, newline);

	size_t firstSpace = firstLine.find(' ', 0);
	size_t secondSpace = firstLine.find(' ', firstSpace + 1);
	out = slice(firstLine, firstSpace + 1, secondSpace - 1);
	return out;
}

string Request::parseBody(const string& input) {
	string out;
	size_t bodybegin = input.find("\r\n\r\n");

	if (bodybegin == string::npos) return out;

	out = slice(input, bodybegin + 2, input.length() - 1);
	return out;
}

methods Request::getVerb() const { return verb; }

// Response class implementation

Response::Response(short statusCode, string statusMsg, const std::unordered_map<std::string, std::string>& headers, WebDoc doc)
	: statusCode{ statusCode }, headers{ headers }, document{ doc }, statusMsg{ statusMsg }
{
	buildResponse();
}

std::string Response::buildResponse() {
	string resp = "HTTP/1.0 ";
	resp += to_string(statusCode);
	resp += " " + statusMsg;
	resp += "\r\n";
	if (headers.find("Content-Type") == headers.end()) {
		resp += "Content-Type: text/html; charset=\"ascii\"\r\n";
	}
	for (const auto& i : headers) {
		resp += i.first + ": " + i.second + "\r\n";
	}
	resp += "\r\n";
	resp += document.getContent();
	this->response = resp;
	return resp;
	//HTTP/1.0 200 OK\r\nContent-Type: text/html; charset=\"ascii\"\r\n\r\n
}

std::string Response::buildResponse(short statusCode, const std::unordered_map<std::string, std::string>& headers, WebDoc doc) {
	string resp = "HTTP/1.0 ";
	resp += statusCode;
	resp += "\r\n";
	resp += "Content-Type: text/html; charset=\"ascii\"";
	for (const auto& i : headers) {
		resp += i.first + ": " + i.second + "\r\n";
	}
	resp += "\r\n";
	resp += doc.getContent();
	return resp;
}

std::string Response::getResponse()
{
	if (document.loadsFromFile && !document.loadsFileIntoMemory) {
		auto data = document.getContent();
		return data;
	}
	else
		return response;
}

void Response::addHeader(std::string key, std::string val) {
	if (headers.find(key) == headers.end()) {
		headers[key] = val;
	}
}