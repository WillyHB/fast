CC = gcc

EXEC := term

BUILD_DIR :=  ./build
SRC_DIR = ./src

SRCS = $(shell find $(SRC_DIR) -name *.c)

# $(srcs:.o=.c) is equal to $(srcs:%.o=%.c)
# anything in srcs is replaced with build_dir/file.c.o
OBJS = $(srcs:%=$(BUILD_DIR)/%.o)
#objs = $(src:%=%.o)

LDFLAGS := -lX11 -lXft
LIBRARIES := -I/usr/include/freetype2 -I/usr/include/X11/Xft 

$(EXEC): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

#$< is the first prereq
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) -o $@ -c $< $(LIBRARIES)

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)
