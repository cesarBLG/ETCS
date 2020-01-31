#ifndef _PACKET_H
#define _PACKET_H
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include "parsed_packet.h"
class packet : public variable
{
    static std::map<int,packet*> packets;
    std::vector<variable*> var;
    bool entered = false;
    bool goInto(int val) override
    {
        if(entered) return false;
        entered = true;
        return true;
    }
public:
    static parsed_packet parse(BitReader &b)
    {
        int num = b.peekByte();
        packet *p = packets[num];
        return parsed_packet(p->name, num, p->read(b));  
    }
    packet(int id)
    {
        packets[id] = this;
        std::ifstream file;
        file.open("Packets/"+std::to_string(id)+".txt");
        std::string str;
        std::getline(file,str);
        name = str;
        std::vector<variable*> depth;
        depth.push_back(this);
        while(!file.eof())
        {
            getline(file,str);
            if(str.size()<1) continue;
            int d = str.find_first_not_of(' ');
            while(d<depth.size()-1) depth.pop_back();
            if(d>=depth.size()) depth.push_back(depth.back()->nested.back());
            variable* var;
            std::stringstream stream(str);
            std::string name;
            int size;
            stream>>name;
            stream>>size;
            int target;
            stream>>target;
            if(name == "N_ITER") var = new N_ITER();
            else if(!stream.fail()) var = new condition(name, size, target);
            else var = new variable(name, size);
            depth.back()->nested.push_back(var);
        }
    }
};
#endif _PACKET_H
