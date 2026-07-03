# 🐝 Flight of the Bumblebee (C Synthesizer)

I might not be able to play "Flight of the Bumblebee" on the piano, but I wrote a C program to do it for me. 

This is a complete WAV synthesizer written entirely from scratch in a single C file. No audio libraries, no pre-recorded samples—just pure math and raw PCM data written directly to disk.

## 🎧 Audio Files
* `bumblebee.wav` - The raw output generated directly by this code.
* `reference.wav` - A real orchestra reference recording (Credit: Pixabay).

## 🚀 How to Run
It's just standard C. Compile with the math library and run:

```bash
gcc -O2 -o bumblebee bumblebee.c -lm
./bumblebee bumblebee.wav
