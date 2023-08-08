#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "compare.h"

double calculateRMSE(const int16_t *data1, const int16_t *data2, int num_samples)
{
    double sumSquared = 0.0;
    for (int i = 0; i < num_samples; i++)
    {
        // Debug-Ausgabe der Daten zum Zeitpunkt der Berechnung
        printf("Sample %d - data1: %d, data2: %d\n", i, data1[i], data2[i]);

        int diff = data1[i] - data2[i];
        sumSquared += (diff * diff);
    }

    double meanSquared = sumSquared / num_samples;
    return sqrt(meanSquared);
}

int16_t* copyData(const int16_t* source, int num_samples) {
    int16_t* dest = (int16_t*)malloc(sizeof(int16_t) * num_samples);
    if (!dest) {
        printf("Fehler beim Allokieren des Pufferspeichers für die Datenkopie\n");
        return NULL;
    }
    
    for (int i = 0; i < num_samples; i++) {
        dest[i] = source[i];
    }
    
    return dest;
}

int16_t* readWavFile(const char* filename, int* num_samples) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Fehler beim Öffnen der Datei '%s'\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 44, SEEK_SET); // WAV-Header überspringen

    *num_samples = (fileSize - 44) / sizeof(int16_t);
    int16_t* data = (int16_t*)malloc(fileSize - 44);
    if (!data) {
        printf("Fehler beim Allokieren des Pufferspeichers für die Audiodaten\n");
        fclose(file);
        return NULL;
    }

    fread(data, sizeof(int16_t), *num_samples, file);
    fclose(file);
    return data;
}
