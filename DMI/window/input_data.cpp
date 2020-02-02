#include "input_data.h"
#include "../graphics/flash.h"
input_data::input_data(string label_text) : label(label_text), data_get([this] {return getData();}), 
data_set([this](string s){setData(s);}), more("More", 102, 50)
{
    if(label!="")
    {
        label_comp = new Component(204,50);
        data_comp = new Component(204,50);
        label_echo = new Component(100,16);
        data_echo = new Component(100,16);
    }
    else data_comp = new Component(204+102,50);
    data_tex = data_comp->getText(data,10,0,12, selected ? Black : (accepted ? White : Grey), LEFT);
    data_comp->add(data_tex);
    data_comp->setDisplayFunction([this]
    {
        data_comp->setBorder(MediumGrey);
        if (selected && flash_state%2)
        {
            float cur = data_tex->width+data_tex->offx;
            time_t now;
            time(&now);
            if (keybd_data.empty()) cur = data_tex->offx;
            else if (difftime(now, holdcursor)<2) cur-=9;
            data_comp->drawLine(cur, 32, cur+9, 32, Black);
        }
    });
    if(label!="")
    {
        label_comp->setBackgroundColor(DarkGrey);
        label_comp->addText(label.c_str(), 10, 0, 12, Grey, RIGHT);
        label_echo->addText(label, 5, 0, 12, White, RIGHT);
        data_echo->addText(prev_data, 4, 0, 12, White, LEFT);
    }
}
void input_data::setData(string s)
{
    data = s;
    keybd_data = s;
    setAccepted(false);
}
void input_data::setSelected(bool val)
{
    if (selected != val) 
    {
        selected = val;
        if (selected)
        {
            data = prev_data;
            keybd_data = "";
        }
        else
        {
            if(!prev_data.empty() && !keybd_data.empty()) data = "";
            prev_data = data;
        }
    }
    data_comp->setBackgroundColor(selected ? MediumGrey : DarkGrey);
    data_comp->clear();
    data_tex = data_comp->getText(data,10,0,12, selected ? Black : (accepted ? White : Grey), LEFT);
    data_comp->add(data_tex);
}
void input_data::setAccepted(bool val)
{
    if (accepted != val)
    {
        accepted = val;
        if (accepted) data_accepted = data;
        else data_accepted = "";
    }
    data_comp->setBackgroundColor(selected ? MediumGrey : DarkGrey);
    data_comp->clear();
    data_tex = data_comp->getText(data,10,0,12, selected ? Black : (accepted ? White : Grey), LEFT);
    data_comp->add(data_tex);
    if(label!="")
    {
        data_echo->clear();
        data_echo->addText(prev_data, 4, 0, 12, White, LEFT);
    }
}
input_data::~input_data()
{
    for(int i=0; i<keys.size(); i++)
    {
        if (keys[i]!=nullptr) delete keys[i];
    }
    if(label_comp!=nullptr) delete label_comp;
    if(data_comp!=nullptr) delete data_comp;
    if(label_echo!=nullptr) delete label_echo;
    if(data_echo!=nullptr) delete data_echo;
}