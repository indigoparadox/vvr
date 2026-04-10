
# vim: ft=make noexpandtab

all: dump d3dgl

d3dgl: CFLAGS := $(shell pkg-config opengl glut --cflags)

dump: obj/vvr.o obj/dump.o
	gcc -o $@ $^

d3dgl: obj/vvr.o obj/d3dgl.o
	gcc -o $@ $^ $(shell pkg-config opengl glut --libs)

obj/%.o: src/%.c
	mkdir -p $(dir $@)
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -rf dump d3dgl obj
   
