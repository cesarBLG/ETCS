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
    int Id;
    int hour;
    int minute;
    bool ack;
    int reason;
    bool firstGroup;
    bool shown;
    string text;
    Message(int id, string text, int hour, int minute, 
            bool firstGroup = false, bool ack = false, int reason = 0) : 
            Id(id), hour(hour), minute(minute), firstGroup(firstGroup), ack(ack), reason(reason),
            shown(false), text(text)
    {

    }
};
void addMsg(Message m);
void revokeMessage(int id);
void displayMessages();