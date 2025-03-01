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
#define USE_OLD_CARES
class AresQuery : public DNSQuery
{
    FdPoller &poller;
    ares_channel channel;
    PlatformUtil::Promise<void> timeout;
    std::map<int, PlatformUtil::Promise<short>> promises;
    std::optional<std::string> a;
    std::optional<std::string> txt;
    static void callback_a(void *arg, int status, int timeouts, struct ares_addrinfo *addr)
    {
        AresQuery *q = (AresQuery*)arg;        
        if (status == ARES_SUCCESS && addr->nodes) {
            char ip[INET6_ADDRSTRLEN];
            ares_inet_ntop(addr->nodes->ai_family, &((struct sockaddr_in *)addr->nodes->ai_addr)->sin_addr, ip, INET6_ADDRSTRLEN);
            q->a = ip;
        } else {
            q->a = "";
        }
        if (q->a && q->txt) {
            q->fulfiller.fulfill({*q->a, *q->txt});
        }
    }
#ifdef USE_OLD_CARES
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
#else
    static void callback_txt(void *arg, ares_status_t status, size_t timeouts, const ares_dns_record_t *dnsrec)
    {
        AresQuery *q = (AresQuery*)arg;    
        q->txt = "";
        if (status == ARES_SUCCESS) {
            const ares_dns_rr_t *res = ares_dns_record_rr_get_const(dnsrec, ARES_SECTION_ANSWER, 0);
            if (res != nullptr) {
                size_t len;
                const unsigned char *data = ares_dns_rr_get_bin(res, ARES_RR_TXT_DATA, &len);
                q->txt = std::string((const char *)data, len);
            }
        }
        if (q->a && q->txt) {
            q->fulfiller.fulfill({*q->a, *q->txt});
        }
    }
#endif
    void process_fd(ares_socket_t fd, int rev)
    {
        ares_process_fd(channel, (rev & POLLIN) ? fd : ARES_SOCKET_BAD, (rev & POLLOUT) ? fd : ARES_SOCKET_BAD);
        if (promises.count(fd))
            callback_fds(this, fd, 1, 1);
    }
    static void callback_fds(void *arg, ares_socket_t fd, int readable, int writable)
    {
        AresQuery *q = (AresQuery*)arg;
        if (!readable && !writable) {
            q->promises.erase(fd);
            return;
        }
        q->promises[fd] = q->poller.on_fd_ready(fd, POLLIN|POLLOUT).then([q, fd](int rev) {
                q->process_fd(fd, rev);
            });
    }
public:
    AresQuery(FdPoller &poller, std::string hostname) : DNSQuery(hostname), poller(poller)
    {
        ares_options options;
        options.sock_state_cb = callback_fds;
        options.sock_state_cb_data = this;
        ares_init_options(&channel, &options, ARES_OPT_SOCK_STATE_CB);
        ares_set_servers_csv(channel, "8.8.8.8,8.8.4.4");
        ares_addrinfo_hints hints;
        hints.ai_family = AF_UNSPEC;
        hints.ai_flags = 0;
        hints.ai_protocol = 0;
        hints.ai_socktype = 0;
        ares_getaddrinfo(channel, hostname.c_str(), "30993", &hints, callback_a, this);
#ifdef USE_OLD_CARES
        ares_query(channel, hostname.c_str(), 1, 16, callback_txt, this);
#else
        ares_query_dnsrec(channel, hostname.c_str(), ARES_CLASS_IN, ARES_REC_TYPE_TXT, callback_txt, this, nullptr);
#endif
        process_timeout();
    }
    ~AresQuery()
    {
        promise = {};
        ares_destroy(channel);
    }
protected:
    void process_timeout()
    {
        timeval req;
        req.tv_sec = 1;
        req.tv_usec = 0;
        timeval tv;
        tv = *ares_timeout(channel, &req, &tv);
        timeout = platform->delay(tv.tv_sec*1000 + tv.tv_usec/1000).then([this]() {
            ares_process_fd(channel, ARES_SOCKET_BAD, ARES_SOCKET_BAD);
            process_timeout();
        });
    }
};