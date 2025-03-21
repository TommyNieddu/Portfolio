TARGET = frogger_processi

# -Wall: warning, -g: debug
CFLAGS  = -Wall -g
# Libreria ncurses (non serve -pthread se non usi thread)
LDFLAGS = -lncurses

SRCS = main.c padre.c npc_rana.c regole_gioco.c prato_tane.c 

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
