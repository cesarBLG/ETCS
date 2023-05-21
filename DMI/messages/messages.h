/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <string>
#include "../graphics/color.h"
/*enum DriverMessage
{
    BaliseReadError,
    TracksideMalfunction,
    CommunicationError,
    EnteringFS,
    EnteringOS,
    RunawayMovement,
    SHrefused,
    SHrequestFailed,
    TracksideNotCompatible,
    TrainDataChanged,
    TrainIsRejected,
    UnauthorizedPassingEOA,
    NoMAreceived,
    SRdistanceExceeded,
    SHstopOrder,
    SRstopOrder,
    EmergencyStop,
    RVdistanceExceeded,
    NoTrackDescription,
    NTCbrakeDemand,
    RouteUnsuitableLoadingGauge,
    RouteUnsuitableTractionSystem,
    RouteUnsuitableAxleLoadCategory,
    RadioNetworkRegistrationFailed,
    NTCnotAvailable,
    NTCneedsData,
    NTCfailed,
    AcknowledgeSR
};*/
struct Message
{
    unsigned int Id;
    int hour;
    int minute;
    bool ack;
    int reason;
    bool firstGroup;
    bool shown;
    std::string text;
    Color fgColor;
    Color bgColor;
    Message() = default;
    Message(unsigned int id, std::string text, int hour, int minute, 
            bool firstGroup = false, bool ack = false, int reason = 0, Color fgColor = White, Color bgColor = DarkBlue) : 
            Id(id), hour(hour), minute(minute), firstGroup(firstGroup), ack(ack), reason(reason),
            shown(false), text(text), fgColor(fgColor), bgColor(bgColor)
    {
    }
};
void addMsg(Message m);
void revokeMessage(unsigned int id);
void displayMessages();
void updateMessages();