/* File: shell.c
 * -------------
 *  and implement a simple command-line shell. 
 *  The program sits in a loop, reading a command typed by the user and then executing it.
 * The public functions implemented in shell are: 
 * void shell_readline(char buf[], size_t bufsize)
int shell_evaluate(const char *line)
 */
#include "shell.h"
#include "shell_commands.h"
#include "uart.h"
#include "malloc.h"
#include "strings.h"
#include "mango.h"
#include "printf.h"
#include "ps2_keys.h"
#define LINE_LEN 80

// Module-level global variables for shell
static struct {
    input_fn_t shell_read;
    formatted_fn_t shell_printf;
} module;

char history [10][128];
int n_cmd = 0;
int nhistory = 0;

// NOTE TO STUDENTS:
// Your shell commands output various information and respond to user
// error with helpful messages. The specific wording and format of
// these messages would not generally be of great importance, but
// in order to streamline grading, we ask that you aim to match the
// output of the reference version.
//
// The behavior of the shell commands is documented in "shell_commands.h"
// https://cs107e.github.io/header#shell_commands
// The header file gives example output and error messages for all
// commands of the reference shell. Please match this wording and format.
//
// Your graders thank you in advance for taking this care!

static int cmd_history(int argc, const char* argv[]);
static const command_t commands[] = {
    {"help",  "help [cmd]",  "print command usage and description", cmd_help},
    {"echo",  "echo [args]", "print arguments", cmd_echo},
    {"clear", "clear",       "clear screen (if your terminal supports it)", cmd_clear},
    {"reboot", "reboot",       "reboot the Mango Pi", cmd_reboot},
    {"peek", "peek [addr]",       "print contents of memory at address", cmd_peek},
    {"poke", "poke [addr] [val]",       "store value into memory at address", cmd_poke},
    {"history", "history",       "keeps track of all the past commands", cmd_history}
};

// Implementations of shell echo command
int cmd_echo(int argc, const char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        module.shell_printf("%s ", argv[i]);
    }
    module.shell_printf("\n");
    return 0;
}

// Implementations of shell help command
int cmd_help(int argc, const char *argv[]) {
    if(argc == 1){ // if no command is specified, print usage and description for all commands
        for(int i = 0; i<7; i++){
            module.shell_printf("%-19s%s\n",commands[i].usage, commands[i].description);
        }
        
    }
    else{ // if a command is specified, print usage and description for that command
        command_t command;
        command.name = argv[1];
        int found = 0;
        for(int i = 0; i<7; i++){
            if(strcmp(command.name, commands[i].name) == 0){ // if the command is found, print its usage and description
                found = 1;
                module.shell_printf("%-19s%s\n",commands[i].usage, commands[i].description);
                return 0;
            }

        }
        if(found == 0){ // if the command is not found, print an error message
            module.shell_printf("error: no such command '%s' \n", command.name); 
            return 1;
        }
    }
    return 1; 

    // *   help [cmd]         print command usage and description
    // *   echo [args]        print arguments
    // *   reboot             reboot the Mango Pi
    // *   peek [addr]        print contents of memory at address
    // *   poke [addr] [val]  store value into memory at address
    //     help [cmd]         print command usage and description
    // echo [args]        print arguments
    // clear              clear screen (if your terminal supports it)
    // reboot             reboot the Mango Pi
    // peek [addr]        print contents of memory at address
    // poke [addr] [val]  store value into memory at address
}


// Implementations of shell clear command
int cmd_clear(int argc, const char* argv[]) {
    module.shell_printf("\f");   // formfeed character
    return 0;
}

// Implementations of shell reboot command
int cmd_reboot(int argc, const char* argv[]) {
    module.shell_printf("Rebooting...\n");   // formfeed character
    mango_reboot();
    return 0;
}
// Implementations of shell peek command
int cmd_peek(int argc, const char* argv[]) {
    if(argc == 2){ // if the correct number of arguments is given, try to convert the address argument and print the contents of memory at that address
        const char* addr = argv[1];


        ///
        // for(int i = 0; i< strlen(addr); i ++){
        //     if(!(addr[i]  >= '0' && addr[i] <= '9')){
        //         module.shell_printf("error: peek cannot convert '%s'\n", addr);
        //         return 1;
        //     }
    
        // }
        const char **endptr = (const char *[]){"Alpha", "Beta", "Gamma"};
        unsigned long address = strtonum(addr,endptr);
        if (address==0 || *endptr[0] != '\0'){
            module.shell_printf("error: peek cannot convert '%s'\n", addr);
            return 1;
        }
        /// If the address argument is missing or cannot be converted, 
        long unsigned int* addrptr = (long unsigned int*)address; 
        if(address%4 == 0){ // if the address is 4-byte aligned, print the contents of memory at that address
            module.shell_printf("0x%8x:\t%8x\n", (unsigned int)address, (unsigned int)*addrptr);
            return 0;
        }
        else{ // if the address is not 4-byte aligned, print an error message
            module.shell_printf("error: peek address must be 4-byte aligned\n");
            return 1;
        }
        
    }
    else{ // if the incorrect number of arguments is given, print an error message
        module.shell_printf("error: peek expects 1 argument [addr]\n");
        return 1; 
    }
    return 0;
}

