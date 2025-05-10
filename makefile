CC = gcc

exec := term

build_dir :=  ./build
src_dir = ./src

srcs = $(shell find $(src_dir) -name *.c)

# $(srcs:.o=.c) is equal to $(srcs:%.o=%.c)
# anything in srcs is replaced with build_dir/file.c.o
objs = $(srcs:%=$(build_dir)/%.o)
#objs = $(src:%=%.o)

ldflags := -lX11 -lXft

$(exec): $(objs)
	$(CC) -o $@ $(objs) $(ldflags)

#$< is the first prereq
$(build_dir)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) -o $@ -c $< -I/usr/include/freetype2 -I/usr/include/X11/Xft 

.PHONY: clean
clean:
	rm -r $(build_dir)
