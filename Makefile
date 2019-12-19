all: shell.c
	mkdir bin -p
	gcc ./sources/shell.c -o ./bin/shell -Wall -Werror -Wextra -O2

clean:
	rm /bin/*
	rmdir bin
%: %.c
	gcc $@.c -o $@ -Wall -Werror -Wextra -lm -g -pipe -fsanitize=leak,address,undefined,null -Ofast
