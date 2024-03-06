//
// Created by ofir on 2/28/24.
//

#ifndef ASSIGNMENTS_OPERATING_SYSTEMS_BASE64_H
#define ASSIGNMENTS_OPERATING_SYSTEMS_BASE64_H


int calcDecodeLength(const char* b64input);
int Base64Decode(char* b64message, char** buffer);
int Base64Encode(const char* message, char** buffer);

#endif //ASSIGNMENTS_OPERATING_SYSTEMS_BASE64_H
