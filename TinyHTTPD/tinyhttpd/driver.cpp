#include "httperror.h"
#include "minihttpd.h"
#include "worldending.h"
#include "instructions.h"
#include "webparts.h"
#include <iostream>
#include <ctime>

using namespace std;



void webserver() {
	minihttpd http("0.0.0.0", 80);

	{
		webfile rootDocument = "webroot/index.html";
		http.registerRoute("/", WebDoc(rootDocument, false, "text/html"), {GET}, nullptr);
	}
	{
		webfile rootDocument = "webroot/genericPfp.png";
		http.registerRoute("/png", WebDoc(rootDocument, false, "image/png"), {GET}, nullptr);
	}


	InstructionSet* timeThing = new Instructions_Time();
	http.registerRoute("/reactive", WebDoc(), { GET }, timeThing);

	InstructionSet* intensiveThing = new Instructions_NotReallyIntensiveTask();
	http.registerRoute("/intense", WebDoc(), { GET }, intensiveThing);

	http.keepRunning();
	delete intensiveThing;
	delete timeThing;
}

int main() {
	try {
		webserver();
	}
	catch (SigTerm&) {
		;
	}
	catch (SigHalt&) {
		;
	}
}
