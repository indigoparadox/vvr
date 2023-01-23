
# vim: ft=make noexpandtab

parse: parse.c
	gcc -o parse parse.c

clean:
	rm -rf parse
   
