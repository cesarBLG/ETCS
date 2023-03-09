#include "language.h"
#include "moFileReader.h"
#include "../graphics/text_button.h"
#include <iostream>
std::string language;
moFileLib::moFileReader reader;
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
    extern TextButton main_button;
    extern TextButton override_button;
    extern TextButton dataview_button;
    extern TextButton special_button;
    main_button.rename(gettext("Main"));
    override_button.rename(gettext("Over-\nride"));
    dataview_button.rename(gettext(gettext("Data\nview")));
    special_button.rename(gettext("Spec"));
}