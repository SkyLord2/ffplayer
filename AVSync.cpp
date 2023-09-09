#include "AVSync.h"

AVSync::AVSync() : _pts_drift(0), _pts(0)
{

}

AVSync::~AVSync()
{

}

void AVSync::init_clock()
{
	set_clock(NAN);
}

void AVSync::set_clock_at(double pts, double time)
{
	_pts       = pts;
	_pts_drift = pts - time;
}

void AVSync::set_clock(double pts)
{
	double time = get_microseconds() * 1000000.0;	//s
	set_clock_at(pts, time);
}

double AVSync::get_clock()
{
	double time = get_microseconds() * 1000000.0;
	return _pts_drift + time;
}

time_t AVSync::get_microseconds()
{
	system_clock::time_point time_point_new = system_clock::now();
	system_clock::duration duration         = time_point_new.time_since_epoch();
	time_t us = duration_cast<microseconds>(duration).count();
	return us;
}
