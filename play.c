/*
 * This is sample program using wavplay.
 * It plays multiple WAV files.
 */

#include "wavplay.h"
#include <stdio.h>
#include <libgen.h>

int main(int argc, char *argv[]) {
	char *prog = basename(argv[0]);
	if (argc < 2) {
		printf("Usage: %s FILE...\n", prog);
		return 1;
	}
	if (snd_init() < 0) {
		fprintf(stderr, "%s: Failed to open device `" DEV_NAME "'\n", prog);
		return 1;
	}
	int i = 1;
	while (1) {
		wav_play(argv[i++]);
		if (i < argc) sleep(1);
		else break;
	}
	snd_end();
    return 0;
}
