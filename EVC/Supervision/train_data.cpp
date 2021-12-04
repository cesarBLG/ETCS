/*
 * European Train Control System
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "train_data.h"
#include "conversion_model.h"
#include "../Parser/nlohmann/json.hpp"
#include "../Supervision/speed_profile.h"
#include <fstream>
#include <list>
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
bool valid = false;
std::string special_train_data;
std::list<traction_type> traction_systems;
bool train_data_valid()
{
    return valid && L_TRAIN > 0 && V_train > 0;
}
void validate_train_data()
{
    if (!special_train_data.empty()) {
#ifdef __ANDROID__
        extern std::string filesDir;
        std::ifstream file(filesDir+"/traindata.txt");
#else
        std::ifstream file("traindata.txt");
#endif
        json j;
        file >> j;
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
            valid = true;
        }
    } else {
        valid = true;
    }
    set_train_max_speed(V_train);
    if (!train_data_valid()) 
        valid = false;
    else
        set_conversion_model();
}
void invalidate_train_data()
{
    valid = false;
}
