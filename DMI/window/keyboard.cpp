#include "keyboard.h"
#include "../graphics/text_button.h"
vector<Button*> getNumericKeyboard(function<void(string)> setData, function<string()> getData)
{
    vector<Button*> keys;
    for(int i=0; i<9; i++)
    {
        keys.push_back(new Button(102, 50, nullptr, nullptr));
    }
    keys[0]->setDisplayFunction([keys]
    {
        keys[0]->drawText("1",0,0,0,0,16);
    });
    keys[1]->setDisplayFunction([keys]
    {
        keys[1]->drawText("2",0,0,0,0,16);
    });
    keys[2]->setDisplayFunction([keys]
    {
        keys[2]->drawText("3",0,0,0,0,16);
    });
    keys[3]->setDisplayFunction([keys]
    {
        keys[3]->drawText("4",0,0,0,0,16);
    });
    keys[4]->setDisplayFunction([keys]
    {
        keys[4]->drawText("5",0,0,0,0,16);
    });
    keys[5]->setDisplayFunction([keys]
    {
        keys[5]->drawText("6",0,0,0,0,16);
    });
    keys[6]->setDisplayFunction([keys]
    {
        keys[6]->drawText("7",0,0,0,0,16);
    });
    keys[7]->setDisplayFunction([keys]
    {
        keys[7]->drawText("8",0,0,0,0,16);
    });
    keys[8]->setDisplayFunction([keys]
    {
        keys[8]->drawText("9",0,0,0,0,16);
    });
    keys.push_back(new TextButton("DEL", 102, 50, nullptr));
    keys.push_back(new Button(102, 50, nullptr, nullptr));
    keys[10]->setDisplayFunction([keys]
    {
        keys[10]->drawText("0",0,0,0,0,16);
    });
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
    keys[0]->setDisplayFunction([keys]
    {
        keys[0]->drawText("1",0,0,0,0,16);
    });
    keys[1]->setDisplayFunction([keys]
    {
        keys[1]->drawText("2",-15,0,0,0,16);
        keys[1]->drawText("abc",4,0,0,0,10);
    });
    keys[2]->setDisplayFunction([keys]
    {
        keys[2]->drawText("3",-15,0,0,0,16);
        keys[2]->drawText("def",4,0,0,0,10);
    });
    keys[3]->setDisplayFunction([keys]
    {
        keys[3]->drawText("4",-15,0,0,0,16);
        keys[3]->drawText("ghi",4,0,0,0,10);
    });
    keys[4]->setDisplayFunction([keys]
    {
        keys[4]->drawText("5",-15,0,0,0,16);
        keys[4]->drawText("jkl",4,0,0,0,10);
    });
    keys[5]->setDisplayFunction([keys]
    {
        keys[5]->drawText("6",-15,0,0,0,16);
        keys[5]->drawText("mno",10,0,0,0,10);
    });
    keys[6]->setDisplayFunction([keys]
    {
        keys[6]->drawText("7",-15,0,0,0,16);
        keys[6]->drawText("pqrs",10,0,0,0,10);
    });
    keys[7]->setDisplayFunction([keys]
    {
        keys[7]->drawText("8",-15,0,0,0,16);
        keys[7]->drawText("tuv",4,0,0,0,10);
    });
    keys[8]->setDisplayFunction([keys]
    {
        keys[8]->drawText("9",-15,0,0,0,16);
        keys[8]->drawText("wxyz",10,0,0,0,10);
    });
    keys.push_back(new TextButton("DEL", 102, 50, nullptr));
    keys.push_back(new Button(102, 50, nullptr, nullptr));
    keys[10]->setDisplayFunction([keys]
    {
        keys[10]->drawText("0",0,0,0,0,16);
    });
    keys.push_back(new TextButton(".", 102, 50, nullptr));
    for(int i=0; i<12; i++)
    {
        keys[i]->setPressedAction([setData, getData, i]
        {
            string data = getData();
            if(i<11 && data=="0") data = "";
            if(i<9) data = data + to_string(i+1);
            if(i==9) data = data.substr(0, data.size()-1);
            if(i==10) data = data + "0";
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
        keys.push_back(new TextButton(posibilities[i].c_str(), 102, 50));
        keys[i]->setPressedAction([setData, i]
        {
            string data;
            if(i<3) data = "Level " + to_string(i+1);
            if(i==3) data = "Level 0";
            setData(data);
        });
    }
    return keys;
    keys.push_back(new TextButton("Level 1", 102, 50));
    keys.push_back(new TextButton("Level 2", 102, 50));
    keys.push_back(new TextButton("Level 3", 102, 50));
    keys.push_back(new TextButton("Level 0", 102, 50));
    keys.push_back(new TextButton("LZB", 102, 50));
    keys.push_back(new TextButton("EBICAB", 102, 50));
    for(int i=0; i<4; i++)
    {
        
    }
}