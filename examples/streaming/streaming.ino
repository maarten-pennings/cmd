// streaming.ino - An example for cmd with a command that streams
#include "cmd.h"


// The stat command =======================================================================


// The statistics counters
int cmdstat_count=0;
int cmdstat_sum=0;

void cmdstat_streamfunc( int argc, char * argv[] ) {
  for( int i=0; i<argc; i++ ) {
    uint16_t val;
    if( strcmp(argv[i],"*")==0 ) {
      // toggle streaming mode
      if( cmd_get_streamfunc()==0 ) cmd_set_streamfunc(cmdstat_streamfunc); else cmd_set_streamfunc(0);
    } else {
      bool ok= cmd_parse_hex(argv[i],&val) ;
      if( !ok ) {
        Serial.print(F("Error: value must be hex '"));
        Serial.print(argv[i]);
        Serial.println("'");
        return;
      }
      cmdstat_sum+= val;
      cmdstat_count+= 1;
    }
  }
  // Set the streaming prompt (will only be shown in streaming mode)
  char buf[6]; snprintf(buf,sizeof buf, "%03d: ",cmdstat_count); cmd_set_streamprompt(buf);
}


// The statistics command handler
void cmdstat_main(int argc, char * argv[]) {
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
  // The rest of the function would do core processing: reading values and add them 
  // to the statistics counters. But that same code would be in the streaming handler.
  // So here we can simply call that one, except that we need to shift the argument.
  cmdstat_streamfunc(argc-1, argv+1);
}


// Note cmd_register needs all strigs to be PROGMEM strings. For longhelp we do that manually
const char cmdstat_longhelp[] PROGMEM = 
  "SYNTAX: stat reset\n"
  "- resets the statistic counters\n"
  "SYNTAX: stat show\n"
  "- shows the statistic counters\n"
  "SYNTAX: stat (*|<hexnum>)...\n"
  "- updates the statistic counters (sum and count)\n"
  "- a * toggle streaming mode\n"
;


// Note cmd_register needs all strings to be PROGMEM strings. For the short string we do that inline with PSTR.
void cmdstat_register(void) {
  cmd_register(cmdstat_main, PSTR("stat"), PSTR("compute count, sum and average of hex numbers"), cmdstat_longhelp);
}


// The main program =======================================================================


void setup() {
  Serial.begin(115200);
  Serial.println( F("Welcome to the demo cmd.streaming") );
  
  cmd_init();
  cmdecho_register();  // Use the built-in echo command
  cmdhelp_register();  // Use the built-in help command
  cmdstat_register();  // Register our own stat command

  Serial.println( );
  Serial.println( F("Type 'help' for help") );
  Serial.println( F("Try 'stat 1 2 3' and 'stat show'") );
  Serial.println( F("or try streaming 'stat *', '1 2 3', '4 5 6 7', '*', and 'stat show'") );
  Serial.println( );
  cmd_prompt();
}


void loop() {
  cmd_pollserial();
}
