/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "train_data.h"
#include "conversion_model.h"
#include <nlohmann/json.hpp>
#include "../Supervision/speed_profile.h"
#include "../TrainSubsystems/brake.h"
#include <list>
#include "platform_runtime.h"
using json = nlohmann::json;
enum Electrifications
{
    NonElectrical,
    DC600_750V,
    DC1500V,
    DC3KV,
    AC15KV,
    AC25KV
};
struct traction_type
{
    Electrifications electrification;
    int additional_info;
};
double A_ebmax;
double L_TRAIN=0;
double T_traction_cutoff = 0.1;
double M_rotating_nom;
double V_train = 0;
bool Q_airtight = false;
int axle_number=12;
int brake_percentage=0;
int cant_deficiency=0;
std::set<int> other_train_categories;
brake_position_types brake_position = PassengerP;
bool train_data_valid = false;
std::string axle_load_category;
std::string loading_gauge;
std::string train_category;
std::string special_train_data;
std::list<traction_type> traction_systems;
std::string traindata_file = "traindata.txt";
void set_train_data(std::string spec)
{
    if (special_train_data != spec) {
        //trigger_brake_reason(3);
    }
    train_data_valid = false;
    special_train_data = spec;
    conversion_model_used = false;
    if (!special_train_data.empty()) {
        json j;
        std::string contents = platform->read_file(traindata_file);
        if (!contents.empty())
            j = json::parse(contents);
        if (j.contains(special_train_data)) {
            json &traindata = j[special_train_data];
            set_brake_model(traindata);
            L_TRAIN = traindata["length"].get<double>();
            V_train = traindata["speed"].get<double>()/3.6;
            cant_deficiency = (int)traindata["cant_deficiency"].get<double>();
            T_traction_cutoff = traindata["t_traction_cutoff"].get<double>();
            Q_airtight = traindata["airtight"].get<int>();
            json &tracts = traindata["traction_systems"];
            traction_systems.clear();
            for (auto it = tracts.begin(); it != tracts.end(); ++it) {
                std::string name = (*it)["name"].get<std::string>();
                Electrifications elec;
                if (name == "none")
                    elec = NonElectrical;
                else if (name == "DC600/750V")
                    elec = DC600_750V;
                else if (name == "DC1.5kV")
                    elec = DC1500V;
                else if (name == "DC3kV")
                    elec = DC3KV;
                else if (name == "AC15kV")
                    elec = AC15KV;
                else if (name == "AC25kV")
                    elec = AC25KV;
                int info = (*it)["nid_ctraction"].get<int>();
                traction_systems.push_back({elec,info});
            }
            train_data_valid = true;
        }
    }
    set_train_max_speed(V_train);
}
