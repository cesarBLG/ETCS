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
};
struct speed_element
{
    int speed;
    float distance;
    bool operator==(speed_element e)
    {
        return (e.speed == speed) && (e.distance = distance);
    }
    bool operator!=(speed_element e)
    {
        return !(e == *this);
    }
};
struct indication_marker
{
    float start_distance;
    speed_element element;
};
extern vector<planning_element> planning_elements;
extern vector<gradient_element> gradient_elements;
extern vector<speed_element> speed_elements;
extern indication_marker imarker;
void displayPlanning();
void displayObjects();
void displayGradient();
void displaySpeed();
void displayPASP();
float getPlanningHeight(float dist);
void drawObjects(int num, int distance);
#endif
