#include "thread.h"
Thread::Thread() {

}

Thread::~Thread() {
	stop();
}

int Thread::start() {
	return 0;
}

int Thread::stop() {
	_abort = 0;
	if (_t)
	{
		_t->join();
		delete _t;
		_t = NULL;
		return 0;
	}
	else 
	{
		return -1;
	}
}