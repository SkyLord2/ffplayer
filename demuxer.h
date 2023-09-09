#pragma once
#include <string>
#include "thread.h"
extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
}
#include "AVPacketQueue.h"

using namespace std;
class Demuxer : public Thread
{
public:
	Demuxer(AVPacketQueue* audio_pkt_queue, AVPacketQueue* video_pkt_queue);
	~Demuxer();
public:
	int	 init(const char* url);
	int  start();
	int	 stop();
	void run();
	AVCodecParameters* getAudioCodecParameters();
	AVCodecParameters* getVideoCodecParameters();
	AVRational getAudioTimebase();
	AVRational getVideoTimebase();
protected:
private:
	char		  errstr[256] = { 0 };
	string				 _url = "";
	AVFormatContext* ifmt_ctx = NULL;
	AVPacketQueue*   _a_pkt_q = NULL;
	AVPacketQueue*   _v_pkt_q = NULL;
	int				     aidx = -1;
	int					 vidx = -1;
};