// cmd.h - command interpreter
#ifndef __CMD_H__
#define __CMD_H__


// Version of this library
#define CMD_VERSION 2


// Recall that F(xxx) puts literal xxx in PROGMEM _and_ makes it printable.
// It uses PSTR() to map it to progmem, and f() to make it printable. Basically F(x) is f(PSTR(x))
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
#define CMD_REGISTRATION_SLOTS 16
// Size of buffer for the streaming prompt
#define CMD_PROMPT_SIZE 10 


// A command must implement a 'main' function. It is much like C's main, it has argc and argv.
typedef void (*cmd_func_t)( int argc, char * argv[] );
// To register a command pass a pointer to its 'main', the string 'name' that invokes it, and a short and long help text.
// All strings must be stored in PROGMEM (either using 'PSTR("help")' or 'const char s[] PROGMEM = "help";'.
// Registering returns -1 when registration failed. Can even be called before cmd_init()
int cmd_register(cmd_func_t main, const char * name, const char * shorthelp, const char * longhelp);  
// There are two standard commands, closely integrated with the command handler
void cmd_register_echo(void);
void cmd_register_help(void);


// Initializes the command interpreter (and prints the prompt).
void cmd_begin(void);
// Add characters to the state machine of the command interpreter (firing a command on <CR>)
void cmd_add(int ch); // Suggested to use cmd_pollserial(), which reads chars from Serial and calls cmd_add()


// Helper functions

// The command handler can support streaming (sending data without commands). 
// To enable streaming, a command must install a streaming function (cmd_set_streamfunc).
// Streaming is disabled via cmd_set_streamfunc(0).
void cmd_set_streamfunc(cmd_func_t func);
cmd_func_t cmd_get_streamfunc(void);
// When streaming is enabled an different prompt is printed.
void cmd_set_streamprompt(const char * prompt);
const char * cmd_get_streamprompt(void);


// Parse a string to a hex number. Returns false if there were errors. If true is returned, *v is the parsed value.
bool cmd_parse(char*s,uint16_t*v) ;
// Returns true iff `prefix` is a prefix of `str`. Note `str` must be in PROGMEM (`prefix` in RAM)
bool cmd_isprefix(const char *str, const char *prefix);
// Reads Serial and calls cmd_add()
void cmd_pollserial( void );
// When cmd_pollserial() detects Serial buffer overflows it steps an error counter
void cmd_steperrorcount( void );
// The current error counter can be obtained with this function; as a side effect it clears the counter.
int cmd_geterrorcount( void );


#endif
