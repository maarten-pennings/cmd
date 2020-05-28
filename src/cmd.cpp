// cmd.cpp - Command interpreter


#include <Arduino.h>
#include <avr/pgmspace.h> // This library assumes most strings (command help texts) are in PROGMEM (flash, not RAM)
#include "cmd.h"


// A struct to store a command descriptor
typedef struct cmd_desc_s { 
  cmd_func_t   main; 
  const char * name; 
  const char * shorthelp; 
  const char * longhelp; 
} cmd_desc_t;


// All command descriptors
static int cmd_descs_count= 0;
static cmd_desc_t cmd_descs[CMD_REGISTRATION_SLOTS];


// The registration function for command descriptors (all strings in PROGMEM!)
int cmd_register(cmd_func_t main, const char * name, const char * shorthelp, const char * longhelp) {
  if( cmd_descs_count >= CMD_REGISTRATION_SLOTS ) return -1;
  cmd_descs[cmd_descs_count].main= main;
  cmd_descs[cmd_descs_count].name= name;
  cmd_descs[cmd_descs_count].shorthelp= shorthelp;
  cmd_descs[cmd_descs_count].longhelp= longhelp;
  cmd_descs_count++;
  return CMD_REGISTRATION_SLOTS - cmd_descs_count;
}


// Finds the command descriptor for a command with name `name`.
// When not found, returns 0.
static cmd_desc_t * cmd_find(char * name ) {
  for( int i=0; i<cmd_descs_count; i++ ) {
    if( cmd_isprefix(cmd_descs[i].name,name) ) return &cmd_descs[i];
  }
  return 0;
}


// The state machine for receiving characters via Serial
static char       cmd_buf[CMD_BUFSIZE];              // Incoming chars
static int        cmd_ix;                            // Fill pointer into cmd_buf
static bool       cmd_echo;                          // Command interpreter should echo incoming chars
static cmd_func_t cmd_streamfunc;                    // If 0, no streaming, else the streaming handler
static char       cmd_streamprompt[CMD_PROMPT_SIZE]; // If streaming (cmd_stream_main!=0), the streaming prompt


// Print the prompt when waiting for input (special variant when in streaming mode)
static void cmd_prompt() {
  if( cmd_streamfunc ) {
    Serial.print( cmd_streamprompt );
  } else {
    Serial.print(F(">> "));  
  }
}


// Initializes the command interpreter
void cmd_begin() {
  cmd_ix= 0;
  cmd_echo= true;
  cmd_streamfunc= 0;
  cmd_streamprompt[0]= 0;
  cmd_prompt();
}


// Execute the entered command (terminated with a press on RETURN key)
void cmd_exec() {
  char * argv[ CMD_MAXARGS ];
  // Cut a trailing comment
  char * cmt= strchr(cmd_buf,'/');
  if( cmt!=0 && *(cmt+1)=='/' ) { *cmt='\0'; cmd_ix= cmt-cmd_buf; } // trim comment
  // Find the arguments (set up argv/argc)
  int argc= 0;
  int ix=0;
  while( ix<cmd_ix ) {
    // scan for begin of word (ie non-space)
    while( (ix<cmd_ix) && ( cmd_buf[ix]==' ' || cmd_buf[ix]=='\t' ) ) ix++;
    if( !(ix<cmd_ix) ) break;
    argv[argc]= &cmd_buf[ix];
    argc++;
    if( argc>CMD_MAXARGS ) { Serial.println(F("ERROR: too many arguments"));  return; }
    // scan for end of word (ie space)
    while( (ix<cmd_ix) && ( cmd_buf[ix]!=' ' && cmd_buf[ix]!='\t' ) ) ix++;
    cmd_buf[ix]= '\0';
    ix++;
  }
  //for(ix=0; ix<argc; ix++) { Serial.print(ix); Serial.print("='"); Serial.print(argv[ix]); Serial.print("'"); Serial.println(""); }
  // Check from streaming
  if( cmd_streamfunc ) {
    cmd_streamfunc(argc, argv); // Streaming mode is active pass the data
    return;
  }
  // Bail out when empty
  if( argc==0 ) {
    // Empty command entered
    return; 
  }
  // Find the command
  char * s= argv[0];
  if( *s=='@' ) s++;
  cmd_desc_t * d= cmd_find(s);
  // If a command is found, execute it 
  if( d!=0 ) {
    d->main(argc, argv ); // Execute handler of command
    return;
  } 
  Serial.println(F("ERROR: command not found (try help)")); 
}


