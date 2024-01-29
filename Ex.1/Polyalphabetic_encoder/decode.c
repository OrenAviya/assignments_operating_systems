
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
    // Read the content of the source file into textin (for decode)
    char *textin = readFile(dst_file, &length);
    if (textin != NULL) {
        // prints for checking...
            // printf("Text in content:%s\n", textin);
            // printf("File length:%zu\n", length);
    } else {
        printf("Error: textin is NULL");
        exit(1);
    }
    // Allocate memory for the encoded textout
    char *textout = (char *)malloc(length + 1);

    // decode the textin using the decode function
    int result = decode(textin,textout, length, codec);
    if (result < 0) {
        fprintf(stderr, "Error decoding\n");
        free(textin);
        free(textout);
        freeCodec(codec);
        return 1;
    }
    if (textout != NULL) {
        // checking print
         // printf("Textout content:%s\n", textout);
    } else {
        printf("Error: textout is NULL");
        free(textin);
        free(textout);
        freeCodec(codec);
        exit(1);
    }

    printf("Decoding successful\n");
    writeToFile(src_file, textout);

    // checking ourselves:
    /*size_t len;
    char *text = readFile(src_file, &len);
    if (text != NULL) {
        printf("Src file content:%s\n", text);
        printf("Src file length:%zu\n", length);
        free(text);
        } else {
        printf("Error: textout is NULL");
        exit(1);}
        */

    return 0;
}
