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
bool train_data_known = false;
axle_load_categories axle_load_category;
loading_gauges loading_gauge;
std::string train_category;
std::string special_train_data;
std::list<traction_type> traction_systems;
std::string traindata_file = "traindata.txt";
void set_train_data(std::string spec)
{
    if (special_train_data != spec) {
        //trigger_brake_reason(3);
    }
    train_data_known = true;
    train_data_valid = false;
    special_train_data = spec;
    conversion_model_used = false;
    train_category = "";
    if (!special_train_data.empty()) {
        json j;
        auto contents = platform->read_file(traindata_file);
        if (contents)
            j = json::parse(*contents);
        if (j.contains(special_train_data)) {
            json &traindata = j[special_train_data];
            if (traindata.contains("brake_percentage")) brake_percentage = (int)traindata["brake_percentage"].get<double>();
            set_brake_model(traindata);
            L_TRAIN = traindata["length"].get<double>();
            V_train = traindata["speed"].get<double>()/3.6;
            cant_deficiency = (int)traindata["cant_deficiency"].get<double>();
            T_traction_cutoff = traindata["t_traction_cutoff"].get<double>();
            Q_airtight = traindata["airtight"].get<int>();
            std::string gauge = traindata.contains("loading_gauge") ? traindata["loading_gauge"].get<std::string>() : "";
            if (gauge == "G1")
                loading_gauge = loading_gauges::G1;
            else if (gauge == "GA")
                loading_gauge = loading_gauges::GA;
            else if (gauge == "GB")
                loading_gauge = loading_gauges::GB;
            else if (gauge == "GC")
                loading_gauge = loading_gauges::GC;
            else 
                loading_gauge = loading_gauges::OutGC;
            std::string axleload = traindata.contains("axle_load_category") ? traindata["axle_load_category"].get<std::string>() : "";
            if (axleload == "A")
                axle_load_category = axle_load_categories::A;
            else if (axleload == "HS17")
                axle_load_category = axle_load_categories::HS17;
            else if (axleload == "B1")
                axle_load_category = axle_load_categories::B2;
            else if (axleload == "B2")
                axle_load_category = axle_load_categories::B2;
            else if (axleload == "C2")
                axle_load_category = axle_load_categories::C2;
            else if (axleload == "C3")
                axle_load_category = axle_load_categories::C3;
            else if (axleload == "C4")
                axle_load_category = axle_load_categories::C4;
            else if (axleload == "D2")
                axle_load_category = axle_load_categories::D2;
            else if (axleload == "D3")
                axle_load_category = axle_load_categories::D3;
            else if (axleload == "D4")
                axle_load_category = axle_load_categories::D4;
            else if (axleload == "D4XL")
                axle_load_category = axle_load_categories::D4XL;
            else if (axleload == "E4")
                axle_load_category = axle_load_categories::E4;
            else
                axle_load_category = axle_load_categories::E5;
            json &tracts = traindata["traction_systems"];
            traction_systems.clear();
            for (auto it = tracts.begin(); it != tracts.end(); ++it) {
                std::string name = (*it)["name"].get<std::string>();
                Electrifications elec;
                if (name == "DC600/750V")
                    elec = DC600_750V;
                else if (name == "DC1.5kV")
                    elec = DC1500V;
                else if (name == "DC3kV")
                    elec = DC3KV;
                else if (name == "AC15kV")
                    elec = AC15KV;
                else if (name == "AC25kV")
                    elec = AC25KV;
                else
                    elec = NonElectrical;
                int info = 0;
                if (elec != NonElectrical)
                    info = (*it)["nid_ctraction"].get<int>();
                traction_systems.push_back({elec,info});
            }
            train_data_valid = true;
        }
    }
    set_train_max_speed(V_train);
}
