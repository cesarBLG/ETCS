/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include "platform_runtime.h"
struct dns_entry
{
    std::string a;
    std::string txt;
};
class DNSQuery
{
protected:
    std::string hostname;
    PlatformUtil::Fulfiller<dns_entry> fulfiller;
public:
    PlatformUtil::Promise<dns_entry> promise;
    DNSQuery(std::string hostname) : hostname(hostname)
    {
        auto pair = PlatformUtil::PromiseFactory::create<dns_entry>();
        promise = std::move(pair.first);
        fulfiller = std::move(pair.second);
    }
    virtual ~DNSQuery() {}
};