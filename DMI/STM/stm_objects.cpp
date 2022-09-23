#include "../graphics/button.h"
#include "../graphics/component.h"
#include "../graphics/color.h"
#include "../messages/messages.h"
#include "../window/window.h"
std::map<int, std::vector<int>> but_pos; 
std::map<int, std::vector<int>> ind_pos;
void setup_areas()
{
    areas["A1"] = {0, 15, 54, 54};
    areas["A2"] = {0, 69, 54, 30};
    areas["A3"] = {0, 99, 54, 191};
    areas["A4"] = {0, 290, 54, 25};
    areas["B3"] = {140, 271, 36, 36};
    areas["B4"] = {176, 271, 36, 36};
    areas["B5"] = {212, 271, 36, 36};
    areas["C8"] = {0, 315, 54, 25};
    areas["C9"] = {0, 340, 54, 25};
    areas["C2"] = {54, 315, 37, 50};
    areas["C3"] = {91, 315, 37, 50};
    areas["C4"] = {128, 315, 37, 50};
    areas["C1"] = {165, 315, 58, 50};
    areas["C5"] = {223, 315, 37, 50};
    areas["C6"] = {260, 315, 37, 50};
    areas["C7"] = {297, 315, 37, 50};
    for (int i=1; i<=4; i++) {
        areas["E"+std::to_string(i)] = {0, 365 + 20*(i-1), 54, 25};
    }
    for (int i=5; i<=9; i++) {
        areas["E"+std::to_string(i)] = {54, 365 + 20*(i-5), 234, 20};
    }
    areas["E10"] = {288, 365, 46, 50};
    areas["E11"] = {288, 415, 46, 50};
    for (int i=5; i<=9; i++) {
        areas["E"+std::to_string(i)] = {54, 365 + 20*(i-5), 234, 20};
    }
    for (int i=1; i<=9; i++) {
        areas["F"+std::to_string(i)] = {580, 15 + 50*(i-1), 60, 50};
    }
    for (int i=1; i<=10; i++) {
        areas["G"+std::to_string(i)] = {334 + ((i-1)%5)*49, 315 + ((i-1)/5) * 50, (i==5 || i==10) ? 50 : 49, 50};
    }
    areas["G11"] = {334, 415, 63, 50};
    areas["G12"] = {397, 415, 120, 50};
    areas["G13"] = {517, 415, 63, 50};
} 
class ntc_window : window
{
    int nid_stm;
    std::map<int, Button*> buttons;
    std::map<int, Component*> indicators;
    std::map<int, image_graphic*> icons;
    std::map<int, Message> messages;
    texture *get_icon(int num)
    {
        if (icons.find(num) == icons.end()) {
            icons[num] = Component::getImage("symbols/STM/"+std::to_string(nid_stm)+"/"+std::to_string(num));
        }
        return icons[num];
    }
    Color get_color(int col, bool bg)
    {
        switch (col) {
            case 0:
                return bg ? DarkBlue : Black;
            case 1:
                return White;
            case 2:
                return Red;
            case 3:
                return Blue;
            case 4:
                return Green;
            case 5:
                return Yellow;
            case 5:
                return LightRed;
            case 5:
                return LightGreen;
        }
    }
    public:
    ntc_window() : window(construct_main)
    {

    }
    void display_button(int id, int icon, std::string text, int properties)
    {
        bool displayed = (properties>>9)&1;
        if (!displayed) {
            auto it = buttons.find(id);
            if (it != buttons.end()) {
                delete it->second;
                buttons.erase(it);
            }
            return;
        }
        Button *b = new TextButton(text);
        if (icon > 0)
            b->add(get_icon(icon));
        bool counterflash = (properties>>8)&1;
        int flash = (properties>>6)&3;
        b->setBackgroundColor(get_color((properties>>3)&7, true));
        b->setForegroundColor(get_color(properties&7, false));
        buttons[id] = b;
    }
    void display_indicator(int id, int position, int icon, std::string text, int properties)
    {
        bool displayed = (properties>>9)&1;
        if (!displayed) {
            auto it = indicators.find(id);
            if (it != indicators.end()) {
                delete it->second;
                indicators.erase(it);
            }
            return;
        }
        std::string area;
        if (position < 4)
            area = "B"+std::to_string(position+2);
        else if (position == 4)
            area = "H1";
        else if (position < 10)
            area = "C"+std::to_string(position-3);
        else if (position < 20)
            area = "G"+std::to_string(position-9);
        /*{
            if (position < 3)
                area = "F"+std::to_string(position+7);
            else if (position < 8)
                area = "C"+std::to_string(position-1);
            else if (position < 18)
                area = "G"+std::to_string(position-7);
        }*/
        if (areas.find(area) == areas.end())
            return;
        auto pos = areas[area];
        Component *c = new Component(pos[2],pos[3]);
        Color bg = get_color((properties>>3)&7, true);
        Color fg = get_color(properties&7, false);
        c->setBackgroundColor(bg);
        c->setForegroundColor(fg);
        if (icon > 0)
            c->add(get_icon(icon));
        if (text.size() > 0)
            c->addText(text, 0, 0, 12, fg);
        bool counterflash = (properties>>8)&1;
        int flash = (properties>>6)&3;
        indicators[id] = c;
        addToLayout(c, new RelativeAlignment(nullptr, pos[0], pos[1]));
    }
    ~ntc_window()
    {
        for (auto it : buttons) {
            delete it.second;
        }
        for (auto it : indicators) {
            delete it.second;
        }
        for (auto it : icons) {
            delete it.second;
        }
        for (auto it : icons) {
            delete it.second;
        }
        for (auto it : messages) {
            revokeMessage(it.second.Id);
        }
    }
};