#pragma once
#include "AVFrameQueue.h"
#include "AVSync.h"
extern "C" {
#include "SDL.h"
}
class VideoOutput
{
public:
	VideoOutput(AVSync* av_sync, AVRational time_base, AVFrameQueue* frameQ, int width, int height);
	~VideoOutput();
public:
	int  init();
	int  main_loop();
	void refresh_loop(SDL_Event* event);
private:
	void video_refresh(double* remaining_time);
private:
	AVFrameQueue* _frameQ			= NULL;
	SDL_Event	  _event			= {0};
	SDL_Rect	  _rect				= {0};
	SDL_Window*	  _win				= NULL;
	SDL_Renderer* _renderer			= NULL;
	SDL_Texture*  _texture			= NULL;
	int           _width			= 0;
	int           _height			= 0;
	uint8_t*      _yuv_buffer		= NULL;
	int           _yuv_buffer_size	= 0;
	SDL_mutex*	  _mutex			= NULL;

	AVSync*		  _av_sync          = NULL;
	AVRational	  _time_base        = { 0 };
};

