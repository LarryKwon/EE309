.PHONY: all clean grade

CC = gcc209

INC_DIR = include
SRC_DIR = src
OBJ_DIR = obj

TEST_DIR = ./ee309_test

CFLAGS = -Wall -g -D_GNU_SOURCE
CPPFLAGS = $(addprefix -I,$(shell find $(SRC_DIR) -type d))

PROG = ls309

SRC = $(shell find $(SRC_DIR) -name '*.c')
OBJ = $(patsubst %.c,%.o,$(notdir $(SRC)))

vpath %.c $(dir $(SRC))
vpath %.o $(OBJ_DIR)

.PRECIOUS: $(OBJ) %.o

all: $(PROG) $(PROG_OFS)

$(PROG): $(addprefix $(OBJ_DIR)/,$(OBJ))
	$(CC) -o $@ $^

$(OBJ_DIR)/%.o: %.c
	mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

grade: $(PROG)
	./run_test.py

clean:
	rm -rf $(PROG) $(PROG_OFS) $(OBJ_DIR) $(TEST_DIR)
