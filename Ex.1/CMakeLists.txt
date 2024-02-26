cmake_minimum_required(VERSION 3.27)
project(untitled C)

set(CMAKE_C_STANDARD 11)

add_executable(untitled "debugExample/divisionByZero.c"
        "Pytagory triangle/pythagoreanTriangle.c"
        TarCompressEncrypt/myzip.c
        ../Ex.2/complexClient.c
        ../Ex.2/Base64Decode.c
        ../Ex.2/Base64Encode.c


)
