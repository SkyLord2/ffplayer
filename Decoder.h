#pragma once
#include "thread.h"
#include "AVPacketQueue.h"
#include "AVFrameQueue.h"
extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
}
class Decoder : public Thread
{
public:
	Decoder(AVFrameQueue* frameQ, AVPacketQueue* packetQ);
	~Decoder();
public:
	int	 init(AVCodecParameters* codecpar);
	int  start();
	int	 stop();
	void run();
private:
	AVCodecContext* _codecCtx = NULL;
	AVPacketQueue*	 _packetQ = NULL;
	AVFrameQueue*	  _frameQ = NULL;
	char		  errstr[256] = { 0 };
};

