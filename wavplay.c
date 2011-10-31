/*-
 * wavplay - a C library to play WAV sound via OSS/ALSA
 *
 * Copyright (c) 2011 Zhihao Yuan.
 * All rights reserved.
 *
 * This file is distributed under the 2-clause BSD License.
 */

#include "wavplay.h"
#include <string.h>
#include <errno.h>
#include <sys/param.h>
#include <netinet/in.h>
#define BUF_SIZE	4096
#define PERIODS	4
#define eputs(s) (fprintf(stderr, "%s: " s "\n", __func__))

#ifndef USE_ALSA

#define WAV_FMT_8	AFMT_U8
#define AIF_FMT_8	AFMT_S8
#define WAV_FMT_16	AFMT_S16_LE
#define AIF_FMT_16	AFMT_S16_BE
#ifdef	AFMT_S24_PACKED
#define WAV_FMT_24	AFMT_S24_PACKED
#elif	__FreeBSD__
#define WAV_FMT_24	AFMT_S24_LE
#define SUN_FMT_24	AFMT_S24_BE
#endif
#ifdef	AFMT_S32_LE
#define WAV_FMT_32	AFMT_S32_LE
#define AIF_FMT_32	AFMT_S32_BE
#endif
#define WAV_FMT_A_LAW	AFMT_A_LAW
#define WAV_FMT_MU_LAW	AFMT_MU_LAW
#define WAV_FMT_IMA_ADPCM	AFMT_IMA_ADPCM

static int devfd = -1;

int snd_init(void) {
	if (devfd > -1) snd_end();
	devfd = open(DEV_NAME, O_WRONLY);
	return devfd;
}

int snd_set(int format, int nchannels, int framerate) {
	if (devfd > -1) {
		fcntl(devfd, F_GETFD);
		if (errno == EBADF) {
			devfd = -1;
			snd_init();
		}
	}
	int st = 0;
	st |= ioctl(devfd, SNDCTL_DSP_RESET, NULL);
	st |= ioctl(devfd, SNDCTL_DSP_SETFMT, &format);
	st |= ioctl(devfd, SNDCTL_DSP_CHANNELS, &nchannels);
	st |= ioctl(devfd, SNDCTL_DSP_SPEED, &framerate);
	return st;
}

int snd_end(void) {
	int st = close(devfd);
	devfd = -1;
	return st;
}

int snd_send(FILE *fp, size_t n) {
	unsigned char buf[BUF_SIZE];
	size_t l;
	while (n > sizeof(buf)) {
		if ((l = fread(buf, 1, sizeof(buf), fp)))
			if (write(devfd, buf, l) < 0) return -1;
			else;
		else goto EOS;
		n -= l;
	}
	if ((l = fread(buf, 1, n, fp)))
		write(devfd, buf, l);
	if (l < n)
		EOS: eputs("Unexpected end of stream");
	return ioctl(devfd, SNDCTL_DSP_SYNC, NULL);
}

int snd_drop(void) {
	return close(devfd);
}

#else

#define WAV_FMT_8	SND_PCM_FORMAT_U8
#define AIF_FMT_8	SND_PCM_FORMAT_S8
#define WAV_FMT_16	SND_PCM_FORMAT_S16_LE
#define AIF_FMT_16	SND_PCM_FORMAT_S16_BE
#define WAV_FMT_24	SND_PCM_FORMAT_S24_3LE
#define SUN_FMT_24	SND_PCM_FORMAT_S24_3BE
#define WAV_FMT_32	SND_PCM_FORMAT_S32_LE
#define AIF_FMT_32	SND_PCM_FORMAT_S32_BE
#define WAV_FMT_A_LAW	SND_PCM_FORMAT_A_LAW
#define WAV_FMT_MU_LAW	SND_PCM_FORMAT_MU_LAW
#define WAV_FMT_IMA_ADPCM	SND_PCM_FORMAT_IMA_ADPCM

static snd_pcm_t *pcm = NULL;

int snd_init(void) {
	if (pcm) snd_end();
	return snd_pcm_open(&pcm, DEV_NAME, SND_PCM_STREAM_PLAYBACK, 0);
}

