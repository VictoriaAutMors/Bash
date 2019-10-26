#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <err.h>
#include <fcntl.h>
#include <limits.h>

int ispipe = 0;

/* functions to free heap after program execution*/

void rm_string(int num, char **list)
{
    char *temp;
    printf("LIN%d\n", num);
    while (list[num + 1] != NULL) { /* swaping i - string with other strings
                                    until end of the list */
        temp = list[num];
        list[num] = list[num + 1];
        list[num + 1] = temp;
        num++;
    }
    list[num] = NULL; // set new end of the list
    free(list[num + 1]); // free previous end of the list
}

void free_list(char **list)
{
    int i = 0;
    while (list[i] != NULL) {
        free(list[i]);
        i++;
    }
    free(list[i]);
    free(list);
}

void free_catalog(char ***catalog)
{
    int i = 0;
    while (catalog[i] != NULL) {
        free_list(catalog[i]);
        i++;
    }
    free(catalog[i]);
    free(catalog);
}

/*  functions to fill array*/

int is_special_symbol(char ch)
{
    return (ch == '>' || ch == '<' || ch == '|');
}

char *get_quote(char mark)
{
    if (mark != '"') {
        return NULL;
    }
    int i = 0;
    char ch, *word = NULL;
    do {
        ch = getchar();
        word = (char *)realloc(word, (i + 1) * sizeof(char));
        if (word == NULL) {
            err(1, NULL);
        }
        word[i] = ch;
        i++;
    } while (ch != '"');
    word[i - 1] = '\0'; // set end of the quote;
    return word;
}

char get_first_letter(char ch)
{
    while (ch == ' ' || ch == '\t') {
        ch = getchar(); // get symbols until it not a letter or '\n'
    }
    return ch;
}

char *separate_symbol_word(char ch, char *end)
{
    if (is_special_symbol(ch)) {
        char *word = (char *)calloc(3, sizeof(char));
        if (word == NULL) {
            err(1, "failed to allocate memory in separate symbol and word");
        }
        word[1] = ch;
        *end = ' '; // separate word and special symbol by space
        return word;
    }
    return NULL;
}

char *word_special_case(char ch, char *end)
{
    char *word = get_quote(ch);
    if (word) {
        return word;
    }
    word = separate_symbol_word(ch, end);
    return word;
}

char *get_word(char *end)
{
    if (*end == '\n' || *end == '|') { // no more lexemes
        return NULL;
    }
    int i = 0;
    char ch, *word = NULL;
    do {
        ch = getchar();
        if (!i) {
            ch = get_first_letter(ch);
            if (ch == '\n' || ch == '|') {
                *end = ch;
                return NULL;
            }
            word = word_special_case(ch, end);
            if (word) {
                return word;
            }
        }
        word = (char *)realloc(word, (i + 1) * sizeof(char));
        if (word == NULL) {
            err(1, "failed reallocate memory in get word");
        }
        word[i] = ch;
        i++;
    } while (ch != ' ' && ch != '\t' && ch != '\n' && ch != '|');
    word[i - 1] = '\0'; // set end of the lexeme
    *end = ch;
    return word;
}

char **get_list(char *end_of_line)
{
    if (*end_of_line == '\n') {
        return NULL;
    }
    char end = 0, **list = NULL, **check = NULL;
    int i = 0;
    do {
        check = (char **)realloc(list, (i + 1) * sizeof(char *));
        if (check == NULL) {
            free_list(list);
            err(1, NULL);
        }
        list = check;
        list[i] = get_word(&end);
        if (!list[0]) {
            free_list(list);
            return NULL;
        }
        i++;
    } while (list[i - 1] != NULL);
    *end_of_line = end;
    return list;
}

char ***get_catalog(int *cmds)
{
    char end_of_line = 0, ***catalog = NULL;
    int i = 0;
    do {
        catalog = (char ***)realloc(catalog, (i + 1) * sizeof(char **));
        catalog[i] = get_list(&end_of_line);
        i++;
    } while (catalog[i - 1] != NULL);
    *cmds = i - 2;
    return catalog;
}

/*functions to change input and output */

