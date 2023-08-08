#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <portaudio.h>

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 2048
#define RECORD_SECONDS 3

static int audioCallback(const void *inputBuffer, void *outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo *timeInfo,
                        PaStreamCallbackFlags statusFlags,
                        void *userData)
{
    int16_t *buffer = (int16_t *)userData;
    const int16_t *in = (const int16_t *)inputBuffer;

    for (unsigned int i = 0; i < framesPerBuffer; i++)
    {
        buffer[i] = *in++; // Speichern der Aufnahmedaten im Puffer
    }

    return paContinue;
}

int main()
{
    PaError err;
    PaStream *stream;
    int num_samples = SAMPLE_RATE * RECORD_SECONDS;

    // Initialisieren von PortAudio
    err = Pa_Initialize();
    if (err != paNoError)
    {
        printf("PortAudio konnte nicht initialisiert werden: %s\n", Pa_GetErrorText(err));
        return 1;
    }

    int16_t *data = (int16_t *)malloc(sizeof(int16_t) * num_samples);
    if (!data)
    {
        printf("Fehler beim Allokieren des Pufferspeichers\n");
        Pa_Terminate();
        return 1;
    }

    // Öffnen des Aufnahme-Streams
    err = Pa_OpenDefaultStream(&stream, 1, 1, paInt16, SAMPLE_RATE, FRAMES_PER_BUFFER, audioCallback, data);
    if (err != paNoError)
    {
        printf("Fehler beim Öffnen des Aufnahme-Streams: %s\n", Pa_GetErrorText(err));
        free(data);
        Pa_Terminate();
        return 1;
    }

    // Starten des Aufnahme-Streams
    err = Pa_StartStream(stream);
    if (err != paNoError)
    {
        printf("Fehler beim Starten des Aufnahme-Streams: %s\n", Pa_GetErrorText(err));
        Pa_CloseStream(stream);
        free(data);
        Pa_Terminate();
        return 1;
    }

    printf("Nehmen Sie bitte eine 3-Sekunden-Aufnahme auf...\n");

    // Warten, bis die Aufnahme beendet ist
    int numFramesRecorded = 0;
    while (numFramesRecorded < (SAMPLE_RATE * RECORD_SECONDS))
    {
        Pa_Sleep(100);
        numFramesRecorded += FRAMES_PER_BUFFER;
    }

    // Stoppen des Aufnahme-Streams
    err = Pa_StopStream(stream);
    if (err != paNoError)
    {
        printf("Fehler beim Stoppen des Aufnahme-Streams: %s\n", Pa_GetErrorText(err));
    }

    // Schließen des Aufnahme-Streams
    err = Pa_CloseStream(stream);
    if (err != paNoError)
    {
        printf("Fehler beim Schließen des Streams: %s\n", Pa_GetErrorText(err));
    }

    // Beenden von PortAudio
    err = Pa_Terminate();
    if (err != paNoError)
    {
        printf("PortAudio konnte nicht beendet werden: %s\n", Pa_GetErrorText(err));
    }

    // Speichern der Aufnahme in einer .wav-Datei
    FILE *file = fopen("aufnahme.wav", "wb");
    if (!file)
    {
        printf("Fehler beim Öffnen der Datei zum Schreiben\n");
        free(data);
        return 1;
    }

    // WAV-Header schreiben
    fwrite("RIFF", 1, 4, file);
    int fileSize = num_samples * sizeof(int16_t) + 36;
    fwrite(&fileSize, 4, 1, file);
    fwrite("WAVE", 1, 4, file);
    fwrite("fmt ", 1, 4, file);
    int fmtSize = 16;
    fwrite(&fmtSize, 4, 1, file);
    int16_t audioFormat = 1;
    fwrite(&audioFormat, 2, 1, file); // Format: PCM (uncompressed)
    int16_t numChannels = 1;
    fwrite(&numChannels, 2, 1, file); // Anzahl der Kanäle: 1 (Mono)
    int sampleRate = SAMPLE_RATE;
    fwrite(&sampleRate, 4, 1, file); // Sample-Rate: 44100
    int bytesPerSecond = SAMPLE_RATE * sizeof(int16_t);
    fwrite(&bytesPerSecond, 4, 1, file); // Bytes pro Sekunde: Sample-Rate * Block-Align
    int16_t blockAlign = sizeof(int16_t);
    fwrite(&blockAlign, 2, 1, file); // Block-Align: Anzahl der Kanäle * Sample-Größe in Bytes
    int16_t bitsPerSample = 16;
    fwrite(&bitsPerSample, 2, 1, file); // Bits pro Sample: 16
    fwrite("data", 1, 4, file);
    fwrite(&num_samples, 4, 1, file);

    // Daten schreiben
    fwrite(data, sizeof(int16_t), num_samples, file);

    fclose(file);
    free(data);
    printf("Aufnahme erfolgreich als 'aufnahme.wav' gespeichert.\n");
    return 0;
}
