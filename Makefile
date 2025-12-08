# Simple Makefile for Storm-Deck-Calculator
# Builds all .c files in the repository root into a single executable.

CC ?= gcc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -g
LDFLAGS ?=
TARGET ?= storm-deck

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)

.PHONY: all clean run run-win

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

run: all
	./$(TARGET)

ifeq ($(OS),Windows_NT)
run-win: all
	@powershell -Command ".\\$(TARGET).exe"

clean:
	@echo Cleaning windows build artifacts...
	@if exist $(TARGET).exe del /Q $(TARGET).exe > nul 2>&1
	@for %%F in (*.o) do if exist "%%F" del /Q "%%F" > nul 2>&1
else
run-win:
	@echo Not Windows; use 'make run' to run on Unix-like shells

clean:
	@echo Cleaning...
	@rm -f $(OBJS) $(TARGET)
endif

# Notes:
# - To build: make
# - To run (Unix): make run
# - To run on Windows with PowerShell: make run-win
# - To clean: make clean
