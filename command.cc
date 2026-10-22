#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <glob.h>
#include <wordexp.h>
#include <vector>
#include <string>
#include <cctype>
#include <iostream>
#include <sstream>
#include "command.h"
#include "tokenizer.h"

void parse(std::vector<Token> &tokens)
{
    Command currentCommand;
    SimpleCommand *currentSimpleCommand = new SimpleCommand();

    if (tokens.empty() || tokens[0].type == TOKEN_EOF)
        return;
    
    int size = tokens.size();
    for (size_t i = 0; i < size; i++)
    {
        Token token = tokens[i];
        // since the method clear() already frees the memory of the current command, I will create new mallocs - strdup
        switch (token.type)
        {
        case TOKEN_COMMAND:
            currentSimpleCommand->insertArgument(strdup(token.value.c_str()));
            break;
        case TOKEN_ARGUMENT:
            currentSimpleCommand->insertArgument(strdup(token.value.c_str()));
            break;
        case TOKEN_REDIRECT:
            if (i + 1 >= size || tokens[i + 1].type != TOKEN_ARGUMENT)
            {
                fprintf(stderr, "Error: Missing filename for output redirection.\n");
                return;
            }
            token = tokens[++i];
            currentCommand._outFile = strdup(token.value.c_str());
            break;
        case TOKEN_APPEND:
            if (i + 1 >= size || tokens[i + 1].type != TOKEN_ARGUMENT)
            {
                fprintf(stderr, "Error: Missing filename for output redirection (append mode).\n");
                return;
            }
            currentCommand._append = 1;
            token = tokens[++i];
            currentCommand._outFile = strdup(token.value.c_str());
            break;
        case TOKEN_INPUT:
            if (i + 1 >= size || tokens[i + 1].type != TOKEN_ARGUMENT)
            {
                fprintf(stderr, "Error: Missing filename for input redirection.\n");
                return;
            }
            token = tokens[++i];
            currentCommand._inputFile = strdup(token.value.c_str());
            break;
        case TOKEN_ERROR:
            if (i + 1 >= size || tokens[i + 1].type != TOKEN_ARGUMENT)
            {
                fprintf(stderr, "Error: Missing filename for error redirection.\n");
                return;
            }
            token = tokens[++i];
            currentCommand._errFile = strdup(token.value.c_str());
            break;
        case TOKEN_PIPE:
            currentCommand.insertSimpleCommand(currentSimpleCommand);
            currentSimpleCommand = new SimpleCommand();
            break;
        case TOKEN_BACKGROUND:
            currentCommand._background = 1;
            break;
        case TOKEN_REDIRECT_AND_ERROR:
            if (i + 1 >= size || tokens[i + 1].type != TOKEN_ARGUMENT)
            {
                fprintf(stderr, "Error: Missing filename for output and error redirection.\n");
                return;
            }
            currentCommand._out_error = 1;
            token = tokens[++i];
            currentCommand._outFile = strdup(token.value.c_str());
            currentCommand._errFile = strdup(token.value.c_str());
            break;
        default:
            break;
        }
    }
    currentCommand.insertSimpleCommand(currentSimpleCommand);

    currentCommand.execute();
    /*
      Implement this function
      you must parse for all possible tokens scenario
      right now command.execute only print the format , you will inject it in your logic.

    */
}

SimpleCommand::SimpleCommand()
{
    _numberOfAvailableArguments = 5;
    _numberOfArguments = 0;
    _arguments = (char **)malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument)
{
    if (_numberOfAvailableArguments == _numberOfArguments + 1)
    {
        _numberOfAvailableArguments *= 2;
        _arguments = (char **)realloc(_arguments,
                                      _numberOfAvailableArguments * sizeof(char *));
    }

    _arguments[_numberOfArguments] = argument;
    _arguments[_numberOfArguments + 1] = NULL;

    _numberOfArguments++;
}

Command::Command() // constructor
{
    _numberOfAvailableSimpleCommands = 1;
    _simpleCommands = (SimpleCommand **)
        malloc(_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *)); // in the base code it was _numberOfSimpleCommands which was not yet initialized!

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
    _append = 0;
    _out_error = 0;
}

Command::~Command() // destructor so after any return free allocations
{
    clear();
    free(_simpleCommands);
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand)
{
    if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands)
    {
        _numberOfAvailableSimpleCommands *= 2;
        _simpleCommands = (SimpleCommand **)realloc(_simpleCommands,
                                                    _numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
    }

    _simpleCommands[_numberOfSimpleCommands] = simpleCommand;
    _numberOfSimpleCommands++;
}

void Command::clear()
{
    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            free(_simpleCommands[i]->_arguments[j]);
        }

        free(_simpleCommands[i]->_arguments);
        free(_simpleCommands[i]);
    }

    if (_outFile)
    {
        free(_outFile);
    }

    if (_inputFile)
    {
        free(_inputFile);
    }

    if (_errFile)
    {
        free(_errFile);
    }

    _numberOfSimpleCommands = 0;
    _outFile = 0;
    _inputFile = 0;
    _errFile = 0;
    _background = 0;
    _append = 0;    // reset append flag
    _out_error = 0; // reset out_error flag
}

