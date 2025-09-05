PROJECT_NAME = lambda-interpreter

CC = gcc

CFLAGS = 

ifeq ($(BUILD_MODE), DEBUG)
	CFLAGS += -g -O0
else
	CFLAGS += -s -O1
endif

INCLUDE_PATHS = -Isrc

SRC_DIR = src
OBJ_DIR = obj

SRC  = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c,obj/%.o,$(SRC))

# Project target defined by PROJECT_NAME
$(PROJECT_NAME): $(OBJS)
	$(CC) -o $(PROJECT_NAME) $(OBJS) $(CFLAGS) $(INCLUDE_PATHS)

# Compile source files
# NOTE: This pattern will compile every module defined on $(OBJS)
#%.o: %.c
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDE_PATHS)

obj:
	mkdir -p obj

clean:
	rm -rf obj