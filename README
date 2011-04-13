=======
wavplay
=======

-------------------------------------------
a C library to play WAV sound via OSS/ALSA
-------------------------------------------

:Author: Zhihao Yuan <lichray@gmail.com>
:Date:   2011-04-12
:Copyright: 2-clause BSD License
:Version: 0.3b
:Manual section: 3

SYNOPSIS
=========

::

 #include "wavplay.h"

 void
 wav_play(const char *filename);

 int
 snd_init(void);

 void
 snd_end(void);

DESCRIPTION
============

A call to ``snd_init()`` is required before any other operations. Until the ``snd_end()`` is called manually or the program is quit, the sound device reminds open.

The ``wav_play()`` function opens and plays a WAV sound file. The sound hardware will be set according the file format.

If the macro ``USE_ALSA`` is defined, `wavplay` uses ALSA instead of OSS as the backend. The default OSS device is `/dev/dsp`, and the default ALSA PCM is `default`. They are defined by the macro ``DEV_NAME``.

RETURN VALUES
==============
If successful, ``snd_init()`` returns a non-negative integer. The actual value may be different for OSS/ALSA.

BUGS
=====

This library only supports standard, little-endian WAV file without a AIFF fact chunk.

