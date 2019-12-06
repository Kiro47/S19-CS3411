### Might need customized
# Final executable/program name
EXECUTABLE_NAME_CLIENT ?= cclient
EXECUTABLE_NAME_SERVER ?= cserver
# Compiler
CC ?= gcc
# C/CPP standard flag
C_STANDARD ?= -ansi
# C/CPP Compile Flags
PROJECT_CC_FLAGS ?= -pedantic -Wall
# Where source code is located (Probably don't change me)
SOURCE_DIRS ?= .

### Don't touch me
# Arg aggregations
PARGS ?=
CC_FLAGS ?= $(INC_FLAGS) $(C_STANDARD) $(PROJECT_CC_FLAGS)
SOURCE_FILES := $(shell find $(SOURCE_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.s')
BUILD_OBJECTS := $(addsuffix .o,$(basename $(SOURCE_FILES)))
BUILD_DEPENDENCIES := $(BUILD_OBJECTS:.o=.d)

debug: CC_FLAGS += -ggdb
debug: CCFLAGS += -ggdb
debug: CFLAGS += -ggdb

INC_DIRS := $(shell find $(SOURCE_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# Multi Bin Obj Links

CLIENT_BUILD_OBJECTS := $(filter-out ./$(EXECUTABLE_NAME_SERVER).o,$(BUILD_OBJECTS))
SERVER_BUILD_OBJECTS := $(filter-out ./$(EXECUTABLE_NAME_CLIENT).o,$(BUILD_OBJECTS))

# alias: all -> standard build
.PHONY: all
all:$(EXECUTABLE_NAME_SERVER) $(EXECUTABLE_NAME_CLIENT)

# Standard build -> Client
.PHONY: $(EXECUTABLE_NAME_CLIENT)
$(EXECUTABLE_NAME_CLIENT): $(CLIENT_BUILD_OBJECTS)
	$(CC) $(LDFLAGS) $(CLIENT_BUILD_OBJECTS) $(LOADLIBES) $(LDLIBS) -o $(EXECUTABLE_NAME_CLIENT)

# Standard build -> Server
.PHONY: $(EXECUTABLE_NAME_SERVER)
$(EXECUTABLE_NAME_SERVER): $(SERVER_BUILD_OBJECTS)
	$(CC) $(LDFLAGS) $(SERVER_BUILD_OBJECTS) $(LOADLIBES) $(LDLIBS) -o $(EXECUTABLE_NAME_SERVER)


# Enable all debug symbols
.PHONY: debug
debug:$(EXECUTABLE_NAME_SERVER) $(EXECUTABLE_NAME_CLIENT)
#	$(CC) $(LDFLAGS) $(CLIENT_BUILD_OBJECTS) $(LOADLIBES) $(LDLIBS) -ggdb -o $(EXECUTABLE_NAME_CLIENT)
#	$(CC) $(LDFLAGS) $(SERVER_BUILD_OBJECTS) $(LOADLIBES) $(LDLIBS) -ggdb -o $(EXECUTABLE_NAME_SERVER)

# Run through valgrind to check for mem leaks
# Pass PARGS=$args on make command to pass for usage
# Conveniently annoying for multi executables.  Disabled
#.PHONY: memory
#memory:debug
#	valgrind "--tool=memcheck --track-origins=yes --leak-check=full $(shell pwd)/$(EXECUTABLE_NAME)" "$(PARGS)"

# Production build, debug symbols stripped
.PHONY: prod
prod:$(EXECUTABLE_NAME)
	$(shell strip $(EXECUTABLE_NAME_CLIENT))
	$(shell strip $(EXECUTABLE_NAME_SERVER))

# Clean all the things
.PHONY: clean
clean:
	$(RM) $(EXECUTABLE_NAME_CLIENT) $(EXECUTABLE_NAME_SERVER) $(BUILD_OBJECTS) $(BUILD_DEPENDENCIES)

-include $(BUILD_DEPENDENCIES)
