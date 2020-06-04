#include "../Supervision/speed_profile.h"
#include "../Position/linking.h"
void svl_shorten(int condition)
{
    distance d = *SvL;
}
void train_shorten(int condition)
{
    SvL = d_maxsafefront(0);
    EoA = d_estfront;
    distance d = d_maxsafefront(0);
    delete_linking(d);
    delete_ma(d);
    delete_gradient(d);
    recalculate_MRSP();
}
void desk_closed_som()
{
    delete_linking();
    delete_TSRs();
    recalculate_MRSP();
}