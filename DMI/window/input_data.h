#ifndef _INPUT_DATA_H
#define _INPUT_DATA_H
#include <string>
#include <vector>
#include "../graphics/button.h"
using namespace std;
class input_data
{
    protected:
    bool selected = false;
    bool accepted = false;
    public:
    string label;
    string data;
    Component* label_comp = nullptr;
    Component* data_comp = nullptr;
    void setSelected(bool val);
    void setAccepted(bool val);
    vector<Button*> keys;
    virtual void validate(){setAccepted(true);}
    input_data(string label_text="");
    virtual ~input_data();
};
#endif