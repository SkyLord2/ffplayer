#pragma once
#include <chrono>
#include <ctime>
#include <math.h>

using namespace std;
using namespace std::chrono;

class AVSync
{
public:
	AVSync();
	~AVSync();
public:
	void   init_clock();
	void   set_clock_at(double pts, double time);
	void   set_clock(double pts);
	double get_clock();
	time_t get_microseconds();
public:
	double _pts       = 0;
	double _pts_drift = 0;

};

