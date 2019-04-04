#ifndef _PLANNING_H
#define _PLANNING_H
#include <vector>
using namespace std;
struct planning_element
{
    int condition;
    float distance;
};
extern vector<planning_element> planning_elements;
void displayPlanning();
float getPlanningHeight(float dist);
void drawObjects(int num, int distance);
#endif