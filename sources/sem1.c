#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <pwd.h>

#define PURPLE "\033[38;5;141m"
#define BLUE "\033[38;5;57m "
#define RESET "\033[0m"

/* functions to free heap after program execution*/

void free_string_in_list(int num, char **list)
{
    free(list[num]);
    while (list[num + 1] != NULL) { /* pushing i's string to the end of the list */
        list[num] = list[num + 1];
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

/* functions to work with background processes */

int is_bg_proc(char **list)
{
    int i = 0;
    while (list[i] != NULL) {
        if (list[i][0] == '&' && list[i][1] == '\0') {
            return i;
        }
        i++;
    }
    return 0;
}

void bg_proc_start(int bg_proc, char **list)
{
    if (bg_proc) {
        printf("%s PID:%d\n", list[0], getpid());
        free_string_in_list(bg_proc, list);
    }
}

void bg_proc_wait(pid_t pid)
{
    pid_t status, child;
    child = waitpid(pid, &status, WNOHANG);
    if (child != -1 && child != 0) {
        printf("%d ended ", child);
        if (!status) {
            puts("succesful");
        } else {
            puts("bad");
        }
    }
}

/*functions to change input and output */

ssize_t change_IO(char **list)
{
    ssize_t fd, io;
    int i = 0;
    char ch;
    while (list[i] != NULL) {
        if (!list[i][0]) {
            ch = list[i][1];
            if (ch == '>') {
                fd = open(list[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0755);
                io = STDOUT_FILENO;
            } else {
                fd = open(list[i + 1], O_RDONLY);
                io = STDIN_FILENO;
            }
            if (fd < 0) {
                free_list(list);
                err(1, NULL);
            }
            dup2(fd, io);
            free_string_in_list(i, list); // remove ">" or "<"
            free_string_in_list(i, list); // remove opened file name
            return fd;
        }
        i++;
    }
    return 0;
}

/* functions to change directory */

int change_dir(char **list)
{
    if (strcmp(list[0], "cd")) {
        return EXIT_FAILURE;
    }
    if (!list[1] || list[1][0] == '~') {
        uid_t uid = getuid();
        struct passwd *pw = getpwuid(uid);
        if (pw == NULL) {
            err(1, NULL);
        }
        if (chdir(pw -> pw_dir)) {
            perror("failed to change directory");
        }
    } else {
        if (chdir(list[1])) {
            perror("failed to chaneg directory");
        }
    }
    return EXIT_SUCCESS;
}

/* other function */

void new_line(void)
{
    char pc_name[HOST_NAME_MAX], login[LOGIN_NAME_MAX];
    char cwd[4096];
    if (getlogin_r(login, LOGIN_NAME_MAX)) {
        err(1, "failed to get username");
    }
    if (gethostname(pc_name, HOST_NAME_MAX)) {
        err(1, "failed to get host name");
    }
    if (!getcwd(cwd, sizeof(cwd))) {
        err(1, "failed to get path to current directory");
    }
    printf(PURPLE"%s@%s" RESET ":" BLUE "%s"RESET"$ ", login, pc_name, cwd);
}

/* function to execute commands */

void execute(int bg_proc, char **list)
{
    ssize_t fd, fd2;
    bg_proc_start(bg_proc, list);
    fd = change_IO(list); // check for special symbols like "<" or ">"
    fd2 = change_IO(list); // check for the second one
    if (execvp(list[0], list) < 0) {
        free_list(list);
        err(1, NULL);
    }
    if (fd != 0) {
        close(fd);
    }
    if (fd2 != 0) {
        close(fd2);
    }
}

int execcat(void)
{
    pid_t pid = -1;
    int i, cmds, bg_proc = 0;
    char ***catalog = NULL;
    while (1) {
        new_line();
        catalog = get_catalog(&cmds);
        if (!catalog) {
            err(1, "failed to create catalog");
        }
        if (catalog[0] != NULL && (!strcmp(catalog[0][0], "exit") || !strcmp(catalog[0][0], "quit"))) {
            free_catalog(catalog);
            return EXIT_SUCCESS;
        }
        i = 0;
        while (catalog[i] != NULL && change_dir(catalog[i])) {
            bg_proc = is_bg_proc(catalog[cmds]);
            if ((pid = fork()) < 0) {
                free_catalog(catalog);
                err(1, NULL);
            }
            if (!pid) { // child process
                execute(bg_proc, catalog[i]);
                return EXIT_SUCCESS; // close child process
            }
            if (!bg_proc) {
                wait(NULL); // waiting for child to end his process
            }
            i++;
        }
        free_catalog(catalog);
        bg_proc_wait(pid);
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
