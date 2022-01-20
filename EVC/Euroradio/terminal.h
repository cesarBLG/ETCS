#pragma once
#include <mutex>
#include <atomic>
#include <deque>
#include <condition_variable>
#include "../Packets/radio.h"
class communication_session;
struct contact_info
{
    unsigned int country;
    unsigned int id;
    uint64_t phone_number;
    bool operator<(const contact_info &o) const
    {
        if (country == o.country) {
            if (id == o.id)
                return phone_number<o.phone_number;
            return id < o.id;
        }
        return country < o.country;
    }
    bool operator==(const contact_info &o) const
    {
        return country == o.country && id == o.id && phone_number == o.phone_number;
    }
};
enum struct safe_radio_status
{
    Disconnected,
    Failed,
    Connected
};
struct mobile_terminal
{
    communication_session *active_session;
    std::atomic<bool> setting_up;
    std::atomic<int> released;
    std::deque<std::shared_ptr<euroradio_message_traintotrack>> pending_write;
    std::deque<std::shared_ptr<euroradio_message>> pending_read;
    std::mutex mtx;
    std::condition_variable cv;
    public:
    std::atomic<safe_radio_status> status;
    std::string radio_network_id;
    bool registered;
    bool setup(communication_session *session);
    void release();
    void update();
};
extern mobile_terminal mobile_terminals[2];