#include "language.h"
#include <moFileReader.hpp>
#include "../graphics/text_button.h"
#include <iostream>
std::string language = "en";
moFileLib::moFileReader reader;
std::string get_text(std::string id)
{
    if (language == "en") return id;
    return reader.Lookup(id.c_str());
}
void set_language(std::string lang)
{
    std::string file = "../locales/dmi/"+lang+".mo";
#ifdef __ANDROID__
    extern std::string filesDir;
    file = filesDir+"/locales/dmi/"+lang+".mo";
#endif
    if (lang == "en") {
        language = "en";
    } else if (reader.ReadFile(file.c_str()) != moFileLib::moFileReader::EC_SUCCESS) {
        std::cout<<reader.GetErrorDescription()<<std::endl;
        language = "en";
    } else {
        language = lang;
    }
    extern TextButton main_button;
    extern TextButton override_button;
    extern TextButton dataview_button;
    extern TextButton special_button;
    main_button.rename(get_text("Main"));
    override_button.rename(get_text("Over-\nride"));
    dataview_button.rename(get_text(get_text("Data\nview")));
    special_button.rename(get_text("Spec"));
}