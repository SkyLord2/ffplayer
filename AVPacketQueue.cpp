#include <iostream>
#include "AVPacketQueue.h"
using namespace std;
AVPacketQueue::AVPacketQueue()
{

}

AVPacketQueue::~AVPacketQueue()
{
	release();
}

void AVPacketQueue::abort()
{
	release();
	_pkt_queue.abort();
}

int AVPacketQueue::push(AVPacket* pkt)
{
	AVPacket* tmp_pkt = av_packet_alloc();
	av_packet_move_ref(tmp_pkt, pkt);
	return _pkt_queue.push(tmp_pkt);
}

AVPacket* AVPacketQueue::pop(const int timeout)
{
	AVPacket* pkt = NULL;
	int ret = _pkt_queue.pop(pkt, timeout);
	if (ret < 0)
	{
		ret == -1 ? 
			cerr << "av packet queue abort" << endl :
			cerr << "av packet queue is empty" << endl;
	}
	return pkt;
}

size_t AVPacketQueue::size()
{
	return _pkt_queue.size();
}

void AVPacketQueue::release()
{
	while (true)
	{
		AVPacket* pkt = NULL;
		int ret = _pkt_queue.pop(pkt, 1);
		if (ret < 0)
		{
			ret == -1 ?
				cerr << "av packet queue abort" << endl :
				cerr << "av packet queue is empty" << endl;
			break;
		}
		else
		{
			av_packet_free(&pkt);
		}
	}
}

