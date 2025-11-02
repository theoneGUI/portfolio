#include "plibcpp.h"

size_t plib_user::concurrent_users = 0;
bool plib_user::init = false;

plib_user::plib_user() {
	if (concurrent_users == 0)
	{
		p_libsys_init();
		init = true;
	}
	concurrent_users++;
}

plib_user::~plib_user() {
	if (concurrent_users == 1)
	{
		p_libsys_shutdown();
		init = false;
	}
	concurrent_users--;
}