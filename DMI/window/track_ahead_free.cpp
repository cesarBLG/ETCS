#include "track_ahead_free.h"
#include "../tcp/server.h"
bool display_taf;
Component question_part(162, 50);
Component answer_part(82, 50);
void build_taf(window *w)
{
    w->addToLayout(&question_part, new RelativeAlignment(nullptr, 334, 15+50, 0));
    w->addToLayout(&answer_part, new ConsecutiveAlignment(&question_part, RIGHT, 0));
    question_part.clear();
    question_part.addImage("symbols/Driver Request/DR_02.bmp");
    question_part.setBackgroundColor(DarkGrey);
    question_part.addBorder(MediumGrey);
    answer_part.setBackgroundColor(MediumGrey);
    answer_part.addText(gettext("Yes"),0,0,12,Black);
    answer_part.setPressedAction([](){write_command("json",R"({"DriverSelection":"TrackAheadFree"})");});
}
window taf_window(build_taf);