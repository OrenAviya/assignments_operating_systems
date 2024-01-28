//
// Created by aviya on 1/28/24.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <encrypted_file.gpg>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *input_file = argv[1];
    char temp_compress_file[] = "/tmp/temp_compress_fileXXXXXX";
    char temp_tar_file[] = "/tmp/temp_tar_fileXXXXXX";
    char output_directory[] = "unzipped_directory";
    char passphrase[16];

    // Decrypt the encrypted file
    char gpg_cmd[256];
    sprintf(gpg_cmd, "echo \"%s\" | gpg --passphrase-fd 0 -d %s > %s", passphrase, input_file, temp_compress_file);
  //  sprintf(gpg_cmd, "gpg -d %s > %s", input_file, temp_compress_file);
    if (system(gpg_cmd) != 0) {
        fprintf(stderr, "Error decrypting file with GPG: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }


    // Create the output directory if it doesn't exist
    if (mkdir(output_directory, 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "Error creating output directory: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Decompress the decrypted file
    char compress_cmd[256];
    sprintf(compress_cmd, "gzip -d -c %s > %s", temp_compress_file, temp_tar_file);
    if (system(compress_cmd) != 0) {
        fprintf(stderr, "Error decompressing file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Extract the tar archive
    char tar_cmd[256];
    sprintf(tar_cmd, "tar -xf %s -C %s", temp_tar_file, output_directory);
    if (system(tar_cmd) != 0) {
        fprintf(stderr, "Error extracting tar archive: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }


    // Clean up temporary files
    unlink(temp_compress_file);
    unlink(temp_tar_file);

    printf("myunzip completed successfully. Unzipped directory: %s\n", output_directory);

    return 0;
}
