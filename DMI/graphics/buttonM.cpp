#include "buttonM.h"
#include "button.h"
#include "window.h"
#include "algorithm"
extern vector<window*> active_windows;
extern window navigation_main;
extern window navigation_bar;
Button modeButton(60, 50, ModePintar, ModePulsado);
void ModePintar(){
    modeButton.setText("Main",12,White);
}
void ModePulsado(){
    active_windows.push_back(&navigation_main);
    remove(active_windows.begin(), active_windows.end(), &navigation_bar);
}
Button Override(60, 50, overpint, overPuls);
void overpint(){
    Override.setText("Override",12,White);
}
void overPuls(){
    
}
Button DataView(60, 50, Datapint, DataPuls);
void Datapint(){
    DataView.setText("Dataview",12,White);
}
void DataPuls(){

}
Button Spec(60, 50, Specpint, SpecPuls);
void Specpint(){
    Spec.setText("Spec",12,White);
}
void SpecPuls(){

}
Button Conf(60, 50, Confprint, ConfPuls);
void Confprint(){
    Conf.setBackgroundImage("symbols/Setting/SE_04.bmp");
}
void ConfPuls(){

}