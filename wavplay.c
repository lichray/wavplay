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
#define BUF_SIZE 4096

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
#undef BUF_SIZE