int cmd_history(int argc, const char* argv[]) {
    int log = 0;
    if (argc == 1) {
        log = nhistory;
    } else if (argc == 2) {
        const char **endptr = (const char *[]){"Alpha", "Beta", "Gamma"};
        log = strtonum(argv[1], endptr);
        if (log > nhistory) log = nhistory;
    } else {
        module.shell_printf("error: history expects 1 argument [num]\n");
        return 1;
    }

    for (int i = 0; i < log; i++) {
        int idx = (n_cmd - log + i + 10) % 10;
        module.shell_printf("%d  %s\n", i + 1, history[idx]);
    }
    return 0;
}



int cmd_poke(int argc, const char* argv[]) {
    // module.shell_printf("%d", argc);
    if(argc == 3){ // if the correct number of arguments is given, try to convert the address and value arguments and store the value into memory at that address
        
        const char* addr = argv[1];
        const char* val = argv[2];

        ///
        // for(int i = 0; i< strlen(addr); i ++){
        //     if(!(addr[i]  >= '0' && addr[i] <= '9')){
        //         module.shell_printf("error: poke cannot convert '%s'\n", addr);
        //         return 1;
        //     }
    
        // }
        const char **endptr = (const char *[]){"Alpha", "Beta", "Gamma"};
        unsigned long address = strtonum(addr,endptr);
        if (address ==0  || *endptr[0] != '\0'){
            module.shell_printf("error: poke cannot convert '%s'\n", addr);
            return 1;
        }
        unsigned long value = 0;
        if(strcmp(val, "0") != 0){
            value = strtonum(val,endptr);
            if (value==0 || *endptr[0] != '\0'){
                module.shell_printf("error: poke cannot convert '%s'\n", val);
                return 1;
            }
        }
        else{
            value = 0;
        }
        
         /// If the address argument is missing or cannot be converted, 
 
        if(address%4 == 0){ // if the address is 4-byte aligned, try to convert the value argument and store the value into memory at that address
            int* addrptr = (int*) address; 
            *addrptr = value;
            // module.shell_printf("0x%8x:\t%8x\n", (unsigned int)address, (unsigned int)*addrptr);
            return 0;
        }
        else{ // if the address is not 4-byte aligned, print an error message
            module.shell_printf("error: poke address must be 4-byte aligned\n");
            return 1;
        }
        
    }
    else{ // if the incorrect number of arguments is given, print an error message
        module.shell_printf("error: poke expects 2 arguments [addr] and [val]\n");
        return 1; 
    }
    return 0;
}

// The shell_init function initializes the shell module by setting the input and output functions that the shell will use.
void shell_init(input_fn_t read_fn, formatted_fn_t print_fn) {
    module.shell_read = read_fn;
    module.shell_printf = print_fn;
}

// The shell_bell function produces a bell sound by printing the ASCII bell character.
void shell_bell(void) {
    uart_putchar('\a');
}

