// cmd.h - header for command interpreter
// From https://github.com/maarten-pennings/cmd
#ifndef __CMD_H__
#define __CMD_H__


#include <stdint.h> // uint16_t


// Version of this library
#define CMD_VERSION "8.2.1" 
// Changed 8.2.0 -> 8.2.1:
//   fix: printf -> println, added #include <stdint.h>
// Changed 8.1.0 -> 8.2.0:
//   cmd_parse_dec(), cmd_parse_hex() now use const
//   cmd_register() has switch for alphabetical order
// Changed 8.0.1 -> 8.1.0:
//   cmd_begin() renamed to cmd_init()
//   cmd_init() serial prints "init"
//   cmd_init() no longer had prompt()
//   cmd_prompt() ow public
//   max number of commands from 16 to 20
//   no longer prints incoming chars as hex on error
//   echo now has wait option
// Fixed 8.0.0 -> 8.0.1:
//   prompt again shown when echo is disabled
// Incompatible 7.0.0 -> 8.0.0:
//   prompt no longer shown when echo is disabled
//   echo errors -> echo faults
//   cmd_parse -> cmd_parse_hex


// For AVR (not relevant for ESP8266/ESP32)
// Recall that F(xxx) puts literal xxx in PROGMEM _and_ makes it printable.
// It uses PSTR() to map it to PROGMEM, and f() to make it printable. Basically F(x) is f(PSTR(x))
// For some reason, Arduino defines PSTR() and F(), but not f().
// The macro f(xxx) assumes xxx is already a pointer to chars in PROGMEM' it only makes it printable.
#ifndef f
#define f(s) ((__FlashStringHelper *)(s)) // An empty class, so that the overloaded print() uses print-from-progmem
#endif


// The maximum number of characters the interpreter can buffer.
// The buffer is cleared when executing a command. Execution happens when a <CR> or <LF> is passed.
#define CMD_BUFSIZE 128
// When a command starts executing, it is split in arguments.
#define CMD_MAXARGS 32
// Total number of registration slots.
#define CMD_REGISTRATION_SLOTS 20
// Size of buffer for the streaming prompt
#define CMD_PROMPT_SIZE 10 
// Size of buffer for cmd_prt
#define CMD_PRT_SIZE 80 


// A command must implement a 'main' function. It is much like C's main, it has argc and argv.
typedef void (*cmd_func_t)( int argc, char * argv[] );
// To register a command pass a pointer to its 'main', the string 'name' that invokes it, and a short and long help text.
// All strings must be stored in PROGMEM (either using 'PSTR("help")' or 'const char s[] PROGMEM = "help";'.
// Registering returns -1 when registration failed. Can even be called before cmd_init()
int cmd_register(cmd_func_t main, const char * name, const char * shorthelp, const char * longhelp);  
// There are two standard commands, closely integrated with the command handler
int cmdecho_register(void);
int cmdhelp_register(void);


// Initializes the command interpreter.
void cmd_init();
// Print the prompt when waiting for input (special variant when in streaming mode). Needed once after init().
void cmd_prompt();
// Add characters to the state machine of the command interpreter (firing a command on <CR>)
void cmd_add(int ch); // Suggested to use cmd_pollserial(), which reads chars from Serial and calls cmd_add()
void cmd_addstr(const char * str); // Convenient for automatic testing of command line processing
void cmd_addstr_P(/*PROGMEM*/const char * str); // Same as above, but str in PROGMEN
int  cmd_pendingschars(); // Returns the number of (not yet executed) chars.


// The command handler can support streaming: sending data without commands. 
// To enable streaming, a command must install a streaming function f with cmd_set_streamfunc(f).
// Streaming is disabled via cmd_set_streamfunc(0).
void cmd_set_streamfunc(cmd_func_t func);
// Check which streaming function is installed (0 for none).
cmd_func_t cmd_get_streamfunc(void);
// Default prompt is >>, but when streaming is enabled a different prompt will be printed.
// Note 'prompt' will be copied to internal cmd_buf[CMD_BUFSIZE].
// Calling cmd_set_streamfunc(f) enables streaming; only when enabled the streamprompt is used.
void cmd_set_streamprompt(const char * prompt);
// Get the streaming prompt.
const char * cmd_get_streamprompt(void);


// Helper functions


// Parse a string of a decimal number ("-12"). Returns false if there were errors. If true is returned, *v is the parsed value.
bool cmd_parse_dec(const char*s,int*v);
// Parse a string of a hex number ("0A8F"). Returns false if there were errors. If true is returned, *v is the parsed value.
bool cmd_parse_hex(const char*s,uint16_t*v) ;
// Returns true iff `prefix` is a prefix of `str`. Note `str` must be in PROGMEM (`prefix` in RAM)
bool cmd_isprefix(/*PROGMEM*/const char *str, const char *prefix);
// Reads Serial and calls cmd_add()
void cmd_pollserial( void );
// A print towards Serial, just like Serial.print, but now with formatting as printf()
int cmd_printf(const char *format, ...);
// A print towards Serial, just like Serial.print, but now with formatting as printf(), now from progmem
int cmd_printf_P(/*PROGMEM*/const char *format, ...);
// When cmd_pollserial() detects Serial buffer overflows it steps an error counter
void cmd_steperrorcount( void );
// The current error counter can be obtained with this function; as a side effect it clears the counter.
int  cmd_geterrorcount( void );


#endif
