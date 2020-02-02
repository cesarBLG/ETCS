#ifndef _INPUT_DATA_H
#define _INPUT_DATA_H
#include <string>
#include <vector>
#include <ctime>
#include "../graphics/button.h"
#include "../graphics/text_button.h"
using namespace std;
class input_data
{
    protected:
    bool selected = false;
    bool accepted = false;
    bool valid = false;
    public:
    string label;
    string data;
    string data_accepted;
    string prev_data;
    string keybd_data;
    text_graphic *data_tex;
    Component* label_comp = nullptr;
    Component* data_comp = nullptr;
    Component* label_echo = nullptr;
    Component* data_echo = nullptr;
    time_t holdcursor;
    void setSelected(bool val);
    void setAccepted(bool val);
    bool isValid() {return valid;}
    TextButton more;
    vector<Button*> keys;
    int keypage=0;
    virtual void validate(){if (data!="") valid = true;}
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