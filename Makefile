
# vim: ft=make noexpandtab

dump: obj/parse.o obj/dump.o
	gcc -o $@ $^

obj/%.o: src/%.c
	mkdir -p $(dir $@)
	gcc -c -o $@ $<

clean:
	rm -rf dump obj
   