// Add characters to the state machine of the command interpreter (firing a command on <CR>)
void cmd_add(int ch) {
  if( ch=='\n' || ch=='\r' ) {
    if( cmd_echo ) Serial.println();
    cmd_buf[cmd_ix]= '\0'; // Terminate (make cmd_buf a c-string)
    cmd_exec();
    cmd_ix=0;
    if( cmd_echo ) cmd_prompt();
  } else if( ch=='\b' ) {
    if( cmd_ix>0 ) {
      if( cmd_echo ) Serial.print(F("\b \b"));
      cmd_ix--;
    } else {
      // backspace with no more chars in buf; ignore
    }
  } else {
    if( cmd_ix<CMD_BUFSIZE-1 ) {
      cmd_buf[cmd_ix++]= ch;
      if( cmd_echo ) Serial.print((char)ch);
    } else {
      if( cmd_echo ) Serial.print(F("_\b"));
      // input buffer full
    }
  }
}


// Helpers =========================================================================


void cmd_set_streamfunc(cmd_func_t func) {
  cmd_streamfunc= func;
}


cmd_func_t cmd_get_streamfunc(void) {
  return cmd_streamfunc;
}


void cmd_set_streamprompt(const char * prompt) {
  strncpy(cmd_streamprompt, prompt, CMD_PROMPT_SIZE);
  cmd_streamprompt[CMD_PROMPT_SIZE-1]= '\0';
}


const char * cmd_get_streamprompt(void) {
  return cmd_streamprompt;
}


// Parse a string to a hex number, returns false if there were errors. 
// If true is returned, *v is the parsed value.
bool cmd_parse(char*s,uint16_t*v) {
  if( v==0 ) return false;
  *v= 0;
  if( s==0 ) return false;
  if( *s==0 ) return false;
  if( strlen(s)>4 ) return false;
  while( *s!=0 ) {
    if     ( '0'<=*s && *s<='9' ) *v = (*v)*16 + *s - '0';
    else if( 'a'<=*s && *s<='f' ) *v = (*v)*16 + *s - 'a' + 10;
    else if( 'A'<=*s && *s<='F' ) *v = (*v)*16 + *s - 'A' + 10;
    else return false;
    s++;
  }
  return true;
}


// Returns true iff `prefix` is a prefix of `str`. Note `str` must be in PROGMEM (and `prefix` in RAM)
bool cmd_isprefix(const char *str, const char *prefix) {
  while( *prefix!='\0') {
    byte b= pgm_read_byte(str);
    if( b!=*prefix ) return false;
    str++;
    prefix++;
  }
  return true;
}


static int cmd_errorcount= 0;

void cmd_steperrorcount( void ) {
  cmd_errorcount++;
}


int cmd_geterrorcount( void ) {
  int current= cmd_errorcount;
  cmd_errorcount= 0;
  return current;
}


void cmd_pollserial( void ) {
  // Check incoming serial chars
  int n= 0; // Counts number of bytes read, this is roughly the number of bytes in the UART buffer
  while( 1 ) {
    int ch= Serial.read();
    if( ch==-1 ) break;
    if( ++n==SERIAL_RX_BUFFER_SIZE ) { // Possible UART buffer overflow
      cmd_steperrorcount();
      Serial.println(); Serial.println( F("WARNING: Serial overflow") ); Serial.println(); 
    }
    // Process read char by feeding it to command interpreter
    cmd_add(ch);
  }
}



// Friend command: echo ================================================================


