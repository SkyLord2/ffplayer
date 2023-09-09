#include <iostream>
#include <thread>
#include "demuxer.h"
using namespace std;
Demuxer::Demuxer(AVPacketQueue* audio_pkt_queue, AVPacketQueue* video_pkt_queue) : _a_pkt_q(audio_pkt_queue), _v_pkt_q(video_pkt_queue)
{

}

Demuxer::~Demuxer()
{
	stop();
}

int Demuxer::init(const char* url)
{
	int  ret = 0;
		_url = url;
	ifmt_ctx = avformat_alloc_context();
	ret = avformat_open_input(&ifmt_ctx, url, NULL, NULL);
	if (ret < 0)
	{
		av_strerror(ret, errstr, sizeof(errstr));
		cerr << "av open input failed:" << errstr << endl;
		return ret;
	}
	ret = avformat_find_stream_info(ifmt_ctx, NULL);
	if (ret < 0)
	{
		av_strerror(ret, errstr, sizeof(errstr));
		cerr << "av find stream info failed:" << errstr << endl;
		return ret;
	}
	av_dump_format(ifmt_ctx, 0, url, 0);
	aidx = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	vidx = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (aidx < 0 && vidx < 0)
	{
		return -1;
	}
	cout << "demuxer init success" << endl;
	return ret;
}

int Demuxer::start()
{
	_t = new thread(&Demuxer::run, this);
	if (!_t)
	{
		cerr << "demuxer start failed" << endl;
		return -1;
	}
	return 0;
}

int Demuxer::stop()
{
	Thread::stop();
	avformat_close_input(&ifmt_ctx);
	return 0;
}

void Demuxer::run()
{
	int ret = 0;
	AVPacket pkt;
	while (!_abort)
	{
		// TODO
		if (_a_pkt_q->size() > 100 && _v_pkt_q->size() > 100)
		{
			cout << "demuxer pause" << endl;
			this_thread::sleep_for(chrono::milliseconds(10));
			continue;
		}
		ret = av_read_frame(ifmt_ctx, &pkt);
		if (ret < 0)
		{
			av_strerror(ret, errstr, sizeof(errstr));
			cerr << "av read frame failed:" << errstr << endl;
			break;
		}
		if (pkt.stream_index == aidx)
		{
			_a_pkt_q->push(&pkt);
			// av_packet_unref(&pkt);
			cout << "audio packet queue size: " << _a_pkt_q->size() << endl;
		} 
		else if (pkt.stream_index == vidx)
		{
			_v_pkt_q->push(&pkt);
			cout << "video packet queue size: " << _v_pkt_q->size() << endl;
		}
		else {
			av_packet_unref(&pkt);
		}
	}
	cout << "demuxer finish the job" << endl;
}

AVCodecParameters* Demuxer::getAudioCodecParameters()
{
	if (aidx >= 0)
	{
		return ifmt_ctx->streams[aidx]->codecpar;
	} 
	else
	{
		return NULL;
	}
}

AVCodecParameters* Demuxer::getVideoCodecParameters()
{
	if (vidx >= 0)
	{
		return ifmt_ctx->streams[vidx]->codecpar;
	}
	else
	{
		return NULL;
	}
}

AVRational Demuxer::getAudioTimebase()
{
	if (aidx != -1)
	{
		return ifmt_ctx->streams[aidx]->time_base;
	} 
	else
	{
		return AVRational{ 0, 0 };
	}
}

AVRational Demuxer::getVideoTimebase()
{
	if (vidx != -1)
	{
		return ifmt_ctx->streams[vidx]->time_base;
	}
	else
	{
		return AVRational{ 0, 0 };
	}
}
