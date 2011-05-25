#ifndef _WAVPLAY_H
#define _WAVPLAY_H

/*-
 * wavplay - a C library to play WAV sound via OSS/ALSA
 *
 * Copyright (c) 2011 Zhihao Yuan.
 * All rights reserved.
 *
 * Reference:
 * http://www.freshports.org/audio/waveplay/
 *
 * This file is distributed under the 2-clause BSD License.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#if !defined(USE_ALSA)
#define DEV_NAME "/dev/dsp"
#include <fcntl.h>
#include <sys/ioctl.h>
#if defined(__NetBSD__) || defined(__OpenBSD__)
#include <soundcard.h>
#else
#include <sys/soundcard.h>
#endif
#else
#define DEV_NAME "default"
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>
#endif

typedef struct _riffchunk {
	char	id[4];
	int32_t	size;
} riffchunk_t;

typedef struct _wavheader {
	int16_t	format;
	int16_t	nchannels;
	int32_t	framerate;
	int32_t	byterate;
	int16_t	blocksize;
	int16_t	bitdepth;
} wavheader_t;

typedef struct _aifheader {
	int16_t	nchannels;
	int32_t	nframes;
	int16_t	bitdepth;
	char	framerate[10]; /* 80-bit IEEE 754 */
	char	comptype[4];
} aifheader_t;


int wav_play(const char *filename);
int wav_send(FILE *stream);

int snd_init(void);
int snd_set(int format, /* OSS/ALSA format constant */
	    int nchannels,
	    int framerate);
int snd_send(FILE *stream,
	     size_t size);
int snd_drop(void);
int snd_end(void);

#endif
