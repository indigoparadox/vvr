
# vim: ft=make noexpandtab

dump: parse.c dump.c
	gcc -o $@ $^

clean:
	rm -rf dump
   
