/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "dns.h"
#include <ares.h>
#ifndef _WIN32
#include "console_fd_poller.h"
#include <signal.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <netdb.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
class AresQuery : public DNSQuery
{
    FdPoller &poller;
    ares_channel channel;
    PlatformUtil::Promise<void> timeout;
    std::map<int, PlatformUtil::Promise<short>> promises;
    std::optional<std::string> a;
    std::optional<std::string> txt;
    static void callback_a(void *arg, int status, int timeouts, struct hostent *host)
    {
        AresQuery *q = (AresQuery*)arg;        
        if (status == ARES_SUCCESS && host->h_length > 0) {
            char ip[INET_ADDRSTRLEN];
            ares_inet_ntop(host->h_addrtype, host->h_addr_list[0], ip, INET_ADDRSTRLEN);
            q->a = ip;
        } else {
            q->a = "";
        }
        if (q->a && q->txt) {
            q->fulfiller.fulfill({*q->a, *q->txt});
        }
    }
    static void callback_txt(void *arg, int status, int timeouts, unsigned char *buff, int alen)
    {
        AresQuery *q = (AresQuery*)arg;    
        q->txt = "";
        if (status == ARES_SUCCESS) {
            ares_txt_reply *reply;
            if (ares_parse_txt_reply(buff, alen, &reply) == ARES_SUCCESS) {
                q->txt = std::string((char*)reply->txt, reply->length);
                ares_free_data(reply);
            }
        }
        if (q->a && q->txt) {
            q->fulfiller.fulfill({*q->a, *q->txt});
        }
    }
public:
    AresQuery(FdPoller &poller, std::string hostname) : DNSQuery(hostname), poller(poller)
    {
        ares_init(&channel);
        ares_set_servers_csv(channel, "8.8.8.8,8.8.4.4");
        ares_gethostbyname(channel, hostname.c_str(), AF_INET, callback_a, this);
        ares_query(channel, hostname.c_str(), 1, 16, callback_txt, this);
        process();
    }
    ~AresQuery()
    {
        promise = {};
        ares_destroy(channel);
    }
protected:
    void process()
    {
        promises.clear();
        ares_socket_t fds[ARES_GETSOCK_MAXNUM];
        int flags = ares_getsock(channel, fds, ARES_GETSOCK_MAXNUM);
        for (int i=0; i<ARES_GETSOCK_MAXNUM; i++) {
            ares_socket_t fd = fds[i];
            if (!ARES_GETSOCK_READABLE(flags, i) && !ARES_GETSOCK_WRITABLE(flags, i))
                continue;
            promises[fd] = poller.on_fd_ready(fd, (ARES_GETSOCK_READABLE(flags, i) ? POLLIN : 0)
            | (ARES_GETSOCK_WRITABLE(flags, i) ? POLLOUT : 0)).then([this, fd](int rev){
                ares_process_fd(channel, (rev & POLLIN) ? fd : ARES_SOCKET_BAD, (rev & POLLOUT) ? fd : ARES_SOCKET_BAD);
                promises.erase(fd);
                if (promises.empty())
                    process();
            });
        }
        timeval req;
        req.tv_sec = 1;
        req.tv_usec = 0;
        timeval tv;
        tv = *ares_timeout(channel, &req, &tv);
        timeout = platform->delay(tv.tv_sec*1000 + tv.tv_usec/1000).then([this](){process();});
    }
};