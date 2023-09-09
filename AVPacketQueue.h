#pragma once
#include "myqueue.h"
extern "C" {
#include "libavcodec/avcodec.h"
}
class AVPacketQueue
{
public:
	AVPacketQueue();
	~AVPacketQueue();
public:
	void      abort();
	int       push(AVPacket* pkt);
	AVPacket* pop(const int timeout);
	size_t	  size();
private:
	void	  release();
private:
	MyQueue<AVPacket*> _pkt_queue;

};

