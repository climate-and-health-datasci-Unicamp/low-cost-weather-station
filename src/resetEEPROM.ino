#include <EEPROM.h>

void setup() {
    // put your setup code here, to run once:
    EEPROM.begin(4);
    EEPROM.write(0,0);
    EEPROM.commit();
}

void loop() {
    // put your main code here, to run repeatedly:

}
