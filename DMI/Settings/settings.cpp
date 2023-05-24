#include "settings.h"
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <algorithm>

std::map<std::string, std::string>  Settings::items;

void Settings::Init()
{
    std::string line;
    std::ifstream file;
    file.open("settings.ini");

    if (!file.is_open()) {
        perror("Settings file error...");
        return;
    }

    items.clear();

    while (std::getline(file, line)) {
        int pos = line.find('=', 0);
        items.insert(std::pair<std::string, std::string>(line.substr(0, pos), line.substr(pos+1)));
    }

    std::cout << items.size() << " settings variables loaded!" << std::endl;
}

std::string Settings::Get(std::string key)
{
    try
    {
        return items.at(key);
    }
    catch(const std::exception& e)
    {
        return key;
    }
}
