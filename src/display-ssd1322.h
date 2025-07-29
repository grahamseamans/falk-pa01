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
    void irProgMode(const char* message);
    void irProgComplete();
};

#endif