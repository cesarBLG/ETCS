#include "logging.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <condition_variable>
#include <mutex>
#ifndef _WIN32
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#else
#include <winsock2.h>
#endif
static int sock;
static std::mutex mtx;
static std::condition_variable cv;
std::deque<std::string> pending_logs;
void start_logging()
{
    std::thread thr([]{
        int server = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in serv;
        serv.sin_family = AF_INET;
        serv.sin_port = htons(5011);
        serv.sin_addr.s_addr = INADDR_ANY;
        if (server == -1) {
            perror("socket");
            return;
        }
        if (0 != ::bind(server, (struct sockaddr *)&(serv), sizeof(serv))) {
            perror("bind");
            return;
        }
        if (listen(server, 1) == -1) {
            perror("listen");
            return;
        }
        extern bool run;
        sock = -1;
        while(sock == -1 && run)
        {
            struct sockaddr_in addr;
            int c = sizeof(struct sockaddr_in);
            int cl = accept(server, (struct sockaddr *)&addr, 
    #ifdef _WIN32
            (int *)
    #else
            (socklen_t *)
    #endif
            &c);
            if(cl == -1) {
                perror("accept");
                continue;
            }
            sock = cl;
            while(sock >= 0 && run) {
                std::unique_lock<std::mutex> lck(mtx);
                while (!pending_logs.empty()) {
                    std::string str = pending_logs.front();
                    pending_logs.pop_front();
                    lck.unlock();
                    if (send(sock, str.c_str(), str.size(), 0) < 0)
                        sock = -1;
                    lck.lock();
                }
                if (sock >= 0) cv.wait(lck);
            }
        }
    });
    thr.detach();
}
void print_vars(std::ostream &stream, std::vector<std::pair<std::string,std::string>> vars)
{
    for (auto &var : vars)
    {
        stream<<var.first<<'\t'<<var.second<<'\n';
    }
}
void log_message(std::shared_ptr<ETCS_message> msg, distance &dist, int64_t time)
{
    std::stringstream ss;
    bit_manipulator b;
    msg->write_to(b);
    ss<<"Distance: "<<(dist.get()+odometer_reference)<<"\t Time: "<<time<<"\n";
    print_vars(ss, b.log_entries);
    ss<<std::endl;
    std::string str = ss.str();
    std::cout<<str;
    std::unique_lock<std::mutex> lck(mtx);
    pending_logs.push_back(str);
    lck.unlock();
    cv.notify_all();
}