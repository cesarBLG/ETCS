#ifndef _PLANNING_H
#define _PLANNING_H
#include <vector>
using namespace std;
struct planning_element
{
    int condition;
    float distance;
};
struct gradient_element
{
    int val;
    float distance;
    float length;
};
struct speed_element
{
    int speed;
    float distance;
};
extern vector<planning_element> planning_elements;
extern vector<gradient_element> gradient_elements;
extern vector<speed_element> speed_elements;
void displayPlanning();
void displayObjects();
void displayGradient();
void displaySpeed();
void displayPASP();
float getPlanningHeight(float dist);
void drawObjects(int num, int distance);
#endif