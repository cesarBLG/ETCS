#ifndef _PACKET_H
#define _PACKET_H
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include "parsed_packet.h"
class packet : public variable
{
    static map<int,packet*> packets;
    vector<variable*> var;
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
        packet *p = packets[b.peekByte()];
        return parsed_packet(p->name, p->read(b));  
    }
    packet(int id)
    {
        packets[id] = this;
        ifstream file;
        file.open("Packets/"+to_string(id)+".txt");
        string str;
        getline(file,str);
        name = str;
        vector<variable*> depth;
        depth.push_back(this);
        while(!file.eof())
        {
            getline(file,str);
            if(str.size()<1) continue;
            int d = str.find_first_not_of(' ');
            while(d<depth.size()-1) depth.pop_back();
            if(d>=depth.size()) depth.push_back(depth.back()->nested.back());
            variable* var;
            stringstream stream(str);
            string name;
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
