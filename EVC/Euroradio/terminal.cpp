#include "terminal.h"
#include "session.h"
#ifndef _WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#else
#include <winsock2.h>
#endif
#include <thread>
mobile_terminal mobile_terminals[2];
bool mobile_terminal::setup(communication_session *session)
{
    if (released > 0 || !registered)
        return false;
    setting_up = true;
    active_session = session;
    released = 2;
    std::thread thr([this]{
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            status = safe_radio_status::Failed;
            released = 0;
            perror("socket");
            std::this_thread::sleep_for(std::chrono::seconds(1));
            setting_up = false;
            return;
        }
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(active_session->contact.phone_number);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            status = safe_radio_status::Failed;
            perror("connect");
            released = 0;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            setting_up = false;
            return;
        }
        status = safe_radio_status::Connected;
        setting_up = false;
        std::thread write([this,fd]() {
            while (status == safe_radio_status::Connected) {
                std::unique_lock<std::mutex> lck(mtx);
                while (!pending_write.empty()) {
                    auto msg = pending_write.front();
                    pending_write.pop_front();
                    lck.unlock();
                    bit_manipulator w;
                    msg->write_to(w);
                    int size = (w.bits.size()+7)/8;
                    unsigned char buff[size];
                    w.get_bytes(buff);
                    int result = ::send(fd, (char*)buff, size, 0);
                    if (result < 0) {
                        released--;
                        if (status == safe_radio_status::Connected)
                            status = safe_radio_status::Failed;
                        cv.notify_all();
                        return;
                    }
                    lck.lock();
                }
                cv.wait(lck);
            }
#ifdef _WIN32
            shutdown(fd, SD_BOTH);
            closesocket(fd);
#else
            shutdown(fd, SHUT_RDWR);
            close(fd);
#endif
            released--;
        });
        write.detach();
        while (status == safe_radio_status::Connected) {
            unsigned char head[3];
            if (recv(fd, (char*)head, 3, 0) < 0)
                break;
            int size = (head[1]<<2)|(head[2]>>6);
            unsigned char pack[size];
            pack[0] = head[0];
            pack[1] = head[1];
            pack[2] = head[2];
            int res = recv(fd, (char*)pack+3, size-3, 0);
            if (res != size-3)
                break;
            bit_manipulator r(pack, size);
            std::shared_ptr<euroradio_message> msg = euroradio_message::build(r);
            std::unique_lock<std::mutex> lck(mtx);
            pending_read.push_back(msg);
        }
        released--;
        if (status == safe_radio_status::Connected)
            status = safe_radio_status::Failed;
        cv.notify_all();
    });
    thr.detach();
    return true;
}
void mobile_terminal::release()
{
    {
        std::unique_lock<std::mutex> lck(mtx);
        pending_read.clear();
        pending_write.clear();
    }
    setting_up = false;
    status = safe_radio_status::Disconnected;
    active_session = nullptr;
    cv.notify_all();
}