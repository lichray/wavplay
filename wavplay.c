/*-
 * wavplay - a C library to play WAV sound via OSS/ALSA
 *
 * Copyright (c) 2011 Zhihao Yuan.
 * All rights reserved.
 *
 * This file is distributed under the 2-clause BSD License.
 */

#include "wavplay.h"
#define DEV_NODE "/dev/dsp"
#define PCM_NAME "default"
#define BUF_SIZE 4096
#define PERIOD   2

#define FMT2BITS(fmt) ((fmt & (7<<3)) + 8)

#if !defined(USE_ALSA)

static int devfd = -1;

int snd_init(void) {
	if (devfd > -1) snd_end();
	devfd = open(DEV_NODE, O_WRONLY);
	return devfd;
}

void snd_set(int format, int nchannels, int framerate) {
	ioctl(devfd, SNDCTL_DSP_RESET, NULL);
	ioctl(devfd, SNDCTL_DSP_SETFMT, &format);
	ioctl(devfd, SNDCTL_DSP_CHANNELS, &nchannels);
	ioctl(devfd, SNDCTL_DSP_SPEED, &framerate);
}

void snd_end(void) {
	close(devfd);
	devfd = -1;
}

void snd_play(FILE *fp, size_t n) {
	unsigned char buf[BUF_SIZE];
	size_t i = 0;
	if (devfd < 0) snd_init();
	while (!feof(fp)) {
		fread(buf, sizeof(buf), 1, fp);
		write(devfd, buf, n - i < sizeof(buf) ? n - i : sizeof(buf));
		i += sizeof(buf);
	}
}

#else

static snd_pcm_t *pcm = NULL;

int snd_init(void) {
	if (pcm) snd_end();
	return snd_pcm_open(&pcm, PCM_NAME, SND_PCM_STREAM_PLAYBACK, 0);
}

void snd_set(int format, int nchannels, int framerate) {
	unsigned int val = framerate;
	snd_pcm_hw_params_t *params;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(pcm, params);
	snd_pcm_hw_params_set_access(pcm, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(pcm, params, format);
	snd_pcm_hw_params_set_channels(pcm, params, nchannels);
	snd_pcm_hw_params_set_rate_near(pcm, params, &val, 0);
	snd_pcm_hw_params_set_periods(pcm, params, PERIOD, 0);
	snd_pcm_hw_params_set_buffer_size(pcm, params, (BUF_SIZE * PERIOD) / 2);
	snd_pcm_hw_params(pcm, params);
}

void snd_end(void) {
	snd_pcm_drain(pcm);
	snd_pcm_close(pcm);
	pcm = NULL;
}

void snd_play(FILE *fp, size_t n) {
	unsigned char buf[BUF_SIZE * 2];
	size_t i = 0;
	if (pcm == NULL) snd_init();
	snd_pcm_prepare(pcm);
	while (!feof(fp)) {
		fread(buf, sizeof(buf), 1, fp);
		snd_pcm_writei(pcm, buf, BUF_SIZE / 2);
		i += sizeof(buf);
	}
}

#endif

wavfile_t * wav_open(const char *fn) {
	FILE *fp;
	wavfile_t *wav = NULL;
	if ((fp = fopen(fn, "r"))) {
		wavheader_t header;
		fread(&header, sizeof(header), 1, fp);
		wav = (wavfile_t*) malloc(sizeof(wavfile_t));
		wav->stream = fp;
		wav->size = header.nframes;
		snd_set(header.format, header.nchannels, header.framerate);
	}
	return wav;
}

void wav_close(wavfile_t *wav) {
	fclose(wav->stream);
	free(wav);
}

void wav_play(const char *fn) {
	wavfile_t *wav;
	if ((wav = wav_open(fn))) {
		snd_play(wav->stream, wav->size);
		wav_close(wav);
	} else perror(__func__);
}

#undef DEV_NODE
#undef PCM_NAME
#undef BUF_SIZE
