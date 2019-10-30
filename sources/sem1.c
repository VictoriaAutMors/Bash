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
#include <signal.h>

#define TRUE 1
#define FALSE 0

#define PURPLE "\033[38;5;141m"
#define BLUE "\033[38;5;57m "
#define RESET "\033[0m"

typedef struct node *Link;

struct node
{
    int num;
    pid_t pid;
    char *name;
    Link next;
};

pid_t pid = -1;

Link proc_roster = NULL;

/* functions to free heap after program execution*/

void free_list(char **list)
{
    int i = 0;
    while (list[i] != NULL)
    {
        free(list[i]);
        i++;
    }
    free(list[i]);
    free(list);
}

void free_roster(Link roster)
{
    Link tmp;
    while (roster != NULL)
    {
        tmp = roster;
        roster = roster -> next;
        free(tmp -> name);
        free (tmp);
    }
}

void free_catalog(char ***catalog)
{
    int i = 0;
    while (catalog[i] != NULL)
    {
        free_list(catalog[i]);
        i++;
    }
    free(catalog[i]);
    free(catalog);
}

/* functions to delete elements in list*/

void del_string_in_list(int num, char **list)
{
    free(list[num]);
    while (list[num + 1] != NULL)  // pushing i's string to the end of the list
    {
        list[num] = list[num + 1];
        num++;
    }
    list[num] = NULL;    // set new end of the list
    free(list[num + 1]); // free previous end of the list
}

Link pop_front(Link ptr)
{
    Link roster = ptr -> next;
    free(ptr -> name);
    free(ptr);
    return roster;
}

Link pop(Link roster, pid_t pid) {
    if (roster -> next == NULL)
    {
        return pop_front(roster);
    }
    Link ptr = roster;
    while (ptr -> next -> next != NULL)
    {
        if (ptr -> next -> pid == pid)
        {
            ptr -> next = pop_front(ptr -> next);
            return roster;
        }
        ptr = ptr -> next;
    }
    ptr -> next = pop_front(ptr -> next);
    return roster;
}

/* functions to print */

void write_int(int num)
{
    if (num == 0)
    {
        return;
    }
    write_int(num / 10);
    char ch = num % 10 + '0';
    if (write(STDOUT_FILENO, &ch, 1) < 0)
    {
        err(1, NULL);
        return;
    }
}

void print(Link roster) {
    putchar('[');
    for (Link t = roster; t != NULL; t = t -> next) {
        printf("%d ", t -> num);
    }
    putchar(']');
    putchar('\n');
}

void write_out(char *string)
{
    if (!write(STDOUT_FILENO, string, strlen(string)))
    {
        err(1, NULL);
    }
}

/* functions to fill with roster */

Link push_front(Link roster, int num, pid_t pid, char *name) {
    Link ptr = (Link)malloc(sizeof(struct node));
    ptr -> num = num;
    ptr -> pid = pid;
    ptr -> name = name;
    ptr -> next = roster;
    return ptr;
}

Link push_back(Link roster, int num, pid_t pid, char *name) {
    if (roster == NULL)
    {
        return push_front(roster, num, pid, name);
    }
    Link ptr = roster;
    while (ptr -> next != NULL)
    {
        ptr = ptr -> next;
    }
    ptr -> next = push_front(NULL, num, pid, name);
    return roster;
}

void fill_roster(char *name, int bg_proc_flag, int num, int pid)
{
    if (!bg_proc_flag)
    {
        num = 0;
    }
    int len = strlen(name);
    char *tmp = (char *)malloc((len + 1) * sizeof(char));
    tmp = strcpy(tmp, name);
    if (tmp == NULL)
    {
        err(1, "failed to copy string");
    }
    proc_roster = push_back(proc_roster, num, pid, tmp);
}

Link find(Link roster, pid_t pid)
{
    if (roster == NULL)
    {
        return roster;
    }
    Link ptr = roster;
    while (ptr && ptr -> pid != pid)
    {
        ptr = ptr -> next;
    }
    return ptr;
}

Link del_proc_fm_roster(Link roster, pid_t pid, int status)
{
    Link ptr = find(roster, pid);
    if (!ptr)
    {
        return NULL;
    }
    write_out("[");
    write_int(ptr -> num);
    write_out("]+   ");
    write_out(ptr -> name);
    if (!status)
    {
        write_out("     Done\n");
    } else if (WIFSIGNALED(status)) {
        psignal(WTERMSIG(status), NULL);
    } else {
        write_out("     Ended Bad\n");
    }
    roster = pop(roster, pid);
    return roster;
}

