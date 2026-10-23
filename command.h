
#ifndef command_h
#define command_h

#include <stdio.h> // for FILE that I added to the Logger struct
#include "tokenizer.h"

struct SimpleCommand
{
	int _numberOfAvailableArguments;
	int _numberOfArguments;
	char **_arguments;

	SimpleCommand();
	void insertArgument(char *argument);
};

struct Command
{
	int _numberOfAvailableSimpleCommands;
	int _numberOfSimpleCommands;
	SimpleCommand **_simpleCommands;
	char *_outFile;
	char *_inputFile;
	char *_errFile;
	int _out_error;
	int _background;
	int _append;

	void prompt();								   // -> parse -> execute
	static void parse(std::vector<Token> &tokens); // -> execute
	void print();
	void execute();
	void clear();

	Command();
	~Command(); // destructor so after any return free allocations
	void insertSimpleCommand(SimpleCommand *simpleCommand);

	static Command _currentCommand;
	static SimpleCommand *_currentSimpleCommand;

	struct Logger
	{
		static FILE *_logFile;

		static void log_execution(int signum); // signal handler for SIGCHLD
	};
};

#endif
