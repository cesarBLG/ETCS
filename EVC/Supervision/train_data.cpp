#include "train_data.h"
double A_ebmax=1.2;
double L_TRAIN=0;
double T_traction = 0.1;
double T_berem = 0.5;
double M_rotating_nom;
double V_train = 0;
bool Q_airtight = false;
int axle_number=12;
int brake_percentage=150;
int cant_deficiency=0;
std::set<int> other_train_categories;
brake_position_types brake_position = PassengerP;