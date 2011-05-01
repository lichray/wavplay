=======
wavplay
=======

-------------------------------------------
a C library to play WAV sound via OSS/ALSA
-------------------------------------------

:Author: Zhihao Yuan <lichray@gmail.com>
:Date:   2011-05-01
:Copyright: 2-clause BSD License
:Version: 0.4
:Manual section: 3

SYNOPSIS
=========


| ``#include "wavplay.h"``
|
| `int`
| ``wav_play``\(`const` `char` `\*` `filename`);
|
| `int`
| ``snd_init``\(`void`);
|
| `int`
| ``snd_end``\(`void`);

DESCRIPTION
============

The ``snd_init()`` function is required to be called before any other operations. Until the ``snd_end()`` function is called or the program is quit, the sound device reminds open.

The ``wav_play()`` function opens and plays a WAV sound file. If the filename is `NULL` (an empty pointer), the function reads from the standard input. The sound device will be set according to the file format.

If the macro ``USE_ALSA`` is defined, `wavplay` uses ALSA instead of OSS as the backend. The default OSS device is `/dev/dsp`, and the default ALSA PCM name is `default`. They are defined by the macro ``DEV_NAME``.

RETURN VALUES
==============
The ``snd_init()`` and ``snd_end()`` functions return a non-negative integer if successful. The actual value is different for OSS/ALSA.

The ``wav_play()`` function returns the value 0 if the sound data was sent to the device; otherwise the value -1 is returned.

BUGS
=====

This library does not support WAV files larger than 2GB.

