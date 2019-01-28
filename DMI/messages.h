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
    RadioNetworkRegistrationFalied,
    NTCnotAvailable,
    NTCneedsData,
    NTCfailed
};
struct Message
{
    DriverMessage message;
    bool ack;
    bool acked;
};
const char *messages[] = {
    "Datos de eurobaliza no consistentes",
    "Trackside malfunction",
    "Communication error",
    "Entrada en modo FS",
    "Entrada en modo OS",
    "Runaway movement",
    "SH refused",
    "SH request failed",
    "Trackside not compatible",
    "Train data changed",
    "EoA o LoA rebasado",
    "No MA received at level transition",
    "SR distance exceeded",
    "SH stop order",
    "SR stop order",
    "Emergency stop",
    "RV distance exceeded",
    "No track description",
    "%s brake demand",
    "Route unsuitable – loading gauge",
    "Route unsuitable – traction system",
    "Route unsuitable – axle load category",
    "Radio network registration failed",
    "%s not available",
    "%s needs data",
    "%s failed"
}