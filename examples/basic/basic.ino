// basic.ino - An example for cmd; just the built-in commands


#include "cmd.h"


void setup() {
  Serial.begin(115200);
  Serial.println( F("Welcome to the demo cmd.basic") );
  Serial.println( );
  Serial.println( F("Type 'help' for help") );
  cmd_begin();
  cmd_register_echo(); // Use the built-in help command
  cmd_register_help(); // Use the built-in echo command
}

void loop() {
  cmd_pollserial();
}
