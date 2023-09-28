#pragma once
#include <stdexcept>

// Signal to be caught (thrown as runtime error) which will cause the program to exit
class SigTerm : public std::runtime_error {
public:
	SigTerm()
		: runtime_error("Shut down") 
	{

	}
};

// Signal to be caught (thrown as runtime error) which will cause the minihttpd server to shut down while keeping the terminal app active
class SigHalt : public std::runtime_error {
public:
	SigHalt()
		: runtime_error("Shut down web server only")
	{

	}
};