// The handler for the "echo" command
static void cmdecho_print() { Serial.print(F("echo: ")); Serial.println(cmd_echo?F("enabled"):F("disabled")); }
static void cmdecho_main(int argc, char * argv[]) {
  if( argc==1 ) {
    cmdecho_print();
    return;
  }
  if( argc==2 && cmd_isprefix(PSTR("errors"),argv[1]) ) {
    int n= cmd_geterrorcount();
    if( argv[0][0]!='@') { Serial.print(F("echo: errors: ")); Serial.println(n); }
    return;
  }
  if( argc==2 && cmd_isprefix(PSTR("enable"),argv[1]) ) {
    cmd_echo= true;
    if( argv[0][0]!='@') cmdecho_print();
    return;
  }
  if( argc==2 && cmd_isprefix(PSTR("disable"),argv[1]) ) {
    cmd_echo= false;
    if( argv[0][0]!='@') cmdecho_print();
    return;
  }
  int start= 1;
  if( cmd_isprefix(PSTR("line"),argv[1]) ) { start=2; }
  // This tries to restore the command line (put spaces back on the '\0's)
  //char * s0=argv[start-1]+strlen(argv[start-1])+1;
  //char * s1=argv[argc-1];
  //for( char * p=s0; p<s1; p++ ) if( *p=='\0' ) *p=' ';
  //Serial.println(s0); 
  for( int i=start; i<argc; i++) { if(i>start) Serial.print(' '); Serial.print(argv[i]);  }
  Serial.println();
}


static const char cmdecho_longhelp[] PROGMEM = 
  "SYNTAX: echo [line] <word>...\n"
  "- prints all words (useful in scripts)\n"
  "SYNTAX: [@]echo errors\n"
  "- shows and resets the number of communication errors detected"
  "- with @ present, no feedback is printed\n"
  "SYNTAX: [@]echo [ enable | disable ]\n"
  "- with arguments enables/disables terminal echoing\n"
  "- (disabled is useful in scripts; output is relevant, but input much less)\n"
  "- with @ present, no feedback is printed\n"
  "- without arguments shows status of terminal echoing\n"
  "NOTES:\n"
  "- 'echo line' prints a white line (there are no <word>s)\n"
  "- 'echo line enable' prints 'enable'\n"
  "- 'echo line disable' prints 'disable'\n"
  "- 'echo line line' prints 'line'\n"
;


void cmdecho_register(void) {
  cmd_register(cmdecho_main, PSTR("echo"), PSTR("echo a message (or en/disables echoing)"), cmdecho_longhelp);
}


// Friend command: help =================================================================


// The handler for the "help" command
static void cmdhelp_main(int argc, char * argv[]) {
  if( argc==1 ) {
    Serial.println(F("Available commands"));
    for( int i=0; i<cmd_descs_count; i++ ) {
      Serial.print(f(cmd_descs[i].name));
      Serial.print(F(" - "));
      Serial.println(f(cmd_descs[i].shorthelp));
    }
  } else if( argc==2 ) {
    cmd_desc_t * d= cmd_find(argv[1]);
    if( d==0 ) {
      Serial.println(F("ERROR: help: command not found (try 'help')"));    
    } else {
      // Copy chunks of longhelp in PROGMEM via RAM to Serial
      const char * str= d->longhelp;
      int len= strlen_P(str);
      while( len>0 ) {
          #define SIZE 32
          char ram[SIZE+1];
          int size= len<SIZE ? len : SIZE;
          memcpy_P(ram, str, size);
          ram[size]='\0';
          Serial.print(ram);
          str+= size;
          len-= size;
      }
      // longhelp is in PROGMEM so we need to get the chars one by one...
      //for(unsigned i=0; i<strlen_P(d->longhelp); i++) 
      //  Serial.print((char)pgm_read_byte_near(d->longhelp+i));
    }
  } else {
    Serial.println(F("ERROR: help: too many arguments"));
  }
}


static const char cmdhelp_longhelp[] PROGMEM = 
  "SYNTAX: help\n"
  "- lists all commands\n"
  "SYNTAX: help <cmd>\n"
  "- gives detailed help on command <cmd>\n"
  "NOTES:\n"
  "- all commands may be shortened, for example 'help', 'hel', 'he', 'h'\n"
  "- all sub commands may be shortened, for example 'help help' to 'help h'\n"
  "- normal prompt is >>, other prompt indicates streaming mode\n"
  "- commands may be suffixed with a comment starting with //\n"
  "- some commands support a @ as prefix; it suppresses output of that command\n"
;


void cmdhelp_register(void) {
  cmd_register(cmdhelp_main, PSTR("help"), PSTR("gives help (try 'help help')"), cmdhelp_longhelp);
}
