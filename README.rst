=======
wavplay
=======

-------------------------------------------
a C library to play WAV sound via OSS/ALSA
-------------------------------------------

:Author: Zhihao Yuan <lichray@gmail.com>
:Date:   2011-05-24
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
| ``wav_send``\(`FILE` `\*` `stream`);
|
| `int`
| ``snd_init``\(`void`);
|
| `int`
| ``snd_end``\(`void`);
|
| `int`
| ``snd_drop``\(`void`);

DESCRIPTION
============

The ``snd_init()`` initializes the sound device. Until the ``snd_end()`` function is called or the program is quit, the device reminds open. The ``snd_drop()`` function stops the device from playing.

The ``wav_play()`` function opens and plays a WAV sound file, and the ``wav_send()`` function plays a WAV data stream. The sound device will be set according to the sampling format.

If the macro ``USE_ALSA`` is defined, ALSA is used instead of OSS as the backend. The default OSS device is `/dev/dsp`, and the default ALSA PCM name is `default`. They are defined by the macro ``DEV_NAME``.

RETURN VALUES
==============
The ``snd_init()``, ``snd_end()``, and ``snd_drop()`` functions return a non-negative integer if successful. The actual value is different for OSS/ALSA.

The ``wav_play()`` and ``wav_send()`` functions return the value 0 if the sound data was sent to the device; otherwise the value -1 is returned.

BUGS
=====

This library does not support WAV files larger than 2GB.

