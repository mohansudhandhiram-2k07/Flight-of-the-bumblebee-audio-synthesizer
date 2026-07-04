# 🐝 C-Bumblebee: WAV Synthesizer

I might not be able to play "Flight of the Bumblebee" on the piano, but I wrote a C program to do it for me. 

This project is a complete WAV synthesizer written entirely from scratch in C. There are no external audio libraries, zero APIs, and no pre-recorded samples. The program mathematically calculates the precise frequency of every note, accumulates the phase, and writes the raw PCM audio data directly to a binary file.

## 🎧 Audio Samples
* **`bumblebee.wav`**: The raw output generated directly by this code.
* **`reference.wav`**: A real orchestra reference recording (Credit: Pixabay).

## 🛠️ Under the Hood (Technical Features)

* **Custom Binary File I/O:** Utilizes `#pragma pack(push, 1)` to strictly control struct padding, allowing for the direct bit-level generation of a standards-compliant RIFF/WAVE header.
* **Algorithmic DSP:** Phase-accumulating Sine, Square, and Sawtooth oscillators running at a strict 44.1kHz sample rate (respecting the Nyquist limit).
* **Polyphonic Mixing:** Dynamically mixes active chords and applies hard clipping (thresholded at 0.95) to prevent integer overflow.
* **Transient Control:** Custom 8% Attack / 8% Release software envelopes calculated per sample to eliminate audio clicking and smooth out waveforms during high-speed transitions.

## 🚀 Quick Start / Build Instructions

This is written in standard C with no external dependencies other than the math library. 

```bash
# Compile with optimization and the math library linked
gcc -O2 -o bumblebee bumblebee.c -lm

# Run the executable to generate the audio file
./bumblebee bumblebee.wav
