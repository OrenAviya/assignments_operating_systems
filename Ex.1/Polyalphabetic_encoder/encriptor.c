#include <stdlib.h>
#include <string.h>
#include "encriptor.h"
#include <stdio.h>


void* createCodec(char key[62]) {
    PolyAlphabeticCodec* codec = (PolyAlphabeticCodec*)malloc(sizeof(PolyAlphabeticCodec));
    if (codec == NULL) {
        return NULL;
    }

    // Validate key
    int usedChars[256] = {0}; // Assuming ASCII characters
    for (int i = 0; i < 62; ++i) {
        if (usedChars[key[i]] == 1) {
            free(codec);
            return NULL; // Invalid key
        }
        usedChars[key[i]] = 1;
    }

    // Copy the key to the codec structure
    strncpy(codec->encodingKey, key, 62);

    return codec;
}

int encode(char* textin, char* textout, int len, void* codec) {
    if (textin == NULL || textout == NULL || codec == NULL) {
        return -1;
    }

    PolyAlphabeticCodec* polyCodec = (PolyAlphabeticCodec*)codec;

    for (int i = 0; i < len; ++i) {
        char currentChar = textin[i];

        // Check if the character is a lowercase letter
        if ('a' <= currentChar && currentChar <= 'z') {
            textout[i] = polyCodec->encodingKey[currentChar - 'a'];
        }
            // Check if the character is an uppercase letter
        else if ('A' <= currentChar && currentChar <= 'Z') {
            textout[i] = polyCodec->encodingKey[currentChar - 'A' + 26];
        }
            // If it's not a letter, leave it unchanged
        else {
            textout[i] = currentChar;
        }
    }

    // Null-terminate the encoded string
    textout[len] = '\0';

    return 0;
}


int decode(char* textin, char* textout, int len, void* codec) {
    // Check for invalid input
    if (textin == NULL || textout == NULL || codec == NULL) {
        return -1;
    }

    PolyAlphabeticCodec* polyCodec = (PolyAlphabeticCodec*)codec;

    // Implement decoding logic
    for (int i = 0; i < len; ++i) {
        char currentChar = textin[i];

        // Check if the character is a lowercase letter
        if ('a' <= currentChar && currentChar <= 'z') {
            // Find the index of the character in the encoding key
            int index = strchr(polyCodec->encodingKey, currentChar) - polyCodec->encodingKey;
            textout[i] = 'a' + index;
        }
            // Check if the character is an uppercase letter
        else if ('A' <= currentChar && currentChar <= 'Z') {
            // Find the index of the character in the encoding key
            int index = strchr(polyCodec->encodingKey + 26, currentChar) - (polyCodec->encodingKey + 26);
            textout[i] = 'A' + index;
        }
            // Check if the character is a digit
        else if ('0' <= currentChar && currentChar <= '9') {
            // Find the index of the character in the encoding key
            int index = strchr(polyCodec->encodingKey + 52, currentChar) - (polyCodec->encodingKey + 52);
            textout[i] = '0' + index;
        }
            // For non-alphanumeric characters, leave them unchanged
        else {
            textout[i] = currentChar;
        }

    }
    textout[len] = '\0';

    return 0;
}

void freeCodec(void* codec) {
    if (codec != NULL) {
        free(codec);
    }
}


char *readFile(const char *filename, size_t *length) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    // Get the size of the file
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the content
    char *textin = (char *)malloc(fileSize + 1);  // +1 for null terminator
    if (textin == NULL) {
        perror("Error allocating memory");
        fclose(file);
        return NULL;
    }

    // Read the content of the file
    size_t bytesRead = fread(textin, 1, fileSize, file);
    if (bytesRead != fileSize) {
        perror("Error reading file");
        free(textin);
        fclose(file);
        return NULL;
    }

    // Null-terminate the string
    textin[fileSize] = '\0';

    // Update the length
    if (length != NULL) {
        *length = fileSize;
    }

    // Close the file
    fclose(file);

    return textin;
}


void writeToFile(const char *filename, const char *textout) {
    FILE *fp = fopen(filename, "w");

    if (fp == NULL) {
        fprintf(stderr, "Error opening file '%s' for writing.\n", filename);
        exit( -1);
    }

    fprintf(fp, "%s", textout);

    fclose(fp);
}