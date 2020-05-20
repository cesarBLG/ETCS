#include "messages.h"
#include "../graphics/color.h"
#include "../graphics/component.h"
#include "../graphics/flash.h"
#include "../sound/sound.h"
#include "../graphics/icon_button.h"
#include "../tcp/server.h"
#include "text_strings.h"
#include <string>
#include <deque>
#include <vector>
#include <algorithm>
using namespace std;
Component e1(54, 25, nullptr);
Component e2(54, 25, nullptr);
Component e3(54, 25, nullptr);
Component e4(54, 25, nullptr);
Component textArea(234, 100, displayMessages);
void displayArrows();
void arrowUp();
void arrowDown();
IconButton upArrow("symbols/Navigation/NA_13.bmp", 46, 50, arrowUp, "symbols/Navigation/NA_15.bmp");
IconButton downArrow("symbols/Navigation/NA_14.bmp", 46, 50, arrowDown, "symbols/Navigation/NA_16.bmp");
deque <Message> messageList;
int line;
int current=0;
void disp();
void addMsg(Message m)
{
    bool found = false;
    for (auto it = messageList.begin(); it != messageList.end(); ++it) {
        if (it->Id == m.Id) {
            *it = m;
            found = true;
        }
    }
    current = 0;
    if (!found)
        messageList.push_back(m);
    disp();
}
void revokeMessage(int id)
{
    current = 0;
    for(int i=0; i<messageList.size(); i++)
    {
        if(messageList[i].Id == id) messageList.erase(messageList.begin() + i);
    }
    disp();
}
bool operator < (Message a, Message b)
{
    if(a.firstGroup && !b.firstGroup) return true;
    if(!a.firstGroup && b.firstGroup) return false;
    if(a.hour != b.hour) return a.hour>b.hour;
    if (a.minute != b.minute) return a.minute>b.minute;
    return a.Id > b.Id;
}
void displayMessages()
{
    /*if(messageList.empty())
    {
        addMsg(Message(4,BaliseReadError, 11, 9));
        addMsg(Message(5,SHrefused, 11, 8));
        addMsg(Message(2,UnauthorizedPassingEOA, 11, 11, true, true, true));
        addMsg(Message(3,EnteringFS, 11, 9, true));
    }*/
}
void disp()
{
    sort(messageList.begin(), messageList.end());
    vector<Message*> displayMsg;
    bool ack = false;
    for(int i=0; i<messageList.size(); i++)
    {
        if(messageList[i].ack)
        {
            displayMsg.push_back(&messageList[i]);
            ack = true;
            break;
        }
    }
    if(!ack)
    {
        for(int i=0; i<messageList.size(); i++)
        {
            displayMsg.push_back(&messageList[i]);
        }
    }
    if(ack)
    {
        Message *msg = displayMsg[0];
        textArea.setAck([msg]() {
            msg->ack = false;
            write_command("messageAcked", to_string(msg->Id));
            disp();
        });
    }
    else textArea.setAck(nullptr);
    textArea.clear();
    line = 0;
    for(int i=0; i<displayMsg.size(); i++)
    {
        Message &m = *displayMsg[i];
        string date = to_string(m.hour) + ":"+ (m.minute<10 ? "0" : "") + to_string(m.minute);
        string text = "";
        if (m.reason == 1) 
            text += "Train TRIP - ";
        else if (m.reason == 2) 
            text += "Train brake - ";
        text += m.text;
        int last = text.size();
        if(text.size()>25) last = text.find_last_of(' ', 25) + 1;
        if(line<5+current && line>=current)
        {
            if(!m.shown && (m.firstGroup || m.ack)) playSinfo();
            m.shown = true;
            textArea.addText(date, 2, 4 + (line-current)*20, 10, White, UP | LEFT, m.firstGroup);
            textArea.addText(text.substr(0, last), 48, 2 + (line-current)*20, 12, White, UP | LEFT, m.firstGroup);
        }
        line++;
        if(last<text.size())
        {
            if(line<5+current && line>=current) textArea.addText(text.substr(last), 48, 2 + (line-current)*20, 12, White, UP | LEFT, m.firstGroup);
            line++;
        }
    }
    upArrow.enabled = current>0;
    downArrow.enabled = line>(5+current);
}
void arrowUp()
{
    if(current==0) return;
    current--;
    upArrow.enabled = current>0;
    downArrow.enabled = line<=5+current;
    disp();
}
void arrowDown()
{
    if(line<=5+current) return;
    current++;
    if(current>line-5) current = line-5;
    upArrow.enabled = current>0;
    downArrow.enabled = line<=5+current;
    disp();
}
