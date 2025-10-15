CC      := gcc
CFLAGS  := -Wall -Wextra -g -I.
LDFLAGS := -L. -lseg -lm

SRC     := client.c
OBJ     := $(SRC:.c=.o)
BIN     := client
DEPS    := segdef.h

all: $(BIN)

$(BIN): $(OBJ)
	@echo "Édition de liens : $@"
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c $(DEPS)
	@echo "Compilation : $<"
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Nettoyage..."
	rm -f $(OBJ) $(BIN) *~ core

indent:
	@echo "Formatage du code..."
	indent -linux -i4 -nut -ts2 *.c

.PHONY: all clean indent run