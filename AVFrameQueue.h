#pragma once
#include "myqueue.h";
#include <iostream>
extern "C" {
#include "libavcodec/avcodec.h"
}
class AVFrameQueue
{
public:
	AVFrameQueue(string type_name);
	~AVFrameQueue();
public:
	void	 abort();
	int		 push(AVFrame* frame);
	AVFrame* pop(const int time);
	AVFrame* font();
	size_t	 size();
private:
	void   release();
private:
	MyQueue<AVFrame*> _frame_queue;
	std::string _type_name = "";
};

