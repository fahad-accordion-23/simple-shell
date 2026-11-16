#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdbool.h>

#define FSH_READ_LINE_BUFFER_SIZE 1024
#define FSH_TOKEN_BUFFER_SIZE 64
#define FSH_TOKEN_DELIMTER " \t\r\n\a"
#define FSH_COUNT_BUILT_IN 3

/*
 * TODO LIST:
 *   - add more built-ins
 *   - implement quoting and backslash escaping
 *   - allow piping
 *   - allow globbing
 */

/* -- FUNCTION DECLARATIONS ------------------------------------------------- */

void fsh_loop(void);

char* fsh_read_line(void);
char** fsh_split_line(char *line);
int fsh_execute(char **arguments);

int fsh_launch(char **arguments);

/*
 * Built-in shell commands:
 */
int fsh_cd(char **arguments);
int fsh_help(/*char **arguments*/);
int fsh_exit(/*char **arguments*/);

/* -- GLOBAL VARIABLES ------------------------------------------------------ */

/*
 * List of built-in commands:
 */
char *command_strings[] =
{
    "cd",
    "help",
    "exit"
};

/*
 * Array of built-in command-functions:
 */
int (* command_functions[]) (char**) = 
{
    &fsh_cd,
    &fsh_help,
    &fsh_exit
};

/* -- MAIN ------------------------------------------------------------------ */

int main(void)
{
    fsh_loop();

    return EXIT_SUCCESS;
}

/* -- FUNCTION DEFINITIONS -------------------------------------------------- */

void fsh_loop(void)
{
    char *line;
    char **arguments;
    int status;

    do
    {
        printf("$ ");

        line = fsh_read_line();
        arguments = fsh_split_line(line);
        status = fsh_execute(arguments);

        free(line);
        free(arguments);
    } while (status);
}

char* fsh_read_line(void)
{
    size_t buffer_size = FSH_READ_LINE_BUFFER_SIZE;
    size_t position = 0;
    char *buffer = malloc(sizeof(char) * buffer_size);
    char *buffer_realloc;
    int ch;

    while(true)
    {
        ch = getchar();

        if (ch == EOF || ch == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        }
        else
        {
            buffer[position] = ch;
        }
        position++;

        if (position >= buffer_size)
        {
            buffer_size += FSH_READ_LINE_BUFFER_SIZE;
            buffer_realloc = realloc(buffer, buffer_size);

            if (buffer_realloc == NULL)
            {
                free(buffer);
                fprintf(stderr, "fsh: allocation error\n");
                exit(EXIT_FAILURE);
            }

            buffer = buffer_realloc;
        }
    }
}

char** fsh_split_line(char *line)
{
    size_t buffer_size = FSH_TOKEN_BUFFER_SIZE;
    size_t position = 0;
    char **tokens = malloc(sizeof(char*) * buffer_size);
    char **tokens_realloc;
    char *token;

    if (tokens == NULL)
    {
        fprintf(stderr, "fsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, FSH_TOKEN_DELIMTER);

    while (token != NULL)
    {
        tokens[position] = token;
        position++;

        if (position >= buffer_size)
        {
            buffer_size += FSH_TOKEN_BUFFER_SIZE;
            tokens_realloc = realloc(tokens, sizeof(char*) * buffer_size);

            if (tokens_realloc == NULL)
            {
                free(tokens);
                fprintf(stderr, "fsh: allocation error\n");
                exit(EXIT_FAILURE);
            }

            tokens = tokens_realloc;
        }

        token = strtok(NULL, FSH_TOKEN_DELIMTER);
    }

    tokens[position] = NULL;
    return tokens;
}

int fsh_execute(char **arguments)
{
    if (arguments[0] == NULL)
    {
        return 1;
    }

    for (int i = 0; i < FSH_COUNT_BUILT_IN; i++)
    {
        if (strcmp(arguments[0], command_strings[i]) == 0)
        {
            return (*command_functions[i])(arguments);
        }
    }

    return fsh_launch(arguments);
}

int fsh_launch(char **arguments)
{
    pid_t process_id; /*, wait_process_id;*/
    int status;

    process_id = fork();
    if (process_id == 0)
    {
        if (execvp(arguments[0], arguments) == -1)
        {
            perror("fsh");
        }
        
        exit(EXIT_FAILURE);
    }
    else if (process_id < 0)
    {
        perror("fsh");
    }
    else
    {
        do 
        {
            /*wait_process_id = */waitpid(process_id, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int fsh_cd(char **arguments)
{
    if (arguments[1] == NULL)
    {
        fprintf(stderr, "fsh: expected argument to \"cd\"\n");
    }
    else
    {
        if (chdir(arguments[1]) != 0)
        {
            perror("fsh");
        }
    }

    return 1;
}

int fsh_help(/*char **arguments*/)
{
    printf("Fahad Hassan's FSH; base implementation totally copied from Stephen Brennan's LSH.\n");
    printf("Checkout his repository at https://github.com/brenns10/lsh for his code and a link to his tutorial.\n");
    printf("The following are built-in:\n");
    
    for (int i = 0; i < FSH_COUNT_BUILT_IN; i++)
    {
        printf("    %s\n", command_strings[i]);
    }

    printf("Use the man command for information on other programs.\n");

    return 1;
}

int fsh_exit(/*char **arguments*/)
{
    return 0;
}
