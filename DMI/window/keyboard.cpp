#include "keyboard.h"
#include "../graphics/text_button.h"
vector<Button*> getNumericKeyboard(function<void(string)> setData, function<string()> getData)
{
    vector<Button*> keys;
    for(int i=0; i<9; i++)
    {
        keys.push_back(new TextButton(to_string(i+1), 102, 50, nullptr, 16));
    }
    keys.push_back(new TextButton("DEL", 102, 50, nullptr));
    keys.push_back(new TextButton("0", 102, 50, nullptr, 16));
    keys.push_back(new TextButton(".", 102, 50));
    for(int i=0; i<12; i++)
    {
        keys[i]->setPressedAction([i, getData, setData]
        {
            string s = getData();
            if(i<11 && s=="0") s = "";
            if(i<9) s = s + to_string(i+1);
            if(i==9) s = s.substr(0, s.size()-1);
            if(i==10) s = s + "0";
            setData(s);
        });
    }
    return keys;
}
vector<Button*> getAlphaNumericKeyboard(function<void(string)> setData, function<string()> getData)
{
    vector<Button*> keys;
    for(int i=0; i<9; i++)
    {
        keys.push_back(new Button(102, 50, nullptr, nullptr));
    }
    keys[0]->addText("1",0,0,16);
    keys[1]->addText("2",-15,0,16);
    keys[1]->addText("abc",4,0,10);
    keys[2]->addText("3",-15,0,16);
    keys[2]->addText("def",4,0,10);
    keys[3]->addText("4",-15,0,16);
    keys[3]->addText("ghi",4,0,10);
    keys[4]->addText("5",-15,0,16);
    keys[4]->addText("jkl",4,0,10);
    keys[5]->addText("6",-15,0,16);
    keys[5]->addText("mno",10,0,10);
    keys[6]->addText("7",-15,0,16);
    keys[6]->addText("pqrs",10,0,10);
    keys[7]->addText("8",-15,0,16);
    keys[7]->addText("tuv",4,0,10);
    keys[8]->addText("9",-15,0,16);
    keys[8]->addText("wxyz",10,0,10);
    keys.push_back(new TextButton("DEL", 102, 50, nullptr));
    keys.push_back(new TextButton("0", 102, 50, nullptr, 16));
    keys.push_back(new TextButton(".", 102, 50, nullptr));
    for(int i=0; i<12; i++)
    {
        keys[i]->setPressedAction([setData, getData, i]
        {
            string data = getData();
            if(i<11 && data=="0") data = "";
            if(i<9) data = data + to_string(i+1);
            if(i==9) data = data.substr(0, data.size()-1);
            if(i==10 && data!="") data = data + "0";
            setData(data);
        });
    }
    return keys;
}
vector<Button*> getSingleChoiceKeyboard(vector<string> posibilities, function<void(string)> setData)
{
    vector<Button*> keys;
    for(int i=0; i<posibilities.size(); i++)
    {
        keys.push_back(new TextButton(posibilities[i], 102, 50));
        keys[i]->setPressedAction([setData, i, posibilities]
        {
            setData(posibilities[i]);
        });
    }
    return keys;
}