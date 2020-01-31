#ifndef _INPUT_DATA_H
#define _INPUT_DATA_H
#include <string>
#include <vector>
#include "../graphics/button.h"
#include "../graphics/text_button.h"
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
    Component* label_echo = nullptr;
    Component* data_echo = nullptr;
    void setSelected(bool val);
    void setAccepted(bool val);
    bool isValid() {return accepted;}
    TextButton more;
    vector<Button*> keys;
    int keypage=0;
    virtual void validate(){if (data!="") setAccepted(true);}
    input_data(string label_text="");
    function<string()> data_get;
    function<void(string)> data_set;
    void setData(string s);
    string getData()
    {
        return data;
    }
    virtual ~input_data();
};
#endif