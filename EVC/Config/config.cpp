#include "config.h"
#include "../DMI/dmi.h"
#include "../Procedures/level_transition.h"
#include <nlohmann/json.hpp>
#include <fstream>
using json = nlohmann::json;
extern std::string traindata_file;
extern int data_entry_type;
void load_config(std::string serie)
{
#ifdef __ANDROID__
    extern std::string filesDir;
    std::ifstream file(filesDir+"/config.json");
#else
    std::ifstream file("config.json");
#endif
    traindata_file = "traindata.json";
    data_entry_type = 0;
    json j;
    file >> j;
    if (j.contains(serie)) {
        json &cfg = j[serie];
        if (cfg.contains("TrainData")) {
            traindata_file = cfg["TrainData"];
            data_entry_type = 1;
        }
    }
    if (serie == "130" || serie == "730") {
        ntc_available_no_stm.insert(0);
    }
    send_command("setSerie", serie);
}