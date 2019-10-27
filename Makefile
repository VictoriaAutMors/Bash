%: %.c
	gcc $@.c -o $@ -Wall -Werror -Wextra -lm -g -pipe -fsanitize=leak,address,undefined,null -O2
	./cstyle.py $@.c
