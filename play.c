/*
 * A sample program using wavplay. It can play multiple PCM files;
 * if the filename is `-', it reads a file from the standard input.
 * Press Ctrl-\ to drop the current playing file.
 */

#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "wavplay.h"

#ifdef SIGQUIT
static void drop(int sig) {
	if (sig == SIGQUIT)
		snd_drop();
}
#endif

int main(int argc, char *argv[]) {
	char *prog = argv[0];
	if (argc < 2) {
		printf("Usage: %s [FILE|-]...\n", prog);
		return 1;
	}
	if (snd_init() < 0) {
		fprintf(stderr, "%s: Failed to open device `"
				DEV_NAME "'\n", prog);
		return 1;
	}
#ifdef SIGQUIT
	signal(SIGQUIT, drop);
#endif
	int i = 1;
	while (1) {
		const char *fn = argv[i++];
		if (!strcmp(fn, "-"))
			fn = NULL;
		if (fn ? wav_play(fn) : wav_send(stdin))
			fprintf(stderr, "%s: Skipping file `%s'\n",
					prog, fn ? fn : "STDIN");
		if (i < argc) sleep(1);
		else break;
	}
	snd_end();
    return 0;
}
