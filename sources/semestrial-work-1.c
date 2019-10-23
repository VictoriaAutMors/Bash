#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <err.h>
#include <fcntl.h>
#include <assert.h>

void free_list(char **list)
{
    int i = 0;
    while (list[i] != NULL) {
        free(list[i]);
        i++;
    }
    free(list);
}

char *get_word(char *end)
{
    if (*end == '\n') { // no more lexemes
        return NULL;
    }
    int i = 0;
    char ch, *word = NULL, *check = NULL;
    do {
        ch = getchar();
        while (!i && (ch == ' ' || ch == '\t')) { // search for first letter
            ch = getchar();
            if (ch == '\n') {
                return NULL;
            }
        }
        check = (char *)realloc(word, (i + 1) * sizeof(char));
        if (check == NULL) {
            err(1, NULL);
        }
        word = check;
        word[i] = ch;
        i++;
    } while (ch != ' ' && ch != '\t' && ch != '\n');
    word[i - 1] = '\0'; // set end of the lexeme
    *end = ch;
    return word;
}

char **get_list(void)
{
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
        i++;
    } while (list[i - 1] != NULL);
    return list;
}

void rm_str(int num, char **list)
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

ssize_t special_case(char **list)
{
    ssize_t fd;
    int i = 0;
    char ch;
    while (list[i] != NULL) {
        ch = list[i][0];
        if (ch == '>' || ch == '<') {
            if (ch == '>') {
                fd = open(list[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0755);
                if (fd < 0) {
                    free_list(list);
                    err(1, NULL);
                }
                dup2(fd, 1);
            } else {
                fd = open(list[i + 1], O_RDONLY);
                if (fd < 0) {
                    free_list(list);
                    err(1, NULL);
                }
                dup2(fd, 0);
            }
            rm_str(i, list); // remove ">" or "<"
            rm_str(i, list); // remove opened file name
            return fd;
        }
        i++;
    }
    return 0;
}

int main(void)
{
    ssize_t fd, pd[2];
    pid_t pid;
    char **list = NULL;
    while (1) {
        list = get_list(); 
        if (list != NULL && (!strcmp(list[0], "exit") || !strcmp(list[0], "quit"))) {
            free_list(list);
            return 0;
        }
        pid = fork();
        if (pipe(pd) < 0) {
            err(1, NULL);
        }
        if (pid < 0) {
            free_list(list);
            err(1, NULL);
        }
        if (pid == 0) { // execute in child process
            close(pd[0]);
            close(pd[1]);
            fd = special_case(list); // check for special symbols like "<" or ">"
            if (execvp(list[0], list) < 0) {
                err(1, NULL);
            }
            free_list(list);
            if (fd != 0) {
                close(fd);
            }
            return 0; // close child process
        }
        close(pd[1]);
        wait(NULL); // waiting for child to end his process
        close(pd[0]);
        free_list(list);
    }
    return 1;
}
