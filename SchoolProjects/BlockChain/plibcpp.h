#pragma once
#include <plibsys.h>

// Keep record of how many people are using the plibsys library resources at a time
// Once the number of people using plibsys reaches zero, then free resources bound by plibsys
class plib_user {

public:
	plib_user();
	~plib_user();
protected:
	static size_t concurrent_users;
	static bool init;
};

// side note: since plibsys is written in C, it has to initialize and free memory and it doesn't exactly happen at will
// long story short, deallocating resources while they're being used by another thread is a great way to lose friends
// the solution is this class to ONLY deallocate when it is safe to do so; all classes using plibsys must inherit from this class
// in order to keep the peace