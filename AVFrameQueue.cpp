#include <iostream>
#include "AVFrameQueue.h"

using namespace std;

AVFrameQueue::AVFrameQueue(string type_name) : _type_name(type_name)
{

}

AVFrameQueue::~AVFrameQueue() 
{
	release();
}

void AVFrameQueue::abort() 
{
	release();
	_frame_queue.abort();
}

int AVFrameQueue::push(AVFrame* frame) 
{
	AVFrame* tmpFrame = av_frame_alloc();
	av_frame_move_ref(tmpFrame, frame);
	return _frame_queue.push(tmpFrame);
}

AVFrame* AVFrameQueue::pop(const int time)
{
	AVFrame* frame = NULL;
	int ret = _frame_queue.pop(frame, time);
	if (ret < 0)
	{
		cerr << "pop frame failed" << endl;
	}
	return frame;
}

AVFrame* AVFrameQueue::font()
{
	AVFrame* frame = NULL;
	int ret = _frame_queue.front(frame);
	if (ret < 0)
	{
		cerr << _type_name << " font frame failed" << endl;
	}
	return frame;
}

size_t AVFrameQueue::size()
{
	return _frame_queue.size();
}

void AVFrameQueue::release()
{
	while (true)
	{
		AVFrame* frame = NULL;
		int ret = _frame_queue.pop(frame, 1);
		if (ret < 0)
		{
			ret == -1 ?
				cerr << "av frame queue abort" << endl :
				cerr << "av frame queue is empty" << endl;
			break;
		}
		else
		{
			av_frame_free(&frame);
		}
	}
}