#include "../Packets/radio.h"
#include "../Euroradio/session.h"
#include "../Supervision/fixed_values.h"
optional<std::pair<distance, distance>> taf_request;
bool start_display_taf;
bool stop_display_taf;
void request_track_ahead_free(distance start, double length)
{
    taf_request = {start, start+length};
    start_display_taf = false;
    stop_display_taf = false;
}
void update_track_ahead_free_request()
{
    if (taf_request && taf_request->second + D_keep_information < d_estfront)
        taf_request = {};
    if (taf_request) {
        if (taf_request->first <= d_estfront)
            start_display_taf = true;
        if (taf_request->second < d_estfront)
            stop_display_taf = true;
    } else {
        start_display_taf = false;
        stop_display_taf = false;
    }
}
void track_ahead_free_granted()
{
    taf_request = {};
    if (supervising_rbc) {
        auto *taf = new taf_granted();
        fill_message(taf);
        supervising_rbc->send(std::shared_ptr<euroradio_message_traintotrack>(taf));
    }
}