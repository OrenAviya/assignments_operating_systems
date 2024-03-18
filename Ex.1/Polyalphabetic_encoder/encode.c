#include <stdio.h>
#include <stdlib.h>
#include "encriptor.h"


int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s src_file dst_file\n", argv[0]);
        return 1;
    }
    // Extract source and destination file paths from command-line arguments
    const char *src_file = argv[1];
    const char *dst_file = argv[2];

    // Encryption key
    char key[62] = "defghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abc";

    // Create a codec using the provided key
    void* codec = createCodec(key);

    // Check if codec creation was successful
    if (codec == NULL) {
        fprintf(stderr, "Error creating codec\n");
        return 1;
    }

    size_t length;

    // Read the content of the source file into textin (for encryption)
    char *textin = readFile(src_file, &length);

    // Check if reading the source file was successful
    if (textin != NULL) {
        printf("Text in content: %s\n", textin);
        printf("File length: %zu\n", length);
    } else {
        printf("Error: textin is NULL");
        exit(1);
    }

    // Allocate memory for the encoded textout
    char *textout = (char *)malloc(length + 1);

    // Check if memory allocation was successful
//    printf("textout: %s\n", textout);

    // Encode the textin using the encode function
    int result = encode(textin, textout, length, codec);

    // Check if encoding was successful
    if (result < 0) {
        fprintf(stderr, "Error encoding\n");
        free(textin);
        free(textout);
        freeCodec(codec);
        exit(1);
    }

    // Check if textout is not NULL and print its content
    if (textout != NULL) {
        printf("Textout content: %s\n", textout);
    } else {
        printf("Error: textout is NULL");
        free(textin);
        free(textout);
        freeCodec(codec);
        exit(1);
    }

    printf("Encoding successful\n");

    // Write the encoded textout to the destination file
    writeToFile(dst_file, textout);

    // Read the content of the destination file and print it
    size_t len;
    char *text = readFile(dst_file, &len);
    if (text != NULL) {
        printf("Dst file content: %s\n", text);
        printf("Dst file length: %zu\n", length);
        free(text);
    } else {
        printf("Error: textout is NULL");
        exit(1);
    }

    // Free allocated memory and exit
    free(textin);
    free(textout);
    freeCodec(codec);

    return 0;
}
