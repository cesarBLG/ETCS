#include "input_data.h"
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
    data_comp->addText(data,10,0,12, selected ? Black : (accepted ? White : Grey), LEFT);
    data_comp->setDisplayFunction([this]
    {
        data_comp->setBorder(MediumGrey);
    });
    if(label!="")
    {
        label_comp->setBackgroundColor(DarkGrey);
        label_comp->addText(label.c_str(), 10, 0, 12, Grey, RIGHT);
        label_echo->addText(label, 5, 0, 12, White, RIGHT);
        data_echo->addText(data, 4, 0, 12, White, LEFT);
    }
}
void input_data::setData(string s)
{
    if(s==data) return;
    data = s;
    setAccepted(false);
    if(label!="")
    {
        data_echo->clear();
        data_echo->addText(data, 4, 0, 12, White, LEFT);
    }
}
void input_data::setSelected(bool val)
{
    selected = val;
    data_comp->setBackgroundColor(selected ? MediumGrey : DarkGrey);
    data_comp->clear();
    data_comp->addText(data,10,0,12, selected ? Black : (accepted ? White : Grey), LEFT);
}
void input_data::setAccepted(bool val)
{
    accepted = val;
    data_comp->setBackgroundColor(selected ? MediumGrey : DarkGrey);
    data_comp->clear();
    data_comp->addText(data,10,0,12, selected ? Black : (accepted ? White : Grey), LEFT);
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