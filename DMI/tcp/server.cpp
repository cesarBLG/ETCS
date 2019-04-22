#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>
#include <string>
#include <sstream>
#include <cstdio>
#include <thread>
#include <chrono>
#include "server.h"
#include "../monitor.h"
#include "../sound/sound.h"
#include "../messages/messages.h"
#include "../time.h"
#include "../planning/planning.h"
#include "../graphics/drawing.h"
using namespace std;
IPaddress ip;
TCPsocket serv_sock;
TCPsocket socket;
#define BUFF_SIZE 512
#define PORT 5000
char data[BUFF_SIZE];
stringstream buffer;
SDLNet_SocketSet set;
static SDL_Event ev;
void notifyDataReceived()
{
    repaint();
}
void parseData(string str)
{
    int index = str.find_first_of('(');
    string command = str.substr(0, index);
    string value = str.substr(index+1, str.find_first_of(')')-index-1);
    if(command == "setVperm") Vperm = stof(value);
    if(command == "setVsbi") Vsbi = stof(value);
    if(command == "setVtarget") Vtarget = stof(value);
    if(command == "setVest") Vest = stof(value);
    if(command == "setVrelease") Vrelease = stof(value);
    if(command == "setDtarget") Dtarg = stof(value);
    if(command == "setTTI") TTI = stof(value);
    if(command == "setLevel") level = value== "NTC" ? NTC : (Level)stoi(value);
    if(command == "setEB") EB = value!="false";
    if(command == "setMode")
    {
        Mode m;
        if(value == "FS")
        {
            m = FS;
            addMsg(Message(1,EnteringFS, getHour(), getMinute(), true));
        }
        if(value == "LS") m = LS;
        if(value == "OS") m = OS;
        if(value == "SH") m = SH;
        if(value == "SB") m = SB;
        if(value == "SR") m = SR;
        if(value == "UN") m = UN;
        mode = m;
    }
    if(command == "setMonitor") setMonitor(value == "CSM" ? CSM : (value == "TSM" ? TSM : RSM));
    if(command == "setSupervision")
    {
        SupervisionStatus s;
        if(value == "NoS") s = NoS;
        if(value == "IndS") s = IndS;
        if(value == "OvS") s = OvS;
        if(value == "WaS") s = WaS;
        if(value == "IntS") s = IntS;
        setSupervision(s);
    }
    if(command == "setPlanningObjects")
    {
        vector<string> val;

        int valsep = value.find(',');
        while(valsep!=string::npos)
        {
            string param = value.substr(0,valsep);
            value = value.substr(valsep+1);
            valsep = value.find(',');
            val.push_back(param);
        }
        val.push_back(value);

        planning_elements.clear();
        for(int i = 0; i < val.size(); i+=2)
        {
            planning_element p;
            p.condition = stoi(val[i]);
            p.distance = stoi(val[i+1]);
            planning_elements.push_back(p);
        }
    }
    if(command == "setPlanningGradients")
    {
        vector<string> val;

        int valsep = value.find(',');
        while(valsep!=string::npos)
        {
            string param = value.substr(0,valsep);
            value = value.substr(valsep+1);
            valsep = value.find(',');
            val.push_back(param);
        }
        val.push_back(value);

        gradient_elements.clear();
        for(int i = 0; i < val.size(); i+=3)
        {
            gradient_element g;
            g.val = stoi(val[i]);
            g.distance = stoi(val[i+1]);
            g.length = stoi(val[i+2]);
            gradient_elements.push_back(g);
        }
    }
    if(command == "setPlanningSpeeds")
    {
        vector<string> val;

        int valsep = value.find(',');
        while(valsep!=string::npos)
        {
            string param = value.substr(0,valsep);
            value = value.substr(valsep+1);
            valsep = value.find(',');
            val.push_back(param);
        }
        val.push_back(value);

        speed_elements.clear();
        for(int i = 0; i < val.size(); i+=2)
        {
            speed_element s;
            s.speed = stoi(val[i]);
            s.distance = stoi(val[i+1]);
            speed_elements.push_back(s);
        }
    }
    notifyDataReceived();
}
void read()
{
    if(SDLNet_CheckSockets(set, 100)<0)
    {
        printf("SDL CheckSockets Error: %s\n", SDL_GetError());
    }
    if(!SDLNet_SocketReady(socket)) return;
    int result = SDLNet_TCP_Recv(socket, data, BUFF_SIZE);
    if(result>0)
    {
        for(int i=0; i<result && i<BUFF_SIZE; i++)
        {
            buffer<<data[i];
        }
        int index = buffer.str().substr(buffer.tellg()).find_first_of(';');
        while(index>0 && index<512)
        {
            stringstream command;
            bool s = false;
            for(int i=0; i<=index; i++)
            {
                char c = buffer.get();
                if(c == 's') s = true;
                if(s) command.put(c);
            }
            parseData(command.str());
            index = buffer.str().substr(buffer.tellg()).find_first_of(';');
        }
        //buffer.str(buffer.str().substr(buffer.tellg()));
    }
}
extern bool running;
void startSocket()
{
    if(SDLNet_Init()<0)
    {
        printf("%SDL Net Init Error: %s\n", SDL_GetError());
    }
    set = SDLNet_AllocSocketSet(2);
    SDLNet_ResolveHost(&ip, nullptr, PORT);
    serv_sock = SDLNet_TCP_Open(&ip);
    SDLNet_TCP_AddSocket(set, serv_sock);
    socket = nullptr;
    while(socket == nullptr && running)
    {
        SDLNet_CheckSockets(set, 100);
        if(SDLNet_SocketReady(serv_sock))
        {
            socket = SDLNet_TCP_Accept(serv_sock);
            SDLNet_TCP_AddSocket(set, socket);
        }
    }
    while(running)
    {
        read();
    }
    closeSocket();
}
void closeSocket()
{
    SDLNet_TCP_Close(socket);
    SDLNet_TCP_Close(serv_sock);
    SDLNet_Quit();
}