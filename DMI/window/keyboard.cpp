#include "keyboard.h"
#include "../graphics/text_button.h"
vector<Button*> getNumericKeyboard(function<void(string)> setData, function<string()> getData)
{
    vector<Button*> keys;
    keys.push_back(new TextButton("1", 102, 50));
    keys.push_back(new TextButton("2", 102, 50));
    keys.push_back(new TextButton("3", 102, 50));
    keys.push_back(new TextButton("4", 102, 50));
    keys.push_back(new TextButton("5", 102, 50));
    keys.push_back(new TextButton("6", 102, 50));
    keys.push_back(new TextButton("7", 102, 50));
    keys.push_back(new TextButton("8", 102, 50));
    keys.push_back(new TextButton("9", 102, 50));
    keys.push_back(new TextButton("DEL", 102, 50));
    keys.push_back(new TextButton("0", 102, 50));
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