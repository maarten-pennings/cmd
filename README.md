# cmd
Command line interpreter Arduino library

## Basic

The [basic](examples/basic/basic.ino) demonstrates the command interpreter with just the built-in commands: `help` and `echo`.

This is a sample run

```text
Welcome to the demo cmd.basic

Type 'help' for help
>> what can I type
ERROR: command not found (try help)
>> help
Available commands
echo - echo a message (or en/disables echoing)
help - gives help (try 'help help')
>> help echo
SYNTAX: echo [line] <word>...
- prints all words (useful in scripts)
SYNTAX: [@]echo [ enable | disable ]
- without arguments shows status of terminal echoing
- with arguments enables/disables terminal echoing
- with @ present, no feedback is printed
- useful in scripts; output is relevant, but input much less
NOTES:
- 'echo line' prints a white line (there are no <word>s)
- 'echo line enable' prints 'enable'
- 'echo line disable' prints 'disable'
- 'echo line line' prints 'line'
>> echo Hello, World!
Hello, World!
>> 
```

The command interpreter does allow commands to be abbreviated

```text
>> h h
SYNTAX: help
- lists all commands
SYNTAX: help <cmd>
- gives detailed help on command <cmd>
NOTES:
- all commands may be shortened, for example 'help', 'hel', 'he', 'h'
- all sub commands may be shortened, for example 'help help' to 'help h'
- normal prompt is >>, other prompt indicates streaming mode
- commands may be suffixed with a comment starting with #
- some commands support a @ as prefix; it suppresses output of that command
```

## Full

The [full](examples/full/full.ino) demonstrates the command interpreter with one command added `stat`.
The `stat` command accepts a list of (hexadecimal) numbers and counts and adds them. 
`stat reset` sets the counters to 0, `stat show` shows the counters, and `stat 1 2 3` adds the numbers to the counters.

This is a sample run

```text
Welcome to the demo cmd.full

Type 'help' for help
Try 'stat 1 2 3' and 'stat show'
>> stat show
stat: 0/0
>> stat 1 A  20
>> stat show
stat: 43/3=14.33
>> stat 9
>> stat show
stat: 52/4=13.00
>> stat reset
stat: reset
>> s s
stat: 0/0
>> ```


## Streaming

The [streaming](examples/streaming/streaming.ino) demonstrates the command interpreter, with the `stat`, which now supports streaming.
Streaming allows data to be send to the arduino, without giving an explicit command each time.

How streaming is enabled, and how it is stopped is up to the command, in this example `stat` uses `*` to toggle streaming mode.


```text
Welcome to the demo cmd.streaming

Type 'help' for help
Try 'stat 1 2 3' and 'stat show'
or try streaming 'stat *', '1 2 3', '4 5 6 7', '*', and 'stat show'

>> stat *
000: 1 2 3
003: 4 5 6 7
007: *
>> stat show
stat: 28/7=4.00
>> stat reset
stat: reset
>> stat 1 2 * 3 4
004: 5 6 * 7
>> stat show
stat: 28/7=4.00
>> 
```

(end of doc)
