#ifndef KEYPAD_MODULE
#define KEYPAD_MODULE

#include <Arduino.h>
#include <Keypad.h>

#define ROW_NUM     4
#define COLUMN_NUM  4
#define TRIES 5

class KeypadModule {
  private:

    char keys[ROW_NUM][COLUMN_NUM] = {
      {'1', '2', '3', 'A'},
      {'4', '5', '6', 'B'},
      {'7', '8', '9', 'C'},
      {'*', '0', '#', 'D'}
    };

    byte pin_rows[ROW_NUM]      = {32, 33, 25, 26};
    byte pin_column[COLUMN_NUM] = {27, 14, 12, 13};

    Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
    String pin;
    int maxPinLength;
  public:
    KeypadModule(const char* pin, int maxPinLength);
    bool enterPin();
};


#endif