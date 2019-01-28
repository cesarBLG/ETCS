#include "MainMenu.h"
#include "button.h"
#include "window.h"
#include "algorithm"
extern vector<window*> active_windows;
extern window navigation_main;
extern window navigation_bar;

Button start(153, 50, starprint, starpulsado);
void starprint(){
    start.setText("Start", 12, White);
}
void starpulsado(){
}
Button DriverId(153, 50, driverprint, driverpulsado);
void driverprint(){
    DriverId.setText("Driver ID", 12, White);
}
void driverpulsado(){

}
Button Level(153, 50, Levelprint, Levelpulsado);
void Levelprint(){
    Level.setText("Level", 12, White);
}
void Levelpulsado(){

}
Button Shunting(153, 50, Shuntingprint, Shuntingpulsado);
void Shuntingprint(){
    Shunting.setText("Shunting", 12, White);
}
void Shuntingpulsado(){

}
Button MaintShunt(153, 50, MainShuntprint, MainShuntpulsado);
void MainShuntprint(){
    MaintShunt.setText("Maintain Shunting", 12, DarkGrey);
}
void MainShuntpulsado(){

}
Button TrainData(153, 50, trainprint, trainpulsado);
void trainprint(){
    TrainData.setText("Train data", 12, White);
}
void trainpulsado(){

}
Button TrainRun(153, 50, trainrunprint, trairunpulsado);
void trainrunprint(){
    TrainRun.setText("Train running number", 12, White);
}
void trairunpulsado(){

}
Button Non(153, 50, Nonprint, Nonpulsado);
void Nonprint(){
    Non.setText("Non-Leading", 12, White);
}
void Nonpulsado(){

}
Button Exit(82, 50, exitprint, exitpulsado);
void exitprint(){
    Exit.setBackgroundImage("symbols/Navigation/NA_11.bmp");
}
void exitpulsado(){
    active_windows.push_back(&navigation_bar);
    remove(active_windows.begin(), active_windows.end(), &navigation_main);
}