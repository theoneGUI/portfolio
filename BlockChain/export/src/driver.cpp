
// if you're having trouble building because plibsys is not installed, set this to false
// It will disable web functionality, but the project will build and be testable
#define WEB true


#if WEB == true
#include "httperror.h"
#include "minihttpd.h"
#include "worldending.h"
#include "instructions.h"
#include "webparts.h"
#endif

#include "BlockChain.h"
#include "Block.h"

#include <iostream>
#include <ctime>


// the blockchain object is a pointer so I can pass it around to whoever I want whenever I want in and out of scope and through threads and stuff if I wanted to
BlockChain* b = nullptr;


using namespace std;

// Set up the minihttpd server on command and put everything where it should be
#if WEB == true
void webserver() {
	// start a minihttpd server listening on all interfaces on port 80. If there's a conflict or you just don't want it to listen with this configuration, just change the port and address
	minihttpd http("0.0.0.0", 80);

	// set up the root document to be from a file on disk, not cached in memory so you can update the file without needing to restart the server every time
	webpath rootDocument = "webroot/index.html";
	http.registerRoute("/", WebDoc(rootDocument, false), { GET }, nullptr);

	// set up a route to handle the actions from the web page and handle them according to the same basic functionality as the driver class sets up for terminal interface
	InstructionSet* actionHandler = new Instructions_Actions(b);
	http.registerRoute("/action", WebDoc(), { POST }, actionHandler);

	// Set up a route to give further information when needed to the client
	InstructionSet* inquiryHandler = new Instructions_Inquiry(b);
	http.registerRoute("/inquiry", WebDoc(), { POST }, inquiryHandler);

	// Set up a route to respond with the current difficulty of the chain forever and only in JSON
	InstructionSet* difficultyHandler = new Instructions_Diff(b);
	http.registerRoute("/difficulty", WebDoc(), { GET }, difficultyHandler);

	// This was just a test route to see if the reactive routes would work (and they do). It's just a current time thing. You can try it if you want
	// http://localhost/reactive
	InstructionSet* timeThing = new Instructions_Time();
	http.registerRoute("/reactive", WebDoc(), { GET }, timeThing);

	// Don't stop after one request
	// I don't know why that was ever an option, but I kept the functionality and have a server that only responds to one request and another that keeps listening to however many requests you want to throw at it
	http.keepRunning();
}
#endif

// Safely get an int from cin
int getIntFromUser(string prompt = "") {
	string strToInt;
	int input;
	cin.clear();
	cout << prompt;
	while (true) {
		try {
			getline(cin, strToInt);
			input = stoi(strToInt);
			return input;
		}
		catch (...) {
			cout << "\nThat's not an option. Try again.\n > ";
			cin.clear();
			continue;
		}
	}
}

// safely get a string from cin
string getStrFromUser(string prompt = "") {
	string str;
	cin.clear();
	cout << prompt;
	while (true) {
		try {
			getline(cin, str);
			return str;
		}
		catch (...) {
			cout << "\nThat's not an option. Try again.\n > ";
			cin.clear();
			continue;
		}
	}
}

