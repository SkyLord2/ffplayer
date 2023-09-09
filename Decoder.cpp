#include "Decoder.h"
#include <iostream>
Decoder::Decoder(AVFrameQueue* frameQ, AVPacketQueue* packetQ) : _frameQ(frameQ), _packetQ(packetQ), _codecCtx(NULL)
{

}

Decoder::~Decoder()
{
	stop();
	if (_codecCtx)
	{
		avcodec_close(_codecCtx);
	}
}

int Decoder::init(AVCodecParameters* codecpar)
{
	if (!codecpar)
	{
		cerr << "decoder init failed, codecpar is null" << endl;
		return -1;
	}
	_codecCtx = avcodec_alloc_context3(NULL);
	if (!_codecCtx)
	{
		cerr << "alloc codec context failed" << endl;
		return -1;
	}
	int ret = 0;
	ret = avcodec_parameters_to_context(_codecCtx, codecpar);
	if (ret < 0)
	{
		av_strerror(ret, errstr, sizeof(errstr));
		cerr << errstr << endl;
		return ret;
	}
	const AVCodec* codec = avcodec_find_decoder(_codecCtx->codec_id);
	if (!codec)
	{
		cerr << "can't find a matched decoder" << endl;
		return -1;
	}
	ret = avcodec_open2(_codecCtx, codec, NULL);
	if (ret < 0)
	{
		av_strerror(ret, errstr, sizeof(errstr));
		cerr << errstr << endl;
		return ret;
	}
	cout << "decoder init success" << endl;
	return 0;
}

int Decoder::start()
{
	_t = new thread(&Decoder::run, this);
	if (!_t)
	{
		cerr << "decoder start failed" << endl;
		return -1;
	}
	return 0;
}

int Decoder::stop()
{
	return Thread::stop();
}

void Decoder::run()
{
	cout << "decode run" << endl;
	AVFrame* frame = av_frame_alloc();
	int ret = 0;
	while (_abort != 1)
	{
		if (_frameQ->size() > 10)
		{
			//cout << _codecCtx->codec_id << "frame queue more than 10, decode pause" << endl;
			this_thread::sleep_for(chrono::milliseconds(10));
			continue;
		}
		AVPacket* pkt = _packetQ->pop(10);
		if (pkt)
		{
			ret = avcodec_send_packet(_codecCtx, pkt);
			av_packet_free(&pkt);
			if (ret < 0)
			{
				av_strerror(ret, errstr, sizeof(errstr));
				cerr << errstr << endl;
				break;
			}
			while (true)
			{
				ret = avcodec_receive_frame(_codecCtx, frame);
				if (ret == 0)
				{
					_frameQ->push(frame);
					//cout << _codecCtx->codec->name << " frame queue size: " << _frameQ->size() << endl;
				}
				else if (AVERROR(EAGAIN) == ret)
				{
					break;
				}
				else
				{
					_abort = 1;
					av_strerror(ret, errstr, sizeof(errstr));
					cerr << errstr << endl;
					break;
				}
			}
		}
		else
		{
			cerr << "packet is null" << endl;
		}
	}
	cout << "decode finish" << endl;
}
