#include "data_validation.h"
#include "../tcp/server.h"
void validation_window::sendInformation()
{
    json j = R"({"DriverSelection":"ValidateDataEntry"})"_json;
    j["WindowTitle"] = title;
    for (auto i : validation_data)
    {
        j["DataInputResult"][i->label] = i->data;
    }
    j["DataInputResult"]["Validated"] = confirmation->data_accepted;
    write_command("json", j.dump());
}