void Command::print()
{
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");

    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        printf("  %-3d ", i);
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++)
        {
            printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
        }
    }

    printf("\n\n");
    printf("  Output       Input        Error        Err&Out       Background\n");
    printf("  ------------ ------------ ------------ ------------ ------------\n");
    printf("  %-12s %-12s %-12s %-12s %-12s\n", _outFile ? _outFile : "default",
           _inputFile ? _inputFile : "default", _errFile ? _errFile : "default", _out_error == 1 ? _errFile : "default", _background ? "YES" : "NO");
    printf("\n\n");
}

void Command::execute()
{
    print();

    if (_numberOfSimpleCommands == 0)
    {
        clear();
        return;
    }
    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        if (_simpleCommands[i]->_numberOfArguments == 0)
        {
            fprintf(stderr, "syntax error: there is an empty command\n");
            clear();
            return;
        }
    }

    char **first = _simpleCommands[0]->_arguments;
    if (strcmp(first[0], "exit") == 0) // needs to be checked here not in a child so the exit(0) actually terminates this process not just the child
        exit(0);
    if (strcmp(first[0], "cd") == 0 && _numberOfSimpleCommands == 1) // cd does not exist as an executable command. (And it cannot, because a process can only change the working directory of itself, not of its parent.) -> ref: https://stackoverflow.com/questions/18686114/cd-command-not-working-with-execvp
    {
        const char *dir = first[1] ? first[1] : getenv("HOME");
        if (!dir)
            fprintf(stderr, "cd: HOME variable is not set; hence, cd defaulting to root failed\n");
        if (chdir(dir) < 0)
            perror("cd");
        clear();
        return;
    }

    // save main descriptors
    const int defaultin = dup(0);
    const int defaultout = dup(1);
    const int defaulterr = dup(2);

    // setting redircetions
    // finalout is used instead of outfd because outfd is used for the pipe and will be closed in the loop; hence, finalout will be used for the last output
    int infd = _inputFile ? open(_inputFile, O_RDONLY) : dup(defaultin);
    int finalout = _outFile ? open(_outFile, O_WRONLY | O_CREAT | (_append ? O_APPEND : O_TRUNC), 0666) : dup(defaultout);
    int errfd = defaulterr;
    if (_out_error)
    {
        errfd = dup(finalout);
    }
    else if (_errFile)
    {
        errfd = open(_errFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    }

    if (infd < 0 || finalout < 0 || errfd < 0)
    {
        perror("open failed");
        if (infd >= 0)
            close(infd);
        if (finalout >= 0)
            close(finalout);
        if (errfd >= 0)
            close(errfd);
        close(defaultin);
        close(defaultout);
        close(defaulterr);
        clear();
        return;
    }

    std::vector<pid_t> pids; // I need to save all the pid values to wait for each
    fflush(stdout);

    for (int i = 0; i < _numberOfSimpleCommands; i++)
    {
        dup2(infd, 0);
        close(infd);

        // set up outfd redirection for the last command or create a pipe for intermediate commands
        int outfd;
        if (i == _numberOfSimpleCommands - 1)
        {
            outfd = finalout;
            finalout = -1;
        }
        else
        {
            int fdpipe[2];
            if (pipe(fdpipe) < 0)
            {
                perror("pipe");
                close(infd);
                close(finalout);
                close(errfd);
                // defaults are closed out of the loop
                clear();
                break;
            }
            outfd = fdpipe[1];
            infd = fdpipe[0];
        }

        // set up redirection (using outfd and errfd)
        dup2(outfd, 1);
        close(outfd);
        dup2(errfd, 2); // set it for each child/command, so that if it is a file it needs setting per each

        // set up the child process to execute the command
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork");
            close(infd);
            close(finalout);
            close(errfd);
            // defaults are closed out of the loop
            break;
        }
        if (pid == 0)
        {
            close(defaultin);
            close(defaultout);
            close(defaulterr);
            close(errfd);
            close(finalout); // stays open before the child creation
            if (i < _numberOfSimpleCommands - 1)
                close(infd); // unused read end
            execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
            perror(_simpleCommands[i]->_arguments[0]); // mention the command name in the error message
            _exit(EXIT_FAILURE);
        }
        pids.push_back(pid);
    }
    if (finalout >= 0)
        close(finalout); // loop broke early due to an error

    // restore the shell's own descriptors
    dup2(defaultin, 0);
    dup2(defaultout, 1);
    dup2(defaulterr, 2);
    close(defaultin);
    close(defaultout);
    close(defaulterr);

    if (!_background)
        for (pid_t p : pids)
            waitpid(p, NULL, 0);

    printf("\n");
    clear();
    // prompt(); // I moved it to a loop in prompt so execute() doesn't keep stacking upove each other infinetly
}

void Command::prompt()
{
    std::string input;
    while (true)
    {
        printf("myshell>");
        fflush(stdout);
        if (!std::getline(std::cin, input))
            break; // Ctrl-D
        if (input.empty())
            continue;
        std::vector<Token> tokens = tokenize(input);
        parse(tokens); // parse -> execute -> clear, then return, loop takes over and prompt again
    }
}

Command Command::_currentCommand;
SimpleCommand *Command::_currentSimpleCommand;

int main()
{
    Command::_currentCommand.prompt(); // prompt -> parse -> execute -> clear -> prompt ...

    return 0;
}