int is_bg_proc(char **list)
{
    int i = 0;
    while (list[i] != NULL) {
        if (list[i][0] == '&')
        {
            rm_string(i, list);
            return 1;
        }
        i++;
    }
    return 0;
}

ssize_t change_IO(char **list)
{
    ssize_t fd = 0;
    int i = 0;
    char ch;
    while (list[i] != NULL) {
        if (!list[i][0]) {
            ch = list[i][1];
            if (ch == '>') {
                fd = open(list[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0755);
                if (fd < 0) {
                    free_list(list);
                    err(1, NULL);
                }
                dup2(fd, STDOUT_FILENO);
            } else {
                fd = open(list[i + 1], O_RDONLY);
                if (fd < 0) {
                    free_list(list);
                    err(1, NULL);
                }
                dup2(fd, STDIN_FILENO);
            }
            rm_string(i, list); // remove ">" or "<"
            rm_string(i, list); // remove opened file name
            return fd;
        }
        i++;
    }
    return fd;
}

/* function to execute commands */

void execute(char **list)
{
    ssize_t fd, fd2;
    fd = change_IO(list); // check for special symbols like "<" or ">"
    fd2 = change_IO(list); // check for the second one
    if (execvp(list[0], list) < 0) {
        err(1, NULL);
    }
    free_list(list);
    if (fd != 0) {
        close(fd);
    }
    if (fd2 != 0) {
        close(fd2);
    }
}

void print_list(char **list)
{
    int i = 0;
    while (list[i] != NULL) {
        puts(list[i]);
        printf("V%d\n", i + 1);
        i++;
    }
}

void print_list_list(char ***list)
{
    int i = 0;
    while (list[i] != NULL) {
        print_list(list[i]);
        printf("%d\n", i + 1);
        i++;
    }
}

void create_pipe(int i, int **pd)
{
    if (!ispipe) {
        return;
    }
    if (!(i % 2)) {
        dup2(pd[i][STDIN_FILENO], 0);
    } else {
        dup2(pd[i][STDOUT_FILENO], 1);        
    }
    close(pd[i][STDIN_FILENO]);
    close(pd[i][STDOUT_FILENO]);
    return;
}

void wait_bg_proc(int bg_proc_count)
{
    for (int i = 0; i < bg_proc_count; i++)
    {
        wait(NULL);
    }
}

void start_line(void)
{
    char hostname[_SC_HOST_NAME_MAX];
    if (gethostname(hostname, _SC_HOST_NAME_MAX)) {
        err(1, NULL);
    }
    printf("\033[1;32m%s@%s\033[0m:\033[1;34m%s\033[0m$ ",
            getenv("USER"), hostname, getenv("PWD"));
}

int execcat(void)
{
//  int (*pd)[2];
    pid_t pid;
    int i, j, cmds, bg_proc_count = 0;
    char ***catalog = NULL;
    while (1) {
        start_line();
        catalog = get_catalog(&cmds);
        if (!catalog) {
            err(1, "failed to create catalog");
        }
        if (catalog[0] != NULL && (!strcmp(catalog[0][0], "exit") || !strcmp(catalog[0][0], "quit"))) {
            free_catalog(catalog);
            wait_bg_proc(bg_proc_count);
            return EXIT_SUCCESS;
        }
        i = 0;
        while (catalog[i] != NULL) {
//            if (!(pipe(pd[pipes - 1])) && !(pipe(pd[pipes]))) {
//                free_catalog(catalog);
//                err(1, "failed to create a pipe");
//            }
            j = is_bg_proc(catalog[cmds]);
            bg_proc_count += j;
            pid = fork();
            if (pid < 0) {
                free_catalog(catalog);
                err(1, NULL);
            }
            if (pid == 0) { // execute in child process
                if (j) {
                    printf("[%d]    %d\n", bg_proc_count, getpid());
                }
                execute(catalog[i]);
                return EXIT_SUCCESS; // close child process
            }
            if (1) {
                wait(NULL); // waiting for child to end his process
            }
            i++;
            j++;
        }
        free_catalog(catalog);
    }
    return EXIT_FAILURE;
}

int main(void)
{
    if(execcat()) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
