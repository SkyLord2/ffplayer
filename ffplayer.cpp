#define _CTR_SECURE_NO_WRANINGS
#include <iostream>
#include <assert.h>

/*
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/error.h"
#include "libavutil/avstring.h"
#include "libavutil/time.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "SDL.h"
}
*/

#include "demuxer.h"
#include "Decoder.h"
#include "AVFrameQueue.h"
#include "AVPacketQueue.h"
#include "video_output.h"
#include "audio_output.h"
#include "AVSync.h"

using namespace std;

/**
TODO_LIST
1. 退出资源释放
2. 音视频同步，只有视频如何同步
3. 播放暂停，seek，逐帧
4. 破音
*/

int main(int argc, char* argv[])
{
	int ret = 0;
	AVPacketQueue v_pkt_q;
	AVPacketQueue a_pkt_q;
	AVFrameQueue  v_frm_q("video");
	AVFrameQueue  a_frm_q("audio");
	AVSync		  av_sync;


	av_sync.init_clock();
	
	Demuxer* demuxer = new Demuxer(&a_pkt_q, &v_pkt_q);
	if (!demuxer)
	{
		return -1;
	}
	ret = demuxer->init(argv[1]);
	if (ret < 0)
	{
		return ret;
	}
	ret = demuxer->start();
	if (ret < 0)
	{
		return ret;
	}
	
	Decoder* a_decoder = new Decoder(&a_frm_q, &a_pkt_q);
	if (!a_decoder)
	{
		return -1;
	}
	AVCodecParameters* a_codec_params = demuxer->getAudioCodecParameters();
	ret = a_decoder->init(a_codec_params);
	if (ret < 0)
	{
		return ret;
	}
	ret = a_decoder->start();
	if (ret < 0)
	{
		return ret;
	}

	Decoder* v_decoder = new Decoder(&v_frm_q, &v_pkt_q);
	if (!v_decoder)
	{
		return -1;
	}
	AVCodecParameters* v_codec_params = demuxer->getVideoCodecParameters();
	ret = v_decoder->init(v_codec_params);
	if (ret < 0)
	{
		return ret;
	}
	ret = v_decoder->start();
	if (ret < 0)
	{
		return ret;
	}
	/**/
	AVRational a_time_base = demuxer->getAudioTimebase();
	AudioParams audio_params = { 0 };
	memset(&audio_params, NULL, sizeof(AudioParams));
	audio_params.channels = a_codec_params->channels;
	audio_params.channel_layout = a_codec_params->channel_layout;
	audio_params.sample_format = (enum AVSampleFormat)a_codec_params->format;
	audio_params.freqence = a_codec_params->sample_rate;
	audio_params.frame_size = a_codec_params->frame_size;
	AudioOutput* audio_output = new AudioOutput(&av_sync, a_time_base, audio_params, &a_frm_q);
	ret = audio_output->init();
	if (ret < 0)
	{
		cerr << "audio output init failed" << endl;
		return -1;
	}

	AVRational v_time_base = demuxer->getVideoTimebase();
	VideoOutput* video_output = new VideoOutput(&av_sync, v_time_base, &v_frm_q, v_codec_params->width, v_codec_params->height);
	if (!video_output)
	{
		return -1;
	}
	ret = video_output->init();
	if (ret < 0)
	{
		cerr << "video output init failed" << endl;
		return -1;
	}
	video_output->main_loop();
	
	this_thread::sleep_for(chrono::milliseconds(1200 * 1000));

	return 0;
}
