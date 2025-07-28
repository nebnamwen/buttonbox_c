ButtonBox is a simple digital synthesizer with a configurable keyboard interface
and modular software-defined intruments.

ButtonBox is implemented in C by Benjamin Newman and is free to use and modify.
Source code can be found at https://github.com/nebnamwen/buttonbox_c
The only build dependencies are the C standard library and SDL3 (https://www.libsdl.org)

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

White and black keys (as on a piano) are distinguished visually, and the
root note of the scale (C by default, but configurable) is marked with a dot.
The layout of the keyboard can be configured in a number of ways (see below).

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
which can be provided at startup on the command line or in files.

Config can also be read from standard input while the application runs.
Press the TAB key to read a line of config from standard input.
The application will block (pausing sound) until a line is available.
End a line with a backslash \ to read another line immediately with
a single press of TAB; config will be read until a line not ending in \
is encountered.  The \ must a separate token delimited by whitespace.

Clauses are separated by whitespace.  Blank lines or lines beginning
with a # are ignored.

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

        origin also determines which keys will be displayed with a dot
        indicating the root of the scale

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

              value: an integer between 1 and 8

split -- the key at which to split this keyboard section from the next

         value: key coordinates as a hex pair (see "origin", above)

slant -- a character representing the angle of the line dividing this
         keyboard section from the next

         value: one of / | \ ` ' , .

         / -- split along a rightward-slanting line, e.g. z-s-e-4

         | -- split along a vertical zig-zag, e.g. z-a-w-2

         \ -- split along a leftward-slanting line, e.g. z-a-q-1

         ` -- split from this key to the left, and all rows above

         ' -- split from this key to the right, and all rows above

         , -- split from this key to the left, and all rows below

         . -- split from this key to the right, and all rows below

         *some slant options must be escaped on the command line
	  to avoid being interpreted as metacharacters by the shell

-- examples of multiple keyboards --

Wicki-Hayden with four distinct octaves in left and right halves:

    inst=0 split=0x04 slant='|' inst=1 origin=0x06 transpose=60 color=brown

    *Wicki-Hayden is the default layout, but when used as a single keyboard
     the upper octave on the left is the same as the lower octave on the right

Piano-like with about three octaves in top and bottom halves:

    layout=0x12 split=0x10 slant=. inst=1 copy=0 ori=0x24 trans=60 col=brown

    *without splitting, this layout gives about an octave and a half of range,
     with the same notes in the bottom two rows repeated in the top two rows

== configuring instruments ==

Each instrument (keyboard section) can be assigned a different sound.

For inspiring the design choices behind ButtonBox's synthesis configuration,
I'm indebted to this video (https://www.youtube.com/watch?v=4SBDH5uhs4Q) of
Wendy Carlos, electronic music pioneer and the first transgender person to
recieve a Grammy Award, explaining the setup of a Moog analog synthesizer.

(Much of Wendy's discography is now out of print, but I particularly recommend
the soundtrack from the original TRON film, which is available digitally.)

-- instrument controls --

copy -- copy another instrument's synthesizer config and keyboard layout
        (useful when setting up multiple keyboards to play the same instrument)
	value: the other config to copy from (an integer between 0 and 8)
        *instrument 0 is reserved and will always be the null instrument;
         copy=0 can therefore be used to clear the current instrument

volume -- the main volume control for the current instrument
          value: a decimal number between 0 and 1.0
          default: 0.2
	  *this is relative to the current volume of your device
           and I recommend keeping it fairly small

pan -- the stereo placement of the current instrument
       value: a signed decimal number between -1.0 (left) and 1.0 (right)
       default: 0

== synthesis nodes ==

The sound-generating function of each instrument is defined as a
collection of nodes that are linked together.

Nodes include envelope generators (used to control volume over time),
waveform generators (used to generate an instrument's particular sound),
and math nodes which allow the outputs of other nodes to be combined and
modified in various ways.

The syntax of a node definition is like this: A=type:input,input,input

Each node is identified by a capital letter from A to Z, a node type, and a
list of input parameters.  As with config keys, only the first three letters
of the node type are used.

Each input can be any of the following:
  - a numerical constant, as a decimal number
  - the output of another node, identified by its letter
  - one of a small number of special values, beginning with an underscore:
    _s: a semitone (the 12th root of 2)
    _e: e, the root of the natural logarithm
    _p: the pitch of the current note, in Hz

Nodes are evaluated in order and can only reference earlier nodes as inputs.

Different types of nodes accept different numbers of inputs and interpret
those inputs differently.  Each input also has a default value depending on
the node type. To leave an input at its default value, it can be left blank.

For example:
    A=env      -- an envelope with all the default values
    B=sin:,4   -- a sine waveform with the default amplitude (1.0)
                    and frequency 4 Hz (e.g. to modify pitch for vibrato)
    C=mix:A,B  -- a mixer with A and B as its first and second inputs (multiply)
    D=mix:A,,C -- a mixer with A and C as its first and third inputs (add)

Each type of node is explained in more detail below.

The output of the instrument is the value of the (alphabetically) last node,
multiplied by the instrument's volume setting, and placed in stereo space
according to the instrument's pan setting.

The "set" keyword is not a node type but can be used to change one or more
of the inputs to a node without changing the other inputs or the node type.
This can be used when reading config from standard input to make adjustments.

Example:
    A=env A=set:,,,0.5 -- change the envelope's release parameter (see below)

-- envelopes --

A=env:attack,decay,sustain,release,delay

An envelope is a function that describes the way that a note gets louder and
softer over time, from the moment the note is struck to when it fades out.
By combining nodes, it can also be used to control other effects that change
intensity over time.

The most common envelope generator used in music synthesis has four parameters:
attack, decay, sustain, and release*.  Sometimes a delay is also included.

*https://en.wikipedia.org/wiki/Envelope_(music)

attack -- the time interval (in seconds) from the moment the note is struck
          to when it reaches maximum intensity
          value: a decimal number >= 0
          default: 0.05

decay -- the time interval (in seconds) it takes the envelope to decay
         from peak intensity to a steady-state (sustain) intensity
         value: a decimal number >= 0
         default: 0.05

sustain -- the steady-state intensity of the envelope for the rest of
           the time that a note is held
           value: a decimal number between 0 and 1.0
           default: 1.0

release -- the time interval (in seconds) it takes for the envelope
           to fade to silence after a note is released
           value: a decimal number >= 0
           default: 0.1

           *setting release to 0 can result in a click when a note is released
	    so it's recommended to use a small but positive value even when
            you want notes to stop "immediately"

delay -- the time interval (in seconds) after a key is pressed before the
         envelope attack begins (the release of the key is not delayed)
         value: a decimal number >= 0
         default: 0

         *chronologically in the progress of the envelope this should be first
          but it's placed last since it will be the most frequently omitted

Types: env | els | ers | esp

  - env (envelope): an envelope controlled by the onset and offset of a note key
  - els (envelope, left shift): controlled by the onset and offset of the left shift key*
  - env (envelope, right shift): controlled by the onset and offset of the right shift key*
  - env (envelope, space bar): controlled by the onset and offset of the space bar*

  *The left and right shift keys and space bar cannot be assigned notes, but can be used
   to modify the sound produced by an instrument when a regular note key is pressed.

-- waveforms --

B=sine:amplitude,pitch

- The default amplitude is 1.0
- The default pitch is _p, the pitch of the current note

types: sine | square | triangle | sawtooth | noise

  - A sine wave is a clear, pure, flutelike tone.

  - Square, triangle, and sawtooth are progressively more reedy
      or brassy, with more higher harmonics.

  - Noise is white noise, sampled at a multiple of the frequency
      of the note played (different noise notes do have distinct
      pitches and you can play a melody with them).

-- math nodes --

A mixer node multiplies and adds its inputs, returning a*b + c*d

F=mix:A,B,C,D

The default inputs are 0,1,0,1
  - so mix:A,B multiplies its inputs
    and mix:A,,B (omitting the second input) adds them

An exponential node raises one input to the power of another, with
additional inputs allowing common use cases to be expressed as one node.
It returns a * b^(c*d)

G=exp:_p,_s,A,3

This example multiplies the current pitch by a semitone raised to the
power 3 times the input A, which is to say it generates a pitch bend
of up to a minor third.

The default values are 1,_e,0,1
  - so exp:,,A is e^A

-- filters --

A filter node modifies a signal by attenuating (partially removing)
frequencies above or below a cutoff or threshold frequency.

J=low:signal,cutoff
K=high:signal,cutoff

There are many ways to implement filters and I've chosen the simplest,
which doesn't completely remove frequencies close to the threshold.
It can help to run a signal through multiple copies of the same filter.

The default cutoff frequency is the pitch of the current note.

== example instruments ==

-- simple waveforms --

simple square wave (using the default envelope):

    A=env B=square:A

snare drum (noise with fast attack, decay, and release):

    A=env:0.001,0.1,0,0 B=noise:A

ocean waves (noise with soft attack, decay, and release):

    A=env:0.5,0.75,0.5,0.5 B=noise:A

-- mixes of two waveforms --

sine wave plus a percussive crunch onset:

    A=env B=sin:A C=env:0.01,0.02,0,0 D=noise:C E=mix:B,,D

sine into sawtooth (sounds a bit brassy):

    A=env:0,0.8,0.5,0.1 B=sin:A C=env:0.8/0/1/0.1 D=saw:C E=mix:B,,D

triangle and sine with a fast decay (sounds a bit like a plucked string):

    A=env:0.005,0.1,0.1,0.1 B=tri:A C=env:0,0.3,0.2,0.1 D=sin:C E=mix:B,,D
