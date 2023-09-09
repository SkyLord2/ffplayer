#include "audio_output.h"
#include <iostream>

using namespace std;

AudioOutput::AudioOutput(AVSync* av_sync, AVRational time_base, const AudioParams& audio_params, AVFrameQueue* frameQ) : _av_sync(av_sync), _time_base(time_base), _src_params(audio_params), _frameQ(frameQ)
{

}

AudioOutput::~AudioOutput()
{

}

void fill_audio_pcm(void* udata, Uint8* stream, int len)
{
	AudioOutput* is = (AudioOutput*)udata;
	uint32_t copy_left = 0;
	int		audio_size = 0;
	while (len > 0)
	{
		cout << "fill pcm len = " << len << endl;
		if (is->_audio_buffer_index == is->_audio_buffer_size)
		{
			is->_audio_buffer_index = 0;
			AVFrame* frame = is->_frameQ->pop(10);
			if (frame)
			{
				is->_pts = frame->pts;
				const AudioParams dst_params = is->_dst_params;
				if (
					frame->format != dst_params.sample_format ||
					frame->sample_rate != dst_params.freqence ||
					frame->channel_layout != dst_params.channel_layout &&
					!is->_swr_ctx
					)
				{
					is->_swr_ctx = swr_alloc_set_opts(
						NULL,
						dst_params.channel_layout,
						dst_params.sample_format,
						dst_params.freqence,
						frame->channel_layout,
						(enum AVSampleFormat)frame->format,
						frame->sample_rate,
						0,
						NULL
					);
					if (!is->_swr_ctx || swr_init(is->_swr_ctx) < 0)
					{
						cerr << "can't create sample rate converter for conversion" << endl;
						swr_free(&is->_swr_ctx);
						return;
					}
				}
				if (is->_swr_ctx)
				{
					// 重采样
					const uint8_t** in = (const uint8_t**)frame->extended_buf;
					uint8_t**      out = &is->_audio_buffer1;
					int out_samples = frame->nb_samples * dst_params.freqence / frame->sample_rate * 256;
					int out_bytes = av_samples_get_buffer_size(NULL, dst_params.channels, out_samples, dst_params.sample_format, 0);
					if (out_bytes < 0)
					{
						cerr << "get out bytes failed" << endl;
						return;
					}
					av_fast_malloc(&is->_audio_buffer1, &is->_audio_buffer1_size, out_bytes);
					int converted_len = swr_convert(is->_swr_ctx, out, out_samples, in, frame->nb_samples);
					if (converted_len < 0)
					{
						cerr << "call swr_convert failed" << endl;
						return;
					}
					is->_audio_buffer = is->_audio_buffer1;
					is->_audio_buffer_size = av_samples_get_buffer_size(NULL, dst_params.channels, converted_len, dst_params.sample_format, 1);
				}
				else
				{
					// 无重采样
					audio_size = av_samples_get_buffer_size(NULL, frame->channels, frame->nb_samples, (enum AVSampleFormat)frame->format, 1);
					av_fast_malloc(&is->_audio_buffer1, &is->_audio_buffer1_size, audio_size);
					is->_audio_buffer = is->_audio_buffer1;
					is->_audio_buffer_size = audio_size;
					memcpy(is->_audio_buffer, frame->data[0], audio_size);
				}
				av_frame_free(&frame);
			}
			else
			{
				is->_audio_buffer = NULL;
				is->_audio_buffer_size = 512;
			}
		}
		copy_left = is->_audio_buffer_size - is->_audio_buffer_index;
		if (copy_left > len)
		{
			copy_left = len;
		}
		if (!is->_audio_buffer)
		{
			memset(stream, NULL, copy_left);
		}
		else
		{
			memcpy(stream, is->_audio_buffer + is->_audio_buffer_index, copy_left);
		}
		len -= copy_left;
		stream += copy_left;
		is->_audio_buffer_index += copy_left;
	}
	if (is->_pts != AV_NOPTS_VALUE)
	{
		double pts = is->_pts * av_q2d(is->_time_base);
		cout << "audio pts = " << pts << endl;
		is->_av_sync->set_clock(pts);
	}
}

int AudioOutput::init()
{
	cout << "audio output init begin" << endl;
	int ret = SDL_Init(SDL_INIT_AUDIO);
	if (ret < 0)
	{
		cerr << "sdl audio init failed: " << ret << endl;
		return ret;
	}
	SDL_AudioSpec wanted_spec, spec;
	wanted_spec.channels = _src_params.channels;
	wanted_spec.freq	 = _src_params.freqence;
	wanted_spec.format   = AUDIO_S16SYS;
	wanted_spec.samples  = _src_params.frame_size;
	wanted_spec.silence  = 0;
	wanted_spec.callback = fill_audio_pcm;
	wanted_spec.userdata = this;
	ret = SDL_OpenAudio(&wanted_spec, &spec);
	if (ret < 0)
	{
		cerr << "sdl open audio failed: " << ret << endl;
		return ret;
	}
	_dst_params.channels	   = spec.channels;
	_dst_params.freqence	   = spec.freq;
	_dst_params.frame_size     = _src_params.frame_size;
	_dst_params.sample_format  = AV_SAMPLE_FMT_S16;
	_dst_params.channel_layout = av_get_default_channel_layout(spec.channels);
	SDL_PauseAudio(0);
	cout << "audio output init end" << endl;
	return 0;
}

int AudioOutput::uninit()
{
	cout << "audio output uninit" << endl;
	SDL_PauseAudio(1);
	SDL_CloseAudio();
	return 0;
}
