#include <stdio.h>
#include <stdlib.h>
#include "encriptor.h"


int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s src_file dst_file\n", argv[0]);
        return 1;
    }

    const char *src_file = argv[1];
    const char *dst_file = argv[2];


    char key[62] = "defghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abc";
    void* codec = createCodec(key);

    if (codec == NULL) {
        fprintf(stderr, "Error creating codec\n");
        return 1;
    }

    size_t length;

    char *textin = readFile(src_file, &length);
    if (textin != NULL) {
        printf("Text in content:%s\n", textin);
        printf("File length:%zu\n", length);
    } else {
        printf("Error: textin is NULL");
        exit(1);
    }
    char *textout = (char *)malloc(length + 1);
    printf("textout: \n",textout);
    int result = encode(textin, textout,length, codec);
    if (result < 0) {
        fprintf(stderr, "Error encoding\n");
        free(textin);
        free(textout);
        freeCodec(codec);
        exit(1);
    }
    if (textout != NULL) {
        printf("Textout content:%s\n", textout);
    } else {
        printf("Error: textout is NULL");
        free(textin);
        free(textout);
        freeCodec(codec);
        exit(1);
    }

    printf("Eecoding successful\n");
    writeToFile(dst_file, textout);

    size_t len;
    char *text = readFile(dst_file, &len);
    if (text != NULL) {
        printf("Dst file content:%s\n", text);
        printf("Dst file length:%zu\n", length);
        free(text);
    } else {
        printf("Error: textout is NULL");
        exit(1);
    }

    return 0;
}