void shell_readline(char buf[], size_t bufsize) {
    int length = 0;
    int history_idx = n_cmd;

    char c = module.shell_read();
    int n_up = 0;
    while (c != '\n') {
        if (c ==  PS2_KEY_ARROW_UP) {  // up arrow — go back in history

            // printf("n_up: %d nhistory: %d\n" , n_up, nhistory);
            if(n_up < nhistory){
                n_up++;
    
                // erase current line
                for (int i = 0; i < length; i++) {
                    module.shell_printf("%c", '\b');
                    module.shell_printf(" ");
                    module.shell_printf("%c", '\b');
                }

                
                memcpy(buf, history[history_idx - 1], bufsize);
                // printf("buf: %s n_cmd: %d\n" , buf, history_idx);
                // for (int i = 0; i < 10; i++) {
                //     printf("%d: %s\n", i, history[i]);
                // }
                history_idx -= 1;
                length = strlen(buf);
                module.shell_printf("%s", buf);
            }
        }
        // when it reaches top, stop at the first history
        
        else if (c == '\b') {
            if (length < 1) {
                shell_bell();
                // break;
            } else {
                length -= 1;
                module.shell_printf("%c", '\b');
                module.shell_printf(" ");
                module.shell_printf("%c", '\b');
            }
        }
        else if (c ==  PS2_KEY_ARROW_DOWN) {  // down arrow — go forward in history
            
            if(n_up <= nhistory && n_up >=0){
                // printf("nup: %d \n", n_up);
                n_up--;

                history_idx = (history_idx + 1) % 10;
                // erase current line
                for (int i = 0; i < length; i++) {
                    module.shell_printf("%c", '\b');
                    module.shell_printf(" ");
                    module.shell_printf("%c", '\b');
                }
                memcpy(buf, history[history_idx], bufsize);
                length = strlen(buf);
                module.shell_printf("%s", buf);
            }
            //  first down key needs to be pressed twice: fix
        }
        else if (length >= bufsize - 1) {
            shell_bell();
            break;
        }
        else {
            if (c <= 0x7f) {
                buf[length] = c;
                length += 1;
                module.shell_printf("%c", c);
            }
        }

        c = module.shell_read();


    }
    buf[length] = '\0';
    module.shell_printf("\n");
}
// A helper function to check if a character is a whitespace character (space, tab, or newline). This is used for tokenizing the input line into commands and arguments.
static bool isspace(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\n';
}

// A helper function to duplicate a string up to a certain length, used for tokenizing the input line. It allocates memory for the new string and copies the specified number of characters from the source string, adding a null terminator at the end.
static char *strndup(const char *src, size_t n) {
    void* result = malloc(n + 1);
    memcpy(result, src, n);
    ((char*)result)[n] = '\0';
    return result;
}


// The shell_evaluate function takes a line of input and evaluates it as a shell command. It tokenizes the input line into command and arguments, checks if the command is valid, and executes the corresponding command function. If the command is not recognized or if there are errors in the arguments, it prints appropriate error messages.
int shell_evaluate(const char *line) {

    //divide line into array of tokens -> non space chars

    int max = strlen(line);
    if(max == 0){
        return 0;
    }   
    const char *tokens[max];

    memcpy(history[n_cmd], line, 128);
    // module.shell_printf("history: %s",history[n_cmd]);
    if(nhistory < 10){
        nhistory ++;
    }
    n_cmd = (n_cmd + 1) % 10;
    // n+=1;
    int ntokens = 0;
    const char *cur = line;

    while (ntokens < max) { // while there are more tokens to read
        while (isspace(*cur)) cur++;    // skip spaces (stop non-space/null)
        if (*cur == '\0') break;        // no more non-space chars
        const char *start = cur;
        while (*cur != '\0' && !isspace(*cur)) cur++; // advance to end (stop space/null)
        tokens[ntokens++] = strndup(start, cur - start);   // make heap-copy, add to array
    }

   

    if(strcmp(tokens[0],"echo") == 0){ // if the command is echo, execute the echo command function
        return cmd_echo(ntokens, tokens);
    }
    else if(strcmp(tokens[0],"help") == 0){ // if the command is help, execute the help command function
        return cmd_help(ntokens, tokens);
    }
    else if(strcmp(tokens[0],"reboot") == 0){ // if the command is reboot, execute the reboot command function
        return cmd_reboot(ntokens, tokens);
    }
    else if(strcmp(tokens[0],"clear") == 0){ // if the command is clear, execute the clear command function
        return cmd_clear(ntokens, tokens);
    }
    else if(strcmp(tokens[0],"peek") == 0){ // if the command is peek, execute the peek command function
        return cmd_peek(ntokens, tokens);
    }
    else if(strcmp(tokens[0],"poke") == 0){ // if the command is poke, execute the poke command function
        return cmd_poke(ntokens, tokens);
    }
    else if(strcmp(tokens[0],"history") == 0){ // if the command is poke, execute the poke command function
        return cmd_history(ntokens, tokens);
    }
    else if(strcmp(tokens[0],"") == 0){ // if the command is empty, execute nothing
        
        //module.shell_printf("\n");
        return 0;

    }
    
    else{
    // if the command is not found, print an error message
        module.shell_printf("error: no such command '%s' \n", tokens[0]); 
        return -1;
    }

}


// The shell_run function is the main loop of the shell. It prints a welcome message and then repeatedly prompts the user for input, reads a line of input, and evaluates it as a command.
void shell_run(void) {
    module.shell_printf("Welcome to the CS107E shell.\nRemember to type on your PS/2 keyboard!\n");
    while (1) { // main loop of the shell
        char line[LINE_LEN];

        module.shell_printf("Pi> ");
        shell_readline(line, sizeof(line));
        shell_evaluate(line);
    }
}
