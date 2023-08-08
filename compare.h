#ifndef COMPARE_H
#define COMPARE_H

#include <stdint.h>

double calculateRMSE(const int16_t *data1, const int16_t *data2, int num_samples);
int16_t* readWavFile(const char* filename, int* num_samples);

#endif
