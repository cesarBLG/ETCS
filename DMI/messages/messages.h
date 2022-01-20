/*
 * European Train Control System
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <string>
using namespace std;
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
    string text;
    Message(unsigned int id, string text, int hour, int minute, 
            bool firstGroup = false, bool ack = false, int reason = 0) : 
            Id(id), hour(hour), minute(minute), firstGroup(firstGroup), ack(ack), reason(reason),
            shown(false), text(text)
    {

    }
};
void addMsg(Message m);
void revokeMessage(unsigned int id);
void displayMessages();
void updateMessages();