/*
 * bumblebee.c — "Flight of the Bumblebee" (Rimsky Korsakov, 1899-1900 , public domain — composer died 1908)
 
 * Compile:  gcc -O2 -o bumblebee bumblebee.c -lm
 * Run:      ./bumblebee out.wav
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#define SAMPLE_RATE 44100   // Nyquist Theorem: 44100 / 2 = 22050 Hz & STM32 Timer 44100 interrupts every second
#define MAX_CHORD   10
#define AMP         0.32    // To avoid clipping

#pragma pack(push, 1)   // 44 bytes & avoid padding by compiler!

// Meta Data for .wav
typedef struct {
    char     riff[4];
    uint32_t chunk_size;
    char     wave[4];
    char     fmt[4];
    uint32_t fmt_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char     data[4];
    uint32_t data_size;
} WavHeader;
#pragma pack(pop)

static double note_freq(int semitone_from_a4) {
    return 440.0 * pow(2.0, semitone_from_a4 / 12.0);   //A4 international tuning standard (400hz)
}

typedef struct {
    int semitones[MAX_CHORD];
    int count;
    int duration_ms;
    int waveform; // 0 = sine, 1 = square, 2 = saw
} Event;

#define REST INT32_MIN

static double osc(int type, double phase) {
    phase = fmod(phase, 1.0);
    switch (type) {
        case 1: return phase < 0.5 ? 1.0 : -1.0;
        case 2: return 2.0 * (phase - floor(phase + 0.5));
        default: return sin(2.0 * M_PI * phase);
    }
}

static double clip(double x, double thresh) {
    if (x > thresh) return thresh;
    if (x < -thresh) return -thresh;
    return x;
}

static Event score[] = {
#include "bee_events.c"
};

int main(int argc, char **argv) {
    const char *outfile = argc > 1 ? argv[1] : "bumblebee.wav";
    int n_events = sizeof(score) / sizeof(score[0]);

    long total_samples = 0;
    for (int i = 0; i < n_events; i++)
    {
        total_samples += (long)(SAMPLE_RATE * (score[i].duration_ms / 1000.0));
    }

    int16_t *buf = malloc(total_samples * sizeof(int16_t));
    if (!buf) 
    {
        fprintf(stderr, "alloc failed\n"); return 1; 
    }

    long pos = 0;
    for (int i = 0; i < n_events; i++) {
        Event *e = &score[i];
        long n = (long)(SAMPLE_RATE * (e->duration_ms / 1000.0));
        double phase[MAX_CHORD] = {0};
        double freq[MAX_CHORD];
        int active = e->count;
        for (int v = 0; v < active; v++)
            freq[v] = note_freq(e->semitones[v]);

        for (long s = 0; s < n; s++) {
            double mix = 0.0;
            /* short attack/release envelope so notes don't click at speed */
            double env = 1.0;
            double t = (double)s / n;
            double attack = 0.08, release = 0.08;   // Not in the mood for ADSR envelope!
            if (t < attack) env = t / attack;
            else if (t > 1.0 - release) env = (1.0 - t) / release;

            double norm = active > 1 ? 1.0 / sqrt((double)active) : 1.0;
            for (int v = 0; v < active; v++) {
                mix += AMP * norm * env * osc(e->waveform, phase[v]);
                phase[v] += freq[v] / SAMPLE_RATE;
            }
            mix = clip(mix, 0.95);
            buf[pos++] = (int16_t)(mix * 32000.0);
        }
    }

    FILE *f = fopen(outfile, "wb");
    if (!f) { fprintf(stderr, "cannot open %s\n", outfile); return 1; }

    WavHeader h;
    memcpy(h.riff, "RIFF", 4);
    memcpy(h.wave, "WAVE", 4);
    memcpy(h.fmt, "fmt ", 4);
    memcpy(h.data, "data", 4);
    h.fmt_size = 16;
    h.audio_format = 1;
    h.num_channels = 1;
    h.sample_rate = SAMPLE_RATE;
    h.bits_per_sample = 16;
    h.block_align = h.num_channels * h.bits_per_sample / 8;
    h.byte_rate = h.sample_rate * h.block_align;
    h.data_size = (uint32_t)(total_samples * sizeof(int16_t));  // if using MCU use circular ring buffer.
    h.chunk_size = 36 + h.data_size;                            // This is for my lap with more than enough RAM

    fwrite(&h, sizeof(h), 1, f);
    fwrite(buf, sizeof(int16_t), total_samples, f);
    fclose(f);
    free(buf);

    printf("Wrote %s: %ld samples (%.2f sec), %d notes\n", outfile,
           total_samples, (double)total_samples / SAMPLE_RATE, n_events);
    return 0;
}
