#include "input_data.h"
input_data::input_data(string label_text) : label(label_text), data_get([this] {return getData();}), 
data_set([this](string s){setData(s);})
{
    if(label!="")
    {
        label_comp = new Component(204,50);
        data_comp = new Component(204,50);
        label_echo = new Component(100,16);
        data_echo = new Component(100,16);
    }
    else data_comp = new Component(204+102,50);
    data_comp->setDisplayFunction([this]
    {
        data_comp->drawText(data.c_str(),10,0,0,0,12, selected ? Black : (accepted ? White : Grey), LEFT);
        data_comp->setBorder(MediumGrey);
    });
    if(label!="")
    {
        label_comp->setBackgroundColor(DarkGrey);
        label_comp->setDisplayFunction([this]{label_comp->drawText(label.c_str(), 10, 0, 0, 0, 12, Grey, RIGHT);});
        label_echo->setDisplayFunction([this]
        {
            label_echo->drawText(label.c_str(), 5, 0, 0, 0, 12, White, RIGHT);
        });
        data_echo->setDisplayFunction([this]
        {
            data_echo->drawText(data.c_str(), 4, 0, 0, 0, 12, White, LEFT);
        });
    }
}
void input_data::setSelected(bool val)
{
    selected = val;
    data_comp->setBackgroundColor(selected ? MediumGrey : DarkGrey);
}
void input_data::setAccepted(bool val)
{
    accepted = val;
    data_comp->setBackgroundColor(selected ? MediumGrey : DarkGrey);
}
input_data::~input_data()
{
    for(int i=0; i<keys.size(); i++)
    {
        delete keys[i];
    }
    if(label_comp!=nullptr) delete label_comp;
    if(data_comp!=nullptr) delete data_comp;
    if(label_echo!=nullptr) delete label_echo;
    if(data_echo!=nullptr) delete data_echo;
}