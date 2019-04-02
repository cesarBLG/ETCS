#ifndef _PASP_H
#define _PASP_H
#include <vector>
using namespace std;
struct pasp_object
{
    int condition;
    float distance;
};
extern vector<pasp_object> pasp_objects;
void displayPASP();
float getPASPy(float dist);
void drawObjects(int num, int distance);
#endif