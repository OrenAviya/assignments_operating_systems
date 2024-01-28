#ifndef OS_MATALA1_ENCRIPTOR_H
#define OS_MATALA1_ENCRIPTOR_H

#include <stdio.h>


typedef struct {
    char encodingKey[62];
} PolyAlphabeticCodec;

void* createCodec(char key[62]);
int encode(char* textin, char* textout, int len, void* codec);
int decode(char* textin, char* textout, int len, void* codec);
void freeCodec(void* codec);
char *readFile(const char *filename, size_t *length);
void writeToFile(const char *filename, const char *textout) ;

#endif //OS_MATALA1_ENCRIPTOR_H
