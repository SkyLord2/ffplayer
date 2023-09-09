#pragma once
#include <stdint.h>
#include "AVFrameQueue.h"
#include "AVSync.h"
extern "C" {
#include "libswresample/swresample.h"
#include "SDL.h"
}
typedef struct AudioParams
{
	int					freqence;
	int					channels;
	int64_t				channel_layout;
	enum AVSampleFormat sample_format;
	int					frame_size;
} AudioParams;

class AudioOutput
{
public:
	AudioOutput(AVSync* av_sync, AVRational time_base, const AudioParams& audio_params, AVFrameQueue* frameQ);
	~AudioOutput();
public:
	int  init();
	int  uninit();
	// void fill_audio_pcm(void* udata, Uint8* stream, int len);
public:
	AudioParams        _src_params			= {0};
	AudioParams		   _dst_params			= {0};
	AVFrameQueue*	   _frameQ              = NULL;
	struct SwrContext* _swr_ctx				= NULL;
	uint8_t*		   _audio_buffer		= NULL;
	uint8_t*		   _audio_buffer1       = NULL; // buf1
	uint32_t		   _audio_buffer_size	= 0;
	uint32_t		   _audio_buffer1_size	= 0;    // buf1 size
	uint32_t		   _audio_buffer_index  = 0;
	AVSync*			   _av_sync				= NULL;
	AVRational		   _time_base			= { 0 };
	int64_t			   _pts					= AV_NOPTS_VALUE;
};

