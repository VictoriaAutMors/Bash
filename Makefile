%: %.c
	gcc $@.c -o $@ -Wall -Werror -Wextra -lm -g -pipe -fsanitize=leak,address,undefined,null -Ofast
	./cstyle.py $@.c
