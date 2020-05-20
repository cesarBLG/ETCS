#ifndef _DMI_H
#define _DMI_H
#include <string>
void start_dmi();
void send_command(std::string command, std::string value);
#endif