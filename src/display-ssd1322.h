#ifndef FALK_DISP
#define FALK_DISP

class Display {
  public:
    void begin();
    void loop();
    void updateScreen();
    void dimScreen();
    void off();
    void setDisplay(bool);
};

#endif