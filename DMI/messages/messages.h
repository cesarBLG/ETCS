#include <string>
using namespace std;
enum DriverMessage
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
};
struct Message
{
    int Id;
    DriverMessage message;
    int hour;
    int minute;
    bool ack;
    bool tripReason;
    bool firstGroup;
    bool shown;
    string text;
    Message(int id, DriverMessage type, int hour, int minute, 
            bool firstGroup = false, bool ack = false, bool tripReason = false, string text = "") : 
            Id(id), message(type), hour(hour), minute(minute), firstGroup(firstGroup), ack(ack), tripReason(tripReason),
            shown(false), text(text)
    {

    }
};
void addMsg(Message m);
void displayMessages();