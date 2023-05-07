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
may be freely mixed in one command line and will be parsed in the order provided.

See the sections below for information on config options.

== playing ButtonBox ==

Use a QWERTY keyboard setting when playing ButtonBox.

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
                  but unfortunately is very vulnerable to ghosting

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

== configuring instruments ==
