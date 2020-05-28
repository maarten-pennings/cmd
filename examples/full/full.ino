// full.ino - An example for cmd; adding a simple statistics command


#include "cmd.h"


// The stat command =======================================================================


// The statistics counters
int cmdstat_count=0;
int cmdstat_sum=0;

// The statistics command handler
void cmdstat_main(int argc, char * argv[]) {
  // Note cmd_isprefix needs a PROGMEM string. PSTR stores the string in PROGMEM.
  if( argc==2 && cmd_isprefix(PSTR("reset"),argv[1]) ) { 
    cmdstat_count= 0;
    cmdstat_sum= 0;
    Serial.println(F("stat: reset"));
    return;
  }
  if( argc==1 || (argc==2 && cmd_isprefix(PSTR("show"),argv[1])) ) {
    Serial.print(F("stat: ")); 
    Serial.print(cmdstat_sum); Serial.print(F("/")); Serial.print(cmdstat_count); 
    if( cmdstat_count>0 ) { Serial.print(F("=")); Serial.print((float)cmdstat_sum/cmdstat_count); }
    Serial.println();
    return;
  }
  for( int i=1; i<argc; i++ ) {
    uint16_t val;
    bool ok= cmd_parse(argv[i],&val) ;
    if( !ok ) {
      Serial.print(F("Error: sum: value must be hex '"));
      Serial.print(argv[i]);
      Serial.println("'");
      return;
    }
    cmdstat_sum+= val;
    cmdstat_count+= 1;
  }
}

// Note cmd_register needs all strigs to be PROGMEM strings. For longhelp we do that manually
const char cmdstat_longhelp[] PROGMEM = 
  "SYNTAX: stat reset\n"
  "- resets the statistic counters\n"
  "SYNTAX: stat show\n"
  "- shows the statistic counters\n"
  "SYNTAX: stat <hexnum>...\n"
  "- updates the statistic counters (sum and count)\n"
;

// Note cmd_register needs all strings to be PROGMEM strings. For the short string we do that inline with PSTR.
void cmdstat_register(void) {
  cmd_register(cmdstat_main, PSTR("stat"), PSTR("compute count, sum and average of hex numbers"), cmdstat_longhelp);
}


// The main program =======================================================================


void setup() {
  Serial.begin(115200);
  Serial.println( F("Welcome to the demo cmd.full") );
  Serial.println( );
  Serial.println( F("Type 'help' for help") );
  Serial.println( F("Try 'stat 1 2 3' and 'stat show'") );
  cmd_begin();
  cmdecho_register();  // Use the built-in echo command
  cmdhelp_register();  // Use the built-in help command
  cmdstat_register();  // Register our own stat command
}

void loop() {
  cmd_pollserial();
}