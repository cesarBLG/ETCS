#include "language.h"
#include "moFileReader.h"
#include "../DMI/dmi.h"
#include <iostream>
moFileLib::moFileReader reader;
std::string language;
std::string gettext(std::string id)
{
    if (language == "en") return id;
    return reader.Lookup(id.c_str());
}
void set_language(std::string lang)
{
    if (lang == "en") {
        language = "en";
    } else if (reader.ReadFile(("../locales/dmi/"+lang+".mo").c_str()) != moFileLib::moFileReader::EC_SUCCESS) {
        std::cout<<reader.GetErrorDescription()<<std::endl;
        language = "en";
    } else {
        language = lang;
    }
    send_command("language", lang);
}