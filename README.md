# NestlingAudio VCV plugin

Like this plugin? Please consider showing your support by making a donation!

[![Donate](https://www.paypalobjects.com/en_US/i/btn/btn_donate_LG.gif)](https://www.paypal.com/donate/?hosted_button_id=MJYE5QYRB5QLW)

## µJazz

µJazz is a module which takes two CV inputs specifying an "underlying" chord, and one CV input specifying a melody note, and outputs pitch CV for three harmony notes for the melody note based on the underlying chord.  The effect is meant to mimic how one might harmonize a melody in jazz arranging, e.g. in a horn section of a big band.

### Input

The underlying chord is specified by a root and a chord type, i.e. for a Cmaj7 chord the root is a "C" and the chord type is "maj7".

The root note is specified with 1v/oct.  However, the octave of the root note is irrelevant since the root note is not output as-is.  Only the pitch class matters, i.e. 'C', 'C#', 'D', etc.

The chord type is specified with voltage from 0-10v which is divided equally amongst the following chord types:

- dim7
- min7b5 (half-diminished)
- min7
- min(maj7)
- 7 (dominant)
- maj7
- 7(#5)
- maj7(#5)
- 7(sus4)
- maj7(sus4)

The melody note is specified with 1v/oct.  

### Output

The harmony notes that are output have the following characteristics:

- they are all lower in pitch than the melody note (so the melody note stays on top)
- if the melody note is one of the notes in the underlying chord, then all the harmony notes will also be notes in the underlying chord, forming an inversion of that chord with the melody note on top
- if the melody note is NOT one of the notes in the underlying chord, then the harmony notes will form a fully-diminished 7th chord based on the *melody* note (NOT the underlying chord), with the melody note still on top.

### Examples

**Root: C  
Chord: maj7**

- Melody: C5  
Output 1: B4  
Output 2: G4  
Output 3: E4

- Melody: D5  
Output 1: B4  
Output 2: G#4  
Output 3: F4

- Melody: E5  
Output 1: C5  
Output 2: B4  
Output 3: G4


**Root: D  
Chord: min7**

- Melody: C#5  
Output 1: A#4  
Output 2: G4  
Output 3: E4

- Melody: D5  
Output 1: C5  
Output 2: A4  
Output 3: F4

- Melody: E5  
Output 1: C#5  
Output 2: A#4  
Output 3: G4

- Melody: F5  
Output 1: D5  
Output 2: C5  
Output 3: A4

TODO: add link to demo video