int snd_set(int format, int nchannels, int framerate) {
	int st = 0;
	unsigned int val = framerate;
	snd_pcm_hw_params_t *params;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(pcm, params);
	st |= snd_pcm_hw_params_set_access(pcm, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	st |= snd_pcm_hw_params_set_format(pcm, params, format);
	st |= snd_pcm_hw_params_set_channels(pcm, params, nchannels);
	st |= snd_pcm_hw_params_set_rate_near(pcm, params, &val, 0);
	st |= snd_pcm_hw_params_set_periods(pcm, params, PERIODS, 0);
	st |= snd_pcm_hw_params(pcm, params);
	return st;
}

int snd_end(void) {
	int st = snd_pcm_close(pcm);
	pcm = NULL;
	return st;
}

int snd_send(FILE *fp, size_t n) {
	snd_pcm_format_t format;
	unsigned int nchannels;
	snd_pcm_uframes_t period;
	snd_pcm_hw_params_t *params;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_current(pcm, params);
	snd_pcm_hw_params_get_format(params, &format);
	snd_pcm_hw_params_get_channels(params, &nchannels);
	snd_pcm_hw_params_get_period_size(params, &period, 0);
	int framesize = snd_pcm_format_width(format) / 8 * nchannels;
	unsigned char buf[period * framesize];
	size_t l;
	while (n > sizeof(buf)) {
		if ((l = fread(buf, 1, sizeof(buf), fp))) {
			switch (snd_pcm_writei(pcm, buf, l / framesize)) {
			case -EBADF:
				return -1;
			case -EPIPE:
#ifndef NDEBUG
				snd_pcm_recover(pcm, -EPIPE, 0);
#else
				snd_pcm_prepare(pcm);
#endif
			}
		}
		else goto EOS;
		n -= l;
	}
	if ((l = fread(buf, 1, n, fp)))
		snd_pcm_writei(pcm, buf, l / framesize);
	if (l < n)
		EOS: eputs("Unexpected end of stream");
	return snd_pcm_drain(pcm);
}

int snd_drop(void) {
	return snd_pcm_drop(pcm);
}

#endif

static int wav2format(const wavheader_t *wav) {
	switch (wav->format) {
	case 1:
	case -2:
		switch ((wav->bitdepth + 7) / 8) {
		case 1: return WAV_FMT_8;
		case 2: return WAV_FMT_16;
#ifdef	WAV_FMT_24
		case 3: return WAV_FMT_24;
#endif
#ifdef	WAV_FMT_32
		case 4: return WAV_FMT_32;
#endif
		default: return -1;
		}
	case 258:
	case 6: return WAV_FMT_A_LAW;
	case 257:
	case 7: return WAV_FMT_MU_LAW;
	case 17: return wav->bitdepth == 4 ? WAV_FMT_IMA_ADPCM : -1;
	default: return -1;
	}
}

#define chktype(s) (!strncmp(aif->comptype, s, 4))

static int aif2format(aifheader_t *aif) {
	int sampwidth = (aif->bitdepth + 7) / 8;
	if (chktype("") || chktype("NONE"))
		switch (sampwidth) {
		case 1: return AIF_FMT_8;
		case 2: return AIF_FMT_16;
#ifdef	WAV_FMT_24
		case 3: return WAV_FMT_24;
#endif
#ifdef	AIF_FMT_32
		case 4: return AIF_FMT_32;
#endif
		default: return -1;
		}
	else if (chktype("sowt")) {
		if (sampwidth == 1)
			return AIF_FMT_8;
		return wav2format(&(wavheader_t)
				{ .format = 1, .bitdepth = aif->bitdepth });
	}
	else if (chktype("alaw") || chktype("ALAW"))
		return WAV_FMT_A_LAW;
	else if (chktype("ulaw") || chktype("ULAW"))
		return WAV_FMT_MU_LAW;
	else if (chktype("ima4"))
		return WAV_FMT_IMA_ADPCM;
	else
		return -1;
}

static int sun2format(sunheader_t *sun) {
	switch (sun->encoding) {
	case 1: return WAV_FMT_MU_LAW;
	case 2: return AIF_FMT_8;
	case 3: return AIF_FMT_16;
#ifdef SUN_FMT_24
	case 4: return SUN_FMT_24;
#endif
	case 5: return AIF_FMT_32;
	case 27: return WAV_FMT_A_LAW;
	default: return -1;
	}
}

/*
 * converts any endianess to host
 * returns -1 when format is wrong,
 *          0 when no convertion is needed,
 *          positive value == len(fmt).
 */
static int endian2h(const char fmt[], void *p) {
	if (htonl(1) == 1) {
		if (fmt[0] == '>') return 0;
		else if (fmt[0] != '<') return -1;
	} else {
		if (fmt[0] == '<') return 0;
		else if (fmt[0] != '>') return -1;
	}
	const char *i = fmt;
	while (*(++i) != '\0') {
#define fmtInt(x, T) \
	case x: { \
		uint##T##_t *n = p; \
		*n = bswap##T(*n); \
		p += sizeof(uint##T##_t); \
		break; \
	}
#define fmtSkip(x, n) \
	case x: { \
		p += n; \
		break; \
	}
		switch (*i) {
			fmtInt('h', 16);
			fmtInt('l', 32);
			fmtInt('q', 64);
			fmtSkip('.', 1);
		default : return -1;
		}
#undef fmtInt
#undef fmtSkip
	}
	return i - fmt;
}

typedef union {
	uint32_t	com[4];
	long double	value;
} ldouble_t;

static __inline long double ext2l(extdouble_t x) {
	return htonl(1) == 1 ?
		(ldouble_t){{ 0, x.expon, x.himant, x.lomant }}.value :
		(ldouble_t){{ x.lomant, x.himant, x.expon, 0 }}.value;
}

#define skip(n) do { \
	if (! fseek(fp, 0, SEEK_CUR)) { \
		fseek(fp, (n), SEEK_CUR); \
		break; \
	} \
	char c[BUF_SIZE]; \
	size_t i = n; \
	ssize_t l; \
	while (i > 0) { \
		l = fread(c, 1, i > sizeof(c) ? sizeof(c) : i, fp); \
		if (l > 0) i -= l; \
		else break; \
	} \
} while (0)
#define read2(t) (fread(&t, sizeof(t), 1, fp))
#define chkid(s) (!strncmp(ck.id, s, 4))

static size_t wavparse(wavheader_t *wav, FILE *fp) {
	riffchunk_t ck;
	if (!read2(ck.id) || !chkid("WAVE"))
		eputs("Not a WAVE file");
	else {
		while (read2(ck)) {
			endian2h("<l", &ck.size);
			if (chkid("data"))
				return ck.size;
			ck.size = (ck.size + 1) / 2 * 2;
			if (chkid("fmt ")) {
				fread(wav, sizeof(wavheader_t), 1, fp);
				if (ck.size < sizeof(wavheader_t))
					eputs("Bad format chunk");
				else
					skip(ck.size - sizeof(wavheader_t));
			}
			else skip(ck.size);
		}
		eputs("Malformed WAVE file");
	}
	return 0;
}

static size_t aifparse(aifheader_t *aif, FILE *fp) {
	riffchunk_t ck;
	if (!read2(ck.id) || !(chkid("AIFF") || chkid("AIFC")))
		eputs("Not a AIFF/AIFC file");
	else {
		while (read2(ck)) {
			endian2h(">l", &ck.size);
			if (chkid("SSND"))
				return ck.size;
			ck.size = (ck.size + 1) / 2 * 2;
			if (chkid("COMM")) {
				int ckl = MIN(ck.size, sizeof(aifheader_t));
				fread(aif, ckl, 1, fp);
				if (ckl < (sizeof(aifheader_t) - sizeof(aif->comptype)))
					eputs("Bad common chunk");
				else
					skip(ck.size - ckl);
			}
			else skip(ck.size);
		}
		eputs("Malformed AIFF/AIFC file");
	}
	return 0;
}

static size_t wav_readinfo(wav_info_t *info, FILE *fp) {
	riffchunk_t ck;
	if (read2(ck)) {
		if (chkid("RIFF")) {
			wavheader_t wav[1];
			size_t sz = wavparse(wav, fp);
			endian2h("<hhllhh", wav);
			info->nchannels = wav->nchannels;
			info->framerate = wav->framerate;
			info->sampwidth = (wav->bitdepth + 7) / 8;
			info->nframes = sz / (info->nchannels * info->sampwidth);
			info->devformat = wav2format(wav);
			return sz;
		}
		else if (chkid("FORM")) {
			aifheader_t aif[1] = {{ 0 }};
			size_t sz = aifparse(aif, fp);
			endian2h(">hlhhll", aif);
			info->nchannels = aif->nchannels;
			info->framerate = (int) ext2l(aif->framerate);
			info->sampwidth = (aif->bitdepth + 7) / 8;
			info->nframes = aif->nframes;
			info->devformat = aif2format(aif);
			return sz;
		}
		else if (chkid(".snd")) {
			sunheader_t sun;
			endian2h(">l", &ck.size);
			read2(sun);
			endian2h(">llll", &sun);
			if (ck.size < sizeof(ck) + sizeof(sun))
				eputs("Bad Au header");
			else
				skip(ck.size - sizeof(ck) - sizeof(sun));
			info->nchannels = sun.nchannels;
			info->framerate = sun.framerate;
			if (1 < sun.encoding || sun.encoding < 6)
				info->nframes = sun.size /
					(info->sampwidth = sun.encoding - 1);
			else {
				info->sampwidth = 2;
				info->nframes = sun.size;
			}
			info->devformat = sun2format(&sun);
			return sun.size;
		}
	}
	eputs("Unknown file format");
	return 0;
}

#undef skip
#undef read2
#undef chkid

static int wav_setdev(const wav_info_t *info) {
	if (info->devformat < 0)
		eputs("Unsupported PCM format");
	else if (snd_set(info->devformat, info->nchannels, info->framerate))
		eputs("Failed to setup the sound device");
	else return 0;
	return -1;
}

int wav_send(FILE *fp) {
	wav_info_t info[1];
	size_t size = wav_readinfo(info, fp);
	if (size && !wav_setdev(info))
		return snd_send(fp, size);
	return -1;
}

int wav_play(const char *fn) {
	int st = -1;
	FILE *fp = fopen(fn, "rb");
	if (fp) {
		st = wav_send(fp);
		fclose(fp);
	} else perror(__func__);
	return st;
}

