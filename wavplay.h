#ifndef _WAVPLAY_H
#define _WAVPLAY_H

/*-
 * wavplay - a C library to play WAV sound via OSS/ALSA
 *
 * Copyright (c) 2011 Zhihao Yuan.
 * All rights reserved.
 *
 * Reference:
 * http://yannesposito.com/Scratch/en/blog/2010-10-14-Fun-with-wav/
 *
 * This file is distributed under the 2-clause BSD License.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

typedef struct {
	char	id[4];
	int32_t	size;
} riffchunk_t;

typedef struct {
	int16_t	comptype;
	int16_t	nchannels;
	int32_t	framerate;
	int32_t	byterate;
	int16_t	_align;
	int16_t	bitdepth;
} wavheader_t;

typedef struct {
	FILE *stream;
	size_t size;
} wavfile_t;

int         snd_init(void);
void        snd_end(void);
void        snd_set(int format, int nchannels, int framerate);
void        snd_play(FILE *stream, size_t size);
wavfile_t * wav_open(const char *filename);
void        wav_play(const char *filename);
void        wav_close(wavfile_t *wav);

#endif
