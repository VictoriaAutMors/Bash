all: shell.c
	gcc shell.c -o shell -Wall -Werror -Wextra -O2

%: %.c
	gcc $@.c -o $@ -Wall -Werror -Wextra -lm -g -pipe -fsanitize=leak,address,undefined,null -Ofast
