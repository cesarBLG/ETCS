#include "distance.h"
#include <limits>
std::set<double*> distance::distances;
distance d_maxsafefront(0);
distance d_minsafefront(0);
distance d_estfront(0);
extern double V_est;
void update_train_position()
{
    d_maxsafefront += 1.001*0.05*V_est*3;
    d_minsafefront += 0.999*0.05*V_est*3;
    d_estfront += 1*0.05*V_est*3;
}
