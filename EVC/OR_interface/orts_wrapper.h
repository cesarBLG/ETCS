#include <orts/common.h>

class ORTSClientWrapper : public ORserver::client {
private:
    BasePlatform::BusSocket &bus;
    uint32_t bus_id;
    bool uid;

public:
    virtual void start() override {};
    virtual std::string ReadLine() override { return {}; };
    virtual void handle() override {};

    virtual void WriteLine(std::string line) override {
        if (uid)
            bus.send_to(bus_id, line);
        else
            bus.broadcast(bus_id, line);
    }

    ORTSClientWrapper(BasePlatform::BusSocket &b, uint32_t t, bool u) : bus(b), bus_id(t), uid(u) {}
};
