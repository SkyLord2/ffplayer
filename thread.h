#pragma once
#include <thread>
using namespace std;
class Thread
{
public:
	Thread();
	~Thread();
public:
	int start();
	int stop();
	virtual	void run() = 0;
protected:
	int _abort = 0;
	thread* _t = NULL;
private:
};