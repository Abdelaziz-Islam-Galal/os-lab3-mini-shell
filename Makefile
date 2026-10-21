## you have command.cc command.h tokenizer.cc tokenizer.h you need compile them and link
## them to one object file called  : "myshell" 
## so we should run make , that will create our object myshell ,
## ./myshell should run your application 


# Compiler and Flags
CXX = g++ -g

# Target Executable Name
TARGET = myshell

# Source and Object Files
SRCS = command.cc tokenizer.cc
OBJS = $(SRCS:.cc=.o)

# Default target
all: $(TARGET)

# Linking the executable
$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS)

# Generic rule for compiling source files to object files
%.o: %.cc %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean