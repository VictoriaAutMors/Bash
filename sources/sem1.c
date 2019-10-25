#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <err.h>
#include <fcntl.h>
#include <assert.h>

#define PIPE_READ 0
#define PIPE_WRITE 1

void free_list(char **list)
{
    int i = 0;
    while (list[i] != NULL) {
        free(list[i]);
        i++;
    }
    free(list);
}

/*  functions to fill our */

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
    if (*end == '\n' || *end == '|' || *end == '&') { // no more lexemes
        return NULL;
    }
    int i = 0;
    char ch, *word = NULL;
    do {
        ch = getchar();
        if (!i) {
            ch = get_first_letter(ch);
            if (ch == '\n' || ch == '|' || ch == '&') {
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

char ***get_catalog(void)
{
    char end_of_line = 0, ***catalog = NULL;
    int i = 0;
    do {
        catalog = (char ***)realloc(catalog, (i + 1) * sizeof(char **));
        catalog[i] = get_list(&end_of_line);
        i++;
    } while (catalog[i - 1] != NULL);
    return catalog;
}

void rm_string(int num, char **list)
{
    char *temp;
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

void print_list(char **list)
{
    int i = 0;
    while (list[i] != NULL) {
        puts(list[i]);
        i++;
    }
}

ssize_t special_case(char **list)
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

void execute(char **list)
{
    ssize_t fd, fd2;
    fd = special_case(list); // check for special symbols like "<" or ">"
    fd2 = special_case(list);
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

void free_catalog(char ***catalog)
{
    int i = 0;
    while (catalog[i] != NULL) {
        free_list(catalog[i]);
        i++;
    }
    free(catalog);
}

void print_list_list(char ***list)
{
    int i = 0;
    while (list[i] != NULL) {
        print_list(list[i]);
        printf(" %d\n", i + 1);
        i++;
    }
}

int execcat(void)
{
    pid_t pid;
    int i = 0;
    char ***catalog = NULL;
    while (1) {
        catalog = get_catalog();
        if (catalog != NULL && catalog[0] != NULL && (!strcmp(catalog[0][0], "exit") || !strcmp(catalog[0][0], "quit"))) {
            free_catalog(catalog);
            return 0;
        }
        i = 0;
        while (catalog[i] != NULL) {
            pid = fork();
            if (pid < 0) {
                free_catalog(catalog);
                err(1, NULL);
            }
            if (pid == 0) { // execute in child process
                execute(catalog[i]);
                return 0; // close child process
            }
            wait(NULL); // waiting for child to end his process
            i++;
        }
        free_catalog(catalog);
    }
    return 1;
}

int main(void)
{
    execcat();
    return 0;
}
