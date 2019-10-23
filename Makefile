%: %.c
	gcc $@.c -o $@ -Wall -Werror -Wextra -lm -g -pipe -fsanitize=leak,address
	./cstyle.py $@.c
