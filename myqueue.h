#pragma once
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>

using namespace std;

template<typename T>
class MyQueue
{
public:
	MyQueue() {};
	~MyQueue() {};
public:
	void abort() {
		_abort = 1;
		_cond.notify_all();
	}
	int	 push(T val) {
		lock_guard<mutex> lock(_mutex);
		if (!_abort)
		{
			_queue.push(val);
			_cond.notify_one();
			return 0;
		}
		else
		{
			return -1;
		}
	}
	int  pop(T& val, const int timeout) {
		unique_lock<mutex> lock(_mutex);
		if (_queue.empty())
		{
			_cond.wait_for(lock, chrono::milliseconds(timeout), [this] {
				return !_queue.empty() || _abort;
				});
		}
		if (_abort)
		{
			return -1;
		}
		if (_queue.empty())
		{
			return -2;
		}
		val = _queue.front();
		_queue.pop();
		return 0;
	}
	int  front(T& val) {
		lock_guard<mutex> lock(_mutex);
		if (_abort)
		{
			return -1;
		}
		if (_queue.empty())
		{
			return -2;
		}
		val = _queue.front();
		return 0;
	}
	size_t size() {
		lock_guard<mutex> lock(_mutex);
		return _queue.size();
	}
private:
	int _abort = 0;
	mutex _mutex;
	condition_variable _cond;
	queue<T> _queue;
};
