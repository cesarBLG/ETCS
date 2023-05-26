#ifndef _Settings_H
#define _Settings_H

#include <map>
#include <string>

class Settings
{
	public:
		static void Init();
		static std::string Get(std::string);
		static std::map<std::string, std::string> items;
};
#endif