// do the things
int main() {
	// initial prompt for loading or creating chain
	cout << "Do you want to import from a file or create a new blockchain?\n\tEnter 0 and then hit enter to import\n\tEnter 1 and then hit enter to create\n > ";
	int input;
	while (true) {
		input = getIntFromUser();
		if (input == 1 || input == 0)
			break;
		cout << "\nThat's not an option. Try again.\n > ";
	}

	// Do what the user said
	if (input == 0) {
		while (true) {
			cout << "You are importing from a file.\nEnter the full path of the file you want to read and then hit enter.\n > ";
			string filePath;
			getline(cin, filePath);
			BlockChainFilePath fp = filePath.c_str();
			try {
				b = new BlockChain(fp);
				cout << "\nThe file has been imported successfully.\n";
				break;
			}
			catch (JSONParseError&) {
				cout << "[!] The file at the path specified does not contain a valid JSON document. Could not continue.\n";
			}
			catch (BlockChainReadError&) {
				cout << "[!] The file at the path specified does not contain a valid blockchain. It is missing data required to reconstruct the blockchain.\n";
			}
		}
	}
	else {
		cout << "You are creating a new blockchain.\n";
		Block genesis("", "", "", 2, "Leeroy Jenkins");
		b = new BlockChain(genesis);
		//b->demo();
	}

	cout << "1 - add block" << endl <<
		"2 - verify blockchain" << endl <<
		"3 - view blockchain" << endl <<
		"4 - corrupt block" << endl <<
		"5 - fix corruption" << endl <<
		"6 - export blockchain" << endl <<
		"7 - change difficulty" << endl <<
		"8 - print all recipients by a sender" << endl <<
		"9 - print all senders by a recipient" << endl <<
		"10 - terminate program" << endl
#if WEB == true
		<< "11 - spin up minihttpd to do this through your browser\n"
#endif
	<< endl;
	while (true) {
		// force user to enter a valid option
		while (true) {
			input = getIntFromUser(" Menu option > ");
			if (input >= 1 && input <= 11)
				break;
			cout << "That's not an option. Try again.\n > ";
		}
		// once they do, do the things that the options say to do
		// These are pretty self explanatory since they have the terminal output inside the if statement scopes
		if (input == 1) {
			cout << "\nYou are creating a new block. I need information for that.\n";
			string sender = getStrFromUser("Who is the sender?\n > ");
			string recv = getStrFromUser("\nWho is the receiver?\n > ");
			string dat = getStrFromUser("\nWhat is the data?\n > ");
			cout << "Adding block (this may take a minute (literally depending on the difficulty (currently set to " << b->getDifficulty() << ")))" << endl;
			b->addBlock(sender, recv, b->getDifficulty(), dat);
			cout << "\nIt is done.\n";
		}
		else if (input == 2) {
			cout << "You are making me verify the blockchain's integrity.\n";
			cout << "Verifying... ";
			int status = b->verifyChain();
			cout << "Done.\n";
			if (status != -1) {
				cout << "The blockchain is invalid. The invalidity begins at block " << status << " of the chain.";
				cout << " If you want to repair the blockchain, remember that option 5 is for that.\n";
			}
			else {
				cout << "No blockchain corruption found.\n";
			}
		}
		else if (input == 3) {
			cout << "You have chosen to print the whole blockchain. I'll do that.\n";
			b->printAllBlocks();
		}
		else if (input == 4) {
			cout << "You have chosen to corrupt the block chain. I'll need information for that.\n";
			cout << "You first need to choose a block to corrupt. There are " << b->chainLength() << " blocks to choose from.\n Enter a number between 0 and " << b->chainLength() - 1 << " to select the block.\n > ";
			while (true) {
				input = getIntFromUser();
				if (input >= 0 && input <= (b->chainLength() - 1))
					break;
				cout << "Enter a number within the specified bounds.\n";
			}
			string newdata = getStrFromUser("What should the new data in this block be?\n > ");
			b->messUpChain(input, newdata);
			cout << "The chain is corrupt. You can verify that this is the case through option 2.\n";
		}
		else if (input == 5) {
			cout << "Repairing blockchain... This may take a while...\n";
			b->fixChain();
			cout << "Chain has been repaired.\n";
		}
		else if (input == 6) {
			cout << "You have chosen to export this blockchain to a JSON file. What is the file path you want to export to?\n";
			b->toFile(getStrFromUser("(relative or absolute path, I don't care) > "));
			cout << "The chain has been exported.\n";
		}
		else if (input == 7) {
			cout << "You have chosen to change your difficulty. Enter new difficulty between 1 and 7 (please don't do 7)\n";
			int newdiff;
			while (true) {
				newdiff = getIntFromUser(" > ");
				if (newdiff >= 1 && newdiff <= 7)
					break;
				cout << "Enter a number within the specified bounds.\n";
			}
			b->setDifficulty(newdiff);
			cout << "The chain difficulty is now " << newdiff << endl;
		}
		else if (input == 8) {
			cout << "Which sender whose receivers are you looking for?\n";
			string r = getStrFromUser(" > ");
			cout << "Who received from " << r << ":\n";
			b->printSenders(r);
			cout << endl;
		}
		else if (input == 9) {
			cout << "Which receiver whose senders are you looking for?\n";
			string r = getStrFromUser(" > ");
			cout << "Who sent to " << r << ":\n";
			b->printReceivers(r);
			cout << endl;
		}
		else if (input == 10) {
			break;
		}
		else if (input == 11) {
#if WEB == true
			cout << "\nThis mode starts a miniature web server on your localhost address. To visit it, go to \"http://localhost\" in any web browser to see it." << endl;
			cout << "You can exit this mode by going to http://localhost and choosing the \"terminate web server only\" option and getting the terminal interface back." << endl << endl;
			try {
				// start the server
				webserver();
			}
			// and wait for a signal of some kind and end the thing it's supposed to end
			catch (SigTerm&) {
				break;
			}
			catch (SigHalt&) {
				continue;
			}
#else
			cout << "This executable has not been built with the library required for HTTP capability. You'll have to run from the executable in the zip folder.\n\n";
#endif
		}
	}
	cout << "[passive aggressive] Bye" << endl;
	return 0;
}
