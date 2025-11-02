#include "minihttpd.h"
using namespace std;
#include "PThread.h"
#include "instructions.h"
#include "worldending.h"

// httpd class implementation

minihttpd::minihttpd(string addressToBindTo, int portToBindTo)
	: interfaceToBindTo{ addressToBindTo }, portToBindTo{ portToBindTo }, isRunning{ false }
{ 
}

void minihttpd::run()
{ 
	// run once
	if (isRunning) // and don't try to open the server if it's already running
		return;
	isRunning = true;
	int code = privateRun(this->interfaceToBindTo, this->portToBindTo);
	isRunning = false;
}

void minihttpd::keepRunning() {
	// run forever
	if (isRunning) // and don't try to open the server if it's already running
		return;
	isRunning = true;
	int code = privateRun(this->interfaceToBindTo, this->portToBindTo, true);
	isRunning = false;
}

int minihttpd::privateRun(string interfaceToBindTo, int portToBindTo, bool keepRunning) {
	PSocketAddress* addr;
	PSocket* sock;
	bool termThrown = false;
	cout << "Starting up minihttpd server on port " << portToBindTo << "..." << endl;
	// Binding socket to local host (we are a server, the appropriate port.  Typically this will always be a localhost port, because we are going to listen to this)
	if ((addr = p_socket_address_new(interfaceToBindTo.c_str(), portToBindTo)) == NULL)
	{
		cout << "Failed to bind to localhost port 80. Could not continue. Strike enter to exit.\n";
		cin;
		return 1;
	}
	// Create socket
	if ((sock = p_socket_new(P_SOCKET_FAMILY_INET, P_SOCKET_TYPE_STREAM, P_SOCKET_PROTOCOL_TCP, NULL)) == NULL)
	{
		// Failed to create socket -- cleanup
		cout << "Failed to create listener socket. Could not continue. Strike enter to exit.\n";
		cin;
		p_socket_address_free(addr);
		return 2;
	}

	// Bind to local host (server) socket
	if (!p_socket_bind(sock, addr, FALSE, NULL))
	{
		// Couldn't bind socket, cleanup
		cout << "Could not bind to listener socket. Could not continue. Strike enter to exit.\n";
		cin;
		p_socket_address_free(addr);
		p_socket_free(sock);
		return 3;
	}

	// Listen for incoming connections on localhost port specified
	if (!p_socket_listen(sock, NULL))
	{
		cout << "Could not start listening for connections. (Firewall?) Could not continue. Strike enter to exit.\n";
		cin;
		// Couldn't start listening, cleanup
		p_socket_address_free(addr);
		p_socket_close(sock, NULL);
		return 4;
	}
	p_socket_set_buffer_size(sock, P_SOCKET_DIRECTION_SND, MAX_MESSAGE_LENGTH, NULL);
	p_socket_set_buffer_size(sock, P_SOCKET_DIRECTION_RCV, MAX_MESSAGE_LENGTH, NULL);
	p_socket_set_keepalive(sock, false);
	p_socket_set_blocking(sock, true);

	char* buffer = new char[MAX_MESSAGE_LENGTH + 1]; // Supports messages up to max length (plus null character that terminates the string, since we're sending text)
	for (int i = 0; i < MAX_MESSAGE_LENGTH; i++) {
		//  clear out our buffer
		buffer[i] = 0;
	}

	cout << "Started minihttpd server... ready for connections\n";
	while (keepRunning) {
		// Blocks until connection accept happens
		PSocket* con = p_socket_accept(sock, NULL);
		p_socket_set_timeout(con, 1000);
		setNewThreadUp(con);
		// Once a connection happens, receive all possible data within the max permitted transmission size
		pssize sizeOfRecvData = p_socket_receive(con, buffer, MAX_MESSAGE_LENGTH, NULL);
		buffer[MAX_MESSAGE_LENGTH] = '\0';
		// Parse the request from the browser
		Request r(buffer);
		// If it's not recognzied to be a HTTP request, close the connection because we don't care about anything except HTTP
		if (r.getHttp().first != "HTTP") {
			p_socket_close(con, NULL);
			continue;
		}
		
		// Get relevant data from the request into local variables
		auto headers = r.getHeaders();
		auto http = r.getHttp();
		auto verb = r.getVerb();
		auto route = r.getRoute();

		try {
			// If our route given is found, act accordingly
			if (routes.find(r.getRoute()) != routes.end()) {
				auto acceptable = routes[route].isAcceptableMethod(verb);
				// check if our given route to look up supports the given http method
				if (!acceptable) {
					// If not acceptable, throw the browser a 400
					throw HTTP400("That method is not supported on this page.");
				}

				// If acceptable, continue going
				WebDoc doc = routes[route].getDoc();
				if (routes[route].hasInstructionSet()) {
					// If the specified route has some instructions it's supposed to carry out (like making a new block in the chain), do that
					Response resp = routes[route].instructionSet()->execute(r);
					resp.addHeader("X-Powered-By", "minihttpd");
					auto response = resp.getResponse();
					p_socket_send(con, response.c_str(), response.length(), NULL);
				}
				else {
					// Respond with static pages appropriately
					Response resp(200, "OK", { std::pair<std::string, std::string>("X-Powered-By", "minihttpd") }, doc);
					auto response = resp.getResponse();
					p_socket_send(con, response.c_str(), response.length(), NULL);
				}

			}
			// If we don't know what route they're talking about, return a 404
			else {
				throw HTTP404("We can't find the page you're looking for.");
			}
		}
		catch (HTTPError& e) {
			// If a known http error is thrown, handle it
			Response r = e.getResponse();
			auto response = r.getResponse();
			p_socket_send(con, response.c_str(), response.length(), NULL);
		}
		// in the case of a user requested server shutdown, do so gracefully and clean up after yourself
		catch (SigTerm&) {
			keepRunning = false;
			// exit and throw a sigterm to the driver once cleanup is complete
			termThrown = true;
		}
		catch (SigHalt&) {
			// just exit
			keepRunning = false;
		}
		// If something goes wrong that we don't know how to handle, throw a 500 to the browser
		catch (...) {
			HTTP500 x;
			string response = x.getResponse().getResponse();
			p_socket_send(con, response.c_str(), response.length(), NULL);
		}
		p_socket_close(con, NULL);
		for (int i = 0; i < MAX_MESSAGE_LENGTH; i++) {
			//  clear out our buffer
			buffer[i] = 0;
		}
	}
	// Release buffer memory
	delete[] buffer;
	// Close down server
	p_socket_close(sock, NULL);
	p_socket_address_free(addr);
	// Bubble the termination signal to the next stack frame
	if (termThrown == true)
		throw SigTerm();
	return 0;
}

bool minihttpd::registerRoute(const string& routeName, const WebDoc& doc, const vector<methods> acceptable, InstructionSet* instructions) {
	// if the specified route already exists, return false to let the user know that the route creation failed
	if (routes.find(routeName) == routes.end()) {
		// if route is open, make the route object part of our server's route map
		routes[routeName] = Route(routeName, acceptable, doc, instructions);
		return true;
	}
	else
		return false;
}

