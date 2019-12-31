#include "../MA/movement_authority.h"
#include "../Supervision/supervision.h"
#include "../Supervision/speed_profile.h"
void update_mode_status()
{
    if (MA != nullptr) {
        mode = Mode::FS;
    }
}