CC = gcc
WFLAGS =-fsanitize=address -fno-omit-frame-pointer -fno-lto -no-pie
LDFLAGS = -fsanitize=address -lm -no-pie 
CFLAGS = $(WFLAGS) -O0 -g3

RUN_DIR = /home/max/class/d277-front-end-web-development
OBJ_DIR = obj
BIN_DIR = bin

TARGET = $(BIN_DIR)/inet
SOURCES = inet.c \
		  parse.c \
		  arena.c 
OBJS = $(OBJ_DIR)/inet.o \
	   $(OBJ_DIR)/parse.o \
	   $(OBJ_DIR)/arena.o 


$(TARGET): $(OBJS) 
	@mkdir -p $(dir $@)
	$(CC) $^ -o $@ $(LDFLAGS) 
	sudo setcap cap_sys_chroot+eip $(TARGET)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c -o $@ $^ $(CFLAGS)

run: $(TARGET) 
	$(TARGET) $(RUN_DIR) 2> log.txt || true

debug: run 
	/usr/bin/cat log.txt | grep -oP '0x[0-9a-f]{6}\)' | sed s/\)// | llvm-symbolizer -e ./bin/inet >> log.txt
	/usr/bin/cat log.txt

clean:
	rm -rf $(OBJ_DIR) 
	rm -rf $(BIN_DIR)
