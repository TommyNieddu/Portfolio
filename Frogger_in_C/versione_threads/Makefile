TARGET = frogger_threads

# -Wall: warning, -g: debug, -pthread: per libreria pthread
CFLAGS  = -Wall -g -pthread
# Librerie da linkare
LDFLAGS = -lncurses -pthread

SRCS = main.c padre.c npc_rana.c regole_gioco.c prato_tane.c buffer.c


OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)
