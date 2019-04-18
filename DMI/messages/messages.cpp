#include "messages.h"
#include "../graphics/color.h"
#include "../graphics/component.h"
#include "../graphics/flash.h"
#include "../sound/sound.h"
#include "../graphics/button.h"
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
Button upArrow(46, 50, nullptr, arrowUp);
Button downArrow(46, 50, displayArrows, arrowDown);
const char *message_text[] = {
    "Datos de eurobaliza no consistentes",
    "Trackside malfunction",
    "Communication error",
    "Entrada en modo FS",
    "Entrada en modo OS",
    "Runaway movement",
    "SH refused",
    "SH request failed",
    "Trackside not compatible",
    "Train data changed",
    "Train is rejected",
    "EoA o LoA rebasado",
    "No MA received at level transition",
    "SR distance exceeded",
    "SH stop order",
    "SR stop order",
    "Emergency stop",
    "RV distance exceeded",
    "No track description",
    "%s brake demand",
    "Route unsuitable – loading gauge",
    "Route unsuitable – traction system",
    "Route unsuitable – axle load category",
    "Radio network registration failed",
    "%s not available",
    "%s needs data",
    "%s failed",
    "Reconocer modo SR"
};
deque <Message> messageList;
int line;
int current=0;
void addMsg(Message m)
{
    current = 0;
    messageList.push_back(m);
}
void revokeMessage(int id)
{
    current = 0;
    for(int i=0; i<messageList.size(); i++)
    {
        if(messageList[i].Id == id) messageList.erase(messageList.begin() + i);
    }
}
bool operator < (Message a, Message b)
{
    if(a.firstGroup && !b.firstGroup) return true;
    if(!a.firstGroup && b.firstGroup) return false;
    if(a.hour != b.hour) return a.hour>b.hour;
    return a.minute > b.minute;
}
void displayMessages()
{
    if(messageList.empty())
    {
        addMsg(Message(4,BaliseReadError, 11, 9));
        addMsg(Message(5,SHrefused, 11, 8));
        //addMsg(Message(2,UnauthorizedPassingEOA, 11, 11, true, true, true));
        //addMsg({3,EnteringFS, 11, 9, false, false, true, false});
    } 
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
        textArea.setAck([msg]() {msg->ack = false;});
    }
    else textArea.setAck(nullptr);
    line = 0;
    for(int i=0; i<displayMsg.size(); i++)
    {
        Message &m = *displayMsg[i];
        string date = to_string(m.hour) + ":"+ (m.minute<10 ? "0" : "") + to_string(m.minute);
        string text = m.tripReason ? "Modo TRIP - " : "";
        text += message_text[m.message];
        int last = text.size();
        if(text.size()>25) last = text.find_last_of(' ', 25) + 1;
        if(line<5+current && line>=current)
        {
            if(!m.shown && (m.firstGroup || m.ack)) playSinfo();
            m.shown = true;
            textArea.drawText(date.c_str(), 2, 4 + (line-current)*20, 0, 0, 10, White, UP | LEFT, m.firstGroup);
            textArea.drawText(text.substr(0, last).c_str(), 48, 2 + (line-current)*20, 0, 0, 12, White, UP | LEFT, m.firstGroup);
        }
        line++;
        if(last<text.size())
        {
            if(line<5+current && line>=current) textArea.drawText(text.substr(last).c_str(), 48, 2 + (line-current)*20, 0, 0, 12, White, UP | LEFT, m.firstGroup);
            line++;
        }
    }
}
void displayArrows()
{
    string path = "symbols/Navigation/NA_";
    upArrow.setBackgroundImage((path + (current>0 ? "13" : "15") + ".bmp").c_str());
    downArrow.setBackgroundImage((path + (line>5+current ? "14" : "16") + ".bmp").c_str());
}
void arrowUp()
{
    if(current==0) return;
    current--;
}
void arrowDown()
{
    if(line<=5+current) return;
    current++;
    if(current>line-5) current = line-5;
}
