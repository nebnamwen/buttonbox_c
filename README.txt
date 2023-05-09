ButtonBox is a simple digital synthesizer with a configurable keyboard interface.

ButtonBox is implemented in C by Benjamin Newman and is free to use and modify.
Source code can be found at https://github.com/nebnamwen/buttonbox_c
The only build dependencies are the C standard library and SDL2 (https://www.libsdl.org)

A proof-of-concept in Java with slightly different features is also available
at https://github.com/nebnamwen/buttonbox -- it uses Midi instruments, so it can
produce a wider variety of more-realistic sounds, but has too much input lag
to be usable as a performing instrument.  This C version is much more responsive.

== building ButtonBox ==

I'm more experienced with writing code than I am with setting up build environments.
Consult the documentation for your C compiler and the SDL2 library if necessary.

The ButtonBox project comes with a Makefile for use with the `make` build tool
common on Unix-ish systems.  If your system has `make`, you should be able to build
ButtonBox from its source directory by just invoking `make` without arguments.

== running ButtonBox ==

Invoke the built executable `buttonbox` with or without command-line arguments.

Any arguments will be interpreted either as config clauses (if they contain a '=')
or as files to read and parse for config clauses.  Raw config clauses and config files
may be freely mixed in one command line and will be parsed in the order given.

See the sections below for information on config options.

Press the Escape key or close the display window to quit.

== playing ButtonBox ==

ButtonBox displays a panel of buttons representing part of a computer keyboard.
The four rows of buttons correspond to the four rows of keys that make up the
main part of the keyboard and are arranged approximately in a hexagonal grid,
i.e. with the keys in each row offset by half from the keys in the next row.

Play the instrument using the keyboard -- the display is for visual feedback only
and cannot be interacted with using a pointing device.

White and black keys (as on a piano) are distinguished visually.
The layout of the keyboard can be configured in a number of ways (see below).

*Text labels identifying each note may be added in a future update.

-- key ghosting --

You may notice when trying to play chords that some combinations of keys
cannot be pressed at the same time.  This is called "ghosting" and is usually
a hardware limitation of your keyboard.  Unfortunately there is no way to
compensate for this at the software level.  Which combinations of keys are
affected may vary from keyboard to keyboard.

Non-ghosting keyboards are available, or you can try modifying the layout
so that the chords you want to play correspond to combinations that won't
ghost on the keyboard you have.

== configuring ButtonBox ==

ButtonBox accepts configuration in the form of key=value clauses,
which can be provided directly on the command line or in files.

The key and value must be separated by a '=' sign (no whitespace),
and only the first three letters of the key are significant.

The expected format of the value varies depending on the key.
See below for all config keys, expected values, and what they do.

== configuring the keyboard ==

color -- determines the color of the keys in the graphical display
         (especially useful when configuring multiple keyboards, see below)

         values: gray | red | green | yellow | blue | magenta | cyan | brown

origin, transpose -- together determine where on the keyboard
                     and on what note the scale starts

        origin -- the key to start counting from, as a hex pair
                  e.g. 'z' is '0x00' (row 0 from the bottom, key 0 in that row)
                       'd' is '0x12' (row 1 from the bottom, key 2 in that row)
                  *both coordinates are 0-based
                  *the leading '0x' is required

        transpose -- the MIDI note number to assign to the origin key
                     *MIDI note numbers are integers between 0 and 127
                     *middle C is 60 and concert A (440Hz) is 69
                     *the notes of a piano keyboard run from 21 to 108

        for example, to set 'g' to be middle C: origin=0x14 transpose=60

        twelve even-tempered notes per octave is the only supported scale

layout -- determines how notes are arranged on the keyboard

       ButtonBox supports isometric keyboard layouts, i.e. layouts in which
       moving one key to the right always represents the same interval,
       and likewise moving one key up always represents the same interval.

       This has the advantage that chords and melodies can be transposed
       from one key to another without changing their shape.

       There are a variety of such keyboard layouts depending on the two
       intervals corresponding to a rightward step and an upward step.

       value: a vector giving the up-and-to-the-right-interval*
              and the right-interval, in semitones, as a hex pair

              *yes, this is inconsistent with the convention used to
               identify keys (see "origin"), but necessary to get some
               layouts to work since both values must be positive
               (a way to specify negative intervals may be added later)

       examples:
          0x12 -- as close as we can get to a piano keyboard, and also
                  similar to the arrangement of frets on a harpejji

          0x13 -- C system button accordion
          0x23 -- B system button accordion

          0x72 -- Wicki-Hayden concertina

          0x61 -- bass guitar fretboard ("strings" in fourths)
          0x81 -- violin fretboard ("strings" in fifths)

          0x94 -- harmonic table (puts major and minor triads in clusters);
                  this makes it easy to play chords with one finger,
                  but unfortunately is very vulnerable to key ghosting

       references:
          https://en.wikipedia.org/wiki/Harpejji
          https://en.wikipedia.org/wiki/Chromatic_button_accordion
          https://en.wikipedia.org/wiki/Wicki–Hayden_note_layout
          https://en.wikipedia.org/wiki/Harmonic_table_note_layout

== configuring multiple keyboards ==

The keyboard can be divided into multiple sections (up to 8), which can be
assigned different layouts and colors (see above) and sounds (see below).

instrument -- select the keyboard section and instrument to which
              subsequent config clauses will be applied

              value: an integer between 0 and 7

split -- the key at which to split this keyboard section from the next

         value: key coordinates as a hex pair (see "origin", above)

slant -- a character representing the angle of the line dividing this
         keyboard section from the next

         value: one of / | \

         / -- split along a rightward-slanting line, e.g. z-s-e-4

         | -- split along a vertical zig-zag, e.g. z-a-w-2

         \ -- split along a leftward-slanting line, e.g. z-a-q-1

         *some slant options must be escaped on the command line
	  to avoid being interpreted as metacharacters by the shell

-- examples of multiple keyboards --

Wicki-Hayden with four distinct octaves:

    inst=0 split=0x04 slant='|' inst=1 origin=0x06 transpose=60 color=brown

    *Wicki-Hayden is the default layout, but when used as a single keyboard
     the upper octave on the left is the same as the lower octave on the right

== configuring instruments ==

Each instrument (keyboard section) can be assigned a different sound.

For inspiring the design choices behind ButtonBox's synthesis configuration,
I'm indebted to this video (https://www.youtube.com/watch?v=4SBDH5uhs4Q) of
Wendy Carlos, electronic music pioneer and the first transgender person to
recieve a Grammy Award, explaining the setup of a Moog analog synthesizer.

(Much of Wendy's discography is now out of print, but I particularly recommend
the soundtrack from the original TRON film, which is available digitally.)

ButtonBox synthesizes sound directly in software using the most common
waveforms used in music synthesis, modified by ADSR envelopes.

-- instrument controls --

copy -- copy the entire instrument config from another keyboard
        (useful when setting up multiple keyboard sections to
         play the same instrument)
	value: the instrument to copy from (an integer between 0 and 7)

volume -- the main volume control for the current instrument
          value: a decimal number between 0 and 1.0
	  *this is relative to the maximum volume of your device
           and I recommend keeping it fairly small

pan -- the stereo placement of the current instrument
       value: a signed decimal number between -1.0 (left) and 1.0 (right)

-- waveforms --

Five basic waveforms are available.  Each instrument can use any combination
of these waveforms, each with its own envelope.  The instrument also has a
main envelope which is applied after the waveforms are combined.  For config
purposes, "main" is treated as another type of waveform when selecting which
envelope to modify.

waveform -- select the waveform whose envelope will be modified by
            subsequent config clauses

            values: main | sine | square | triangle | sawtooth | noise

            A sine wave is a clear, pure, flutelike tone.

            Square, triangle, and sawtooth are progressively more reedy
            or brassy, with more higher harmonics.

            Noise is white noise, sampled at a multiple of the frequency
            of the note played (different noise notes do have distinct
            pitches and you can play a melody with them).

-- envelopes --

An envelope is a function that describes the way that a note gets louder and
softer over time, from the moment the note is struck to when it fades out.

The most common envelope generator used in music synthesis has four parameters:
attack, decay, sustain, and release*.  ButtonBox adds one more parameter, peak.

*https://en.wikipedia.org/wiki/Envelope_(music)

attack -- the time interval (in seconds) from the moment the note is struck
          to when a given waveform reaches maximum intensity
          value: a decimal number >= 0

peak -- the volume of the given waveform at the end of the attack interval
        value: a decimal number between 0 and 1.0

        *typically the peak of an envelope would always be normalized to 1.0,
         but with a separate peak parameter you can mix multiple waveforms
         with different peak intensities in the same instrument configuration

decay -- the time interval (in seconds) it takes the given waveform to decay
         from peak intensity to a steady-state (sustain) intensity
         value: a decimal number >= 0

sustain -- the steady-state intensity of the given waveform for the rest of
           the time that a note is held
           value: a decimal number between 0 and 1.0

           *despite the names, nothing stops you from making sustain > peak

release -- the time interval (in seconds) it takes for the given waveform
           to fade to silence after a note is released
           value: a decimal number >= 0

           *setting release to 0 can result in a click when a note is released
	    so it's recommended to use a small but positive value even when
            you want notes to stop "immediately"

envelope -- set all five envelope parameters in one config clause
            value: five decimal numbers (as above, in the same order)
                   separated by commas or forward slashes (no whitespace)

-- example instruments --

By default each waveform's envelope is set to constant zero, but can be set
to another constant value simply by setting the sustain parameter.
You can use the "main" envelope to control the instrument's envelope shape
and select a (constant) mix of waveforms by setting sustain only.

Setting more of the envelope parameters for individual waveforms allows you
to build instruments with more complex sounds that change timbre over time.

simple square wave (using the default main envelope):

    waveform=square sustain=1.0

simple sine wave plus a percussive crunch onset:

    waveform=sine sustain=1.0 waveform=noise envelope=0.01/1/0.02/0/0

snare drum (noise with fast attack, decay, and release):

    waveform=main envelope=0.001/1/0.1/0/0 waveform=noise sustain=1

ocean waves (noise with soft attack, decay, and release):

    wav=main env=0.5/1/0.75/0.5/0.5 wav=noise sus=1

sine into sawtooth (sounds a bit brassy):

    wav=sine env=0/1/0.8/0.5/0.1 wav=saw env=0.8/1/0/1/0.1

triangle and sine with a fast decay (sounds a bit like a plucked string):

    wav=tri env=0.005/1/0.1/0.1/0.1 wav=sine env=0/1/0.3/0.2/0.1
