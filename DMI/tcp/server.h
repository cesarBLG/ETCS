#ifndef _SERVER_H
#define _SERVER_H
#include <string>
void startSocket();
void closeSocket();
void write_command(std::string command, std::string value);
#endif