#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <functional>
using namespace std;
struct ETCS_variable
{
    int size;
    function<bool> isvalid(uint32_t val); 
};
map<string,ETCS_variable> vars;
void load()
{
    ifstream in("variables.txt");
    string line;
    while (!getline(in, line).fail()) {
        stringstream s(line);
        string var;
        int len;
        s>>var>>len;
        ETCS_variable b;
        b.size = len;
        
        vars[var] = b;
    }
}