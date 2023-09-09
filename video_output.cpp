#include "video_output.h"
#include <iostream>

#define REFRESH_RATE 0.01

using namespace std;

VideoOutput::VideoOutput(AVSync* av_sync, AVRational time_base, AVFrameQueue* frameQ, int width, int height) : _av_sync(av_sync), _time_base(time_base), _frameQ(frameQ), _width(width), _height(height)
{

}

VideoOutput::~VideoOutput()
{

}

int VideoOutput::init()
{
	int ret = SDL_Init(SDL_INIT_VIDEO);
	if (ret < 0)
	{
		cerr << "sdl video init failed" << endl;
		return ret;
	}
	_win = SDL_CreateWindow("ffmpeg player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _width, _height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (!_win)
	{
		cerr << "sdl window create failed" << endl;
		return -1;
	}
	_renderer = SDL_CreateRenderer(_win, -1, 0);
	if (!_renderer)
	{
		cerr << "sdl renderer create failed" << endl;
		return -2;
	}
	_texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, _width, _height);
	if (!_texture)
	{
		cerr << "sdl texture create failed" << endl;
		return -3;
	}
	_yuv_buffer_size = _width * _height * 1.5;
	_yuv_buffer = (uint8_t*)malloc(_yuv_buffer_size);
	return 0;
}

int VideoOutput::main_loop()
{
	SDL_Event event;
	while (true)
	{
		refresh_loop(&event);
		switch (event.type)
		{
			case SDL_KEYDOWN:
			{
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					cout << "esc key pressed" << endl;
					return 0;
				}
				break;
			}
			case SDL_QUIT:
			{
				cout << "sdl quit" << endl;
				break;
			}
			default:
				break;
		}
	}
}

void VideoOutput::refresh_loop(SDL_Event* event)
{
	double remaining_time = 0.0;
	SDL_PumpEvents();
	while (!SDL_PeepEvents(event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT))
	{
		if (remaining_time > 0.0)
		{
			int64_t sleep_time = (int64_t)(remaining_time * 1000);
			this_thread::sleep_for(chrono::milliseconds(sleep_time));
		}
		remaining_time = REFRESH_RATE;
		video_refresh(&remaining_time);
		SDL_PumpEvents();
	}
}

void VideoOutput::video_refresh(double* remaining_time)
{
	AVFrame* frame = NULL;
	frame = _frameQ->font();
	if (frame)
	{
		double pts = frame->pts * av_q2d(_time_base);
		cout << "video pts = " << pts << endl;
		double diff = pts - _av_sync->get_clock();
		if (diff > 0)
		{
			*remaining_time = FFMIN(*remaining_time, diff);
			return;
		}
		_rect.x = 0;
		_rect.y = 0;
		_rect.w = _width;
		_rect.h = _height;
		SDL_UpdateYUVTexture(
			_texture,
			(const SDL_Rect*)&_rect,
			frame->data[0],
			frame->linesize[0],
			frame->data[1],
			frame->linesize[1],
			frame->data[2],
			frame->linesize[2]
		);
		SDL_RenderClear(_renderer);
		SDL_RenderCopy(_renderer, _texture, NULL, &_rect);
		SDL_RenderPresent(_renderer);
		frame = _frameQ->pop(10);
		av_frame_free(&frame);
	} 
	else
	{
	}
}
