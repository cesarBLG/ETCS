#include "keyboard.h"
#include "../graphics/text_button.h"
vector<Button*> getNumericKeyboard(input_data *input)
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
        keys[i]->setPressedAction([i, input]
        {
            string s = input->keybd_data;
            //if(i<11 && s=="0") s = "";
            if(i<9) s = s + to_string(i+1);
            if(i==9) s = s.substr(0, s.size()-1);
            if(i==10) s = s + "0";
            input->setData(s);
        });
    }
    return keys;
}
vector<Button*> getAlphaNumericKeyboard(input_data *input)
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
        keys[i]->setPressedAction([input, i]
        {
            string data = input->keybd_data;
            //if(i<11 && data=="0") data = "";
            time_t now;
            time(&now);
            bool replaced = false;
            if (difftime(now, input->holdcursor)<2)
            {
                char lc = data[data.size()-1];
                if (i==1)
                {
                    if (lc == '2') lc = 'a';
                    else if (lc == 'a') lc = 'b';
                    else if (lc == 'b') lc = 'c';
                    else if (lc == 'c') lc = '2';
                }
                else if (i==2)
                {
                    if (lc == '3') lc = 'd';
                    else if (lc == 'd') lc = 'e';
                    else if (lc == 'e') lc = 'f';
                    else if (lc == 'f') lc = '3';
                }
                else if (i==3)
                {
                    if (lc == '4') lc = 'g';
                    else if (lc == 'g') lc = 'h';
                    else if (lc == 'h') lc = 'i';
                    else if (lc == 'i') lc = '4';
                }
                else if (i==4)
                {
                    if (lc == '5') lc = 'j';
                    else if (lc == 'j') lc = 'k';
                    else if (lc == 'k') lc = 'l';
                    else if (lc == 'l') lc = '5';
                }
                else if (i==5)
                {
                    if (lc == '6') lc = 'm';
                    else if (lc == 'm') lc = 'n';
                    else if (lc == 'n') lc = 'o';
                    else if (lc == 'o') lc = '6';
                }
                else if (i==6)
                {
                    if (lc == '7') lc = 'p';
                    else if (lc == 'p') lc = 'q';
                    else if (lc == 'q') lc = 'r';
                    else if (lc == 'r') lc = 's';
                    else if (lc == 's') lc = '7';
                }
                else if (i==7)
                {
                    if (lc == '8') lc = 't';
                    else if (lc == 't') lc = 'u';
                    else if (lc == 'u') lc = 'v';
                    else if (lc == 'v') lc = '8';
                }
                else if (i==8)
                {
                    if (lc == '9') lc = 'w';
                    else if (lc == 'w') lc = 'x';
                    else if (lc == 'x') lc = 'y';
                    else if (lc == 'y') lc = 'z';
                    else if (lc == 'z') lc = '9';
                }
                if (lc!=data[data.size()-1])
                {
                    replaced = true;
                    data[data.size()-1] = lc;
                }
            }
            if (!replaced)
            {
                if(i<9) data = data + to_string(i+1);
                if(i==9) data = data.substr(0, data.size()-1);
                if(i==10) data = data + "0";
            }
            if (i!=0 && i!=9 && i!=10 && i!= 11) input->holdcursor = now;
            else input->holdcursor = {0};
            input->setData(data);
        });
    }
    return keys;
}
vector<Button*> getSingleChoiceKeyboard(vector<string> posibilities, input_data *input)
{
    vector<Button*> keys;
    for(int i=0; i<posibilities.size(); i++)
    {
        keys.push_back(new TextButton(posibilities[i], 102, 50));
        keys[i]->setPressedAction([input, i, posibilities]
        {
            input->setData(posibilities[i]);
        });
    }
    return keys;
}
vector<Button*> getYesNoKeyboard(input_data *input)
{
    vector<Button*> keys;
    for (int i=0; i<12; i++) {
        keys.push_back(nullptr);
    }
    keys[6] = new TextButton("No", 102, 50);
    keys[7] = new TextButton("Yes", 102, 50);
    keys[6]->setPressedAction([input]
    {
            input->setData("No");
    });
    keys[7]->setPressedAction([input]
    {
            input->setData("Yes");
    });
    return keys;
}