// functions to work in special cases
// like (quotes, change IO symbols, pipes etc)
// when filling word 

char *get_quote(char mark)
{
    if (mark != '"')
    {
        return NULL;
    }
    int i = 0;
    char ch, *word = NULL;
    do
    {
        ch = getchar();
        word = (char *)realloc(word, (i + 1) * sizeof(char));
        if (word == NULL)
        {
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
    while (ch == ' ' || ch == '\t')
    {
        ch = getchar(); // get symbols until it not a letter or '\n'
    }
    return ch;
}

char *separate_io_word(char ch, char *end)
{
    if (ch == '>' || ch == '<')
    {
        char *word = (char *)calloc(3, sizeof(char));
        if (word == NULL)
        {
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
    if (word)
    {
        return word;
    }
    word = separate_io_word(ch, end);
    return word;
}

/*  functions to fill arrays*/

char *get_word(char *end)
{
    if (*end == '\n' || *end == '|') // no more lexemes
    {
        return NULL;
    }
    int i = 0;
    char ch, *word = NULL;
    do
    {
        ch = getchar();
        if (!i)
        {
            ch = get_first_letter(ch);
            if (ch == '\n' || ch == '|')
            {
                *end = ch;
                return NULL;
            }
            word = word_special_case(ch, end);
            if (word)
            {
                return word;
            }
        }
        word = (char *)realloc(word, (i + 1) * sizeof(char));
        if (word == NULL)
        {
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
    if (*end_of_line == '\n')
    {
        return NULL;
    }
    char end = 0, **list = NULL, **check = NULL;
    int i = 0;
    do
    {
        check = (char **)realloc(list, (i + 1) * sizeof(char *));
        if (check == NULL)
        {
            free_list(list);
            err(1, NULL);
        }
        list = check;
        list[i] = get_word(&end);
        if (!list[0])
        {
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
    do
    {
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
    while (list[i] != NULL)
    {
        if (list[i][0] == '&' && list[i][1] == '\0')
        {
            return i;
        }
        i++;
    }
    return FALSE;
}

void bg_proc_start(char **list, int bg_proc_flag, int bg_proc_count)
{
    if (bg_proc_flag)
    {
        int pid = getpid();
        write_out("[");
        write_int(bg_proc_count);
        write_out("]    ");
        write_int(pid);
        write_out("\n");
        del_string_in_list(bg_proc_flag, list);
    }
}

void bg_proc_check(int *count, int bg_proc_flag)
{
    pid_t status, child;
    if (!bg_proc_flag)
    {
        child = waitpid(-1, &status, 0);
        if (child != -1 && !WIFSIGNALED(status))
        {
            proc_roster = pop(proc_roster, child);
        }
        return;
    } else {
        child = waitpid(-1, &status, WNOHANG);
    }
    if (child != -1 && child != 0)
    {
        proc_roster = del_proc_fm_roster(proc_roster, child, status);
        *count -= 1;
    }
}

void bg_proc_wait(int count)
{
    for (int i = 0; i < count; i++)
    {
        wait(NULL);
    }
}

/*functions to change input and output */

ssize_t change_IO(char **list)
{
    ssize_t fd, io;
    int i = 0;
    char ch;
    while (list[i] != NULL)
    {
        if (!list[i][0])
        {
            ch = list[i][1];
            if (ch == '>')
            {
                fd = open(list[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0755);
                io = STDOUT_FILENO;
            } else {
                fd = open(list[i + 1], O_RDONLY);
                io = STDIN_FILENO;
            }
            if (fd < 0)
            {
                free_list(list);
                err(1, NULL);
            }
            dup2(fd, io);
            del_string_in_list(i, list); // remove ">" or "<"
            del_string_in_list(i, list); // remove opened file name
            return fd;
        }
        i++;
    }
    return 0;
}

/* functions to change directory */

int is_change_dir(char **list)
{
    if (strcmp(list[0], "cd"))
    {
        return FALSE;
    }
    if (!list[1] || list[1][0] == '~') // change directory to home directory
    {
        uid_t uid = getuid();
        struct passwd *pw = getpwuid(uid);
        if (pw == NULL)
        {
            err(1, NULL);
        }
        if (chdir(pw->pw_dir))
        {
            perror("failed to change directory");
        }
    } else {
        if (chdir(list[1]))
        {
            perror("failed to change directory");
        }
    }
    return TRUE;
}

/* functions that work with pipes */

void pipe_close(int (*pipd)[2], int num)
{
    if (close(pipd[num][0]) < 0)
    {
        err(1, "failed to close pipe %d", num);
    }
    if (close(pipd[num][1]) < 0)
    {
        err(1, "failed to close pipe %d", num);
    }
}

void pipe_initialize(int (*pipd)[2], int cmds, int i)
{
    if (cmds == 0)
    { // no pipes in our command
        return;
    }
    if (i)
    {
        dup2(pipd[i - 1][0], 0);
    }
    if (i != cmds)
    {
        dup2(pipd[i][1], 1);
    }
}

/* logical or functions */



/* other functions */

void new_line(void)
{
    char pc_name[_POSIX_HOST_NAME_MAX], login[_POSIX_LOGIN_NAME_MAX];
    char cwd[_POSIX_ARG_MAX];
    if (getlogin_r(login, _POSIX_LOGIN_NAME_MAX))
    {
        err(1, "failed to get username");
    }
    if (gethostname(pc_name, _POSIX_HOST_NAME_MAX))
    {
        err(1, "failed to get host name");
    }
    if (!getcwd(cwd, sizeof(cwd)))
    {
        err(1, "failed to get path to current directory");
    }
    printf(PURPLE "%s@%s" RESET ":" BLUE "%s" RESET "$ ", login, pc_name, cwd);
}

/* function to execute commands */

int is_endline(char ***catalog, int bg_proc_count)
{
    if (!catalog[0])
    { // line consist only '\n'
        return FALSE;
    }
    if ((!strcmp(catalog[0][0], "exit") || !strcmp(catalog[0][0], "quit")))
    {
        bg_proc_wait(bg_proc_count);
        free_catalog(catalog);
        return TRUE;
    }
    return FALSE;
}

void execute(char **list, int bg_proc_flag, int bg_proc_count)
{
    ssize_t fd, fd2;
    bg_proc_start(list, bg_proc_flag, bg_proc_count);
    fd = change_IO(list);  // check for special symbols like "<" or ">"
    fd2 = change_IO(list); // check for the second one
    if (execvp(list[0], list) < 0)
    {
        free_list(list);
        err(1, NULL);
    }
    if (fd != 0)
    {
        close(fd);
    }
    if (fd2 != 0)
    {
        close(fd2);
    }
}

int execcat(void)
{
    int i, cmds, bg_proc_flag = 0, bg_proc_count = 0;
    char ***catalog = NULL;
    while (1)
    {
        new_line();
        catalog = get_catalog(&cmds);
        if (!catalog)
        {
            err(1, "failed to create catalog");
        }
        if (is_endline(catalog, bg_proc_count))
        {
            return EXIT_SUCCESS;
        }
        int pipd[cmds + 2][2];
        i = 0;
        while (catalog[i] != NULL && is_change_dir(catalog[i]) != TRUE)
        {
            bg_proc_flag = is_bg_proc(catalog[cmds]);
            bg_proc_count += bg_proc_flag;
            if (i != cmds && pipe(pipd[i]) < 0)
            {
                err(1, "failed to create pipe");
            }
            if ((pid = fork()) < 0)
            {
                free_catalog(catalog);
                err(1, NULL);
            }
            if (!pid)
            { // child process
                pipe_initialize(pipd, cmds, i);
                for (int j = 0; j != cmds && j < i; j++)
                {
                    pipe_close(pipd, j);
                }
                execute(catalog[i], bg_proc_flag, bg_proc_count);
                return EXIT_SUCCESS; // close child process
            }
            fill_roster(catalog[i][0], bg_proc_flag, bg_proc_count, pid);
            i++;
        }
        print(proc_roster);
        for (int j = 0; j <= i; j++)
        {
            if (j < cmds)
            {
                pipe_close(pipd, j);
            }
            bg_proc_check(&bg_proc_count, bg_proc_flag);
        }
        free_catalog(catalog);
    }
    return EXIT_FAILURE;
}

void handler(void)
{
    Link ptr = proc_roster;
    while (ptr)
    {
        if (ptr -> num == 0)
        {
            kill(ptr -> pid, SIGKILL);
            proc_roster = pop(proc_roster, ptr -> pid);
            ptr = proc_roster;
        } else {
            ptr = ptr -> next;
        }
    }
}

int main(void)
{
    signal(SIGINT, (void (*)(int))handler);
    execcat();
    free_roster(proc_roster);
    return EXIT_SUCCESS;
}
