#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

// Function to generate a random passphrase
/*void generateRandomPassphrase(char passphrase[], size_t length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    const size_t charset_size = sizeof(charset) - 1;

    srand((unsigned int) time(NULL));  // Explicitly cast to unsigned int

    for (size_t i = 0; i < length; ++i) {
        passphrase[i] = charset[rand() % charset_size];
    }

    passphrase[length] = '\0';  // Null-terminate the string
}*/

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *directory = argv[1];
    char temp_tar_file[] = "/tmp/temp_tar_fileXXXXXX";
    char temp_compress_file[] = "/tmp/temp_compress_fileXXXXXX";
    char output_file[] = "output.gpg";
    char passphrase_file[] = "/tmp/passphrase_fileXXXXXX";

    // Create tar archive
    char tar_cmd[256];
    sprintf(tar_cmd, "tar -cf %s %s", temp_tar_file, directory);
    if (system(tar_cmd) != 0) {
        error("Error creating tar archive");
    }

    // Compress the tar archive
    char compress_cmd[256];
    sprintf(compress_cmd, "gzip -c %s > %s", temp_tar_file, temp_compress_file);
    if (system(compress_cmd) != 0) {
        fprintf(stderr, "Error compressing file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }


    // Create a temporary file to store the passphrase
    int passphrase_fd = mkstemp(passphrase_file);
    if (passphrase_fd == -1) {
        error("Error creating passphrase file");
    }

    // Generate a random passphrase
   /* char passphrase[16];  // Adjust the length as needed
    generateRandomPassphrase(passphrase, sizeof(passphrase) - 1);

    // Write the passphrase to the temporary file
    if (write(passphrase_fd, passphrase, strlen(passphrase)) == -1) {
        error("Error writing passphrase to file");
    }*/

    // Encrypt the compressed file using the passphrase file
    char gpg_cmd[256];
    sprintf(gpg_cmd, "gpg -c --passphrase-fd %d -o %s %s", passphrase_fd, output_file, temp_compress_file);
    if (system(gpg_cmd) != 0) {
        fprintf(stderr, "Error encrypting file with GPG. Command: %s\n", gpg_cmd);
        exit(EXIT_FAILURE);
    }

    // Clean up temporary files
    unlink(temp_tar_file);
    unlink(temp_compress_file);
    unlink(passphrase_file);

    printf("myzip completed successfully. Encrypted and compressed file: %s\n", output_file);

    return 0;
}


