// basic.ino - An example for cmd; just the built-in commands

#include "cmd.h"

void setup() {
  Serial.begin(115200);
  Serial.println( F("Welcome to the demo cmd.basic") );
  Serial.println( );
  Serial.println( F("Type 'help' for help") );
  cmd_begin();
  cmdecho_register();  // Use the built-in echo command
  cmdhelp_register();  // Use the built-in help command
}

void loop() {
  cmd_pollserial();
}
