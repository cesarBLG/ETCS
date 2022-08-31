#include "../Supervision/speed_profile.h"
#include "../Position/linking.h"
#include "stored_information.h"
#include "../MA/movement_authority.h"
#include "../Packets/radio.h"
void svl_shorten(char condition)
{
    distance d = *SvL_ma;
    // MA already shortened
    delete_linking(d);
    delete_gradient(d);
    if (condition != 'a' && condition != 'b' && condition != 'f')
    if (level == Level::N2 || level == Level::N3)
        ma_rq_reasons[3] = true;
}
void train_shorten(char condition)
{
    distance d = d_maxsafefront(odometer_orientation, 0);
    delete_linking(d);
    delete_gradient(d);
    delete_MA(d_estfront, d);
    if (level == Level::N2 || level == Level::N3)
        ma_rq_reasons[3] = true;
}
void desk_closed_som()
{
    /*delete_linking();
    delete_TSRs();
    recalculate_MRSP();*/
}