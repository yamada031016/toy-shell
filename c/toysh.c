#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define toysh_TOK_BUFSIZE 64
#define toysh_TOK_DELIM " \t\r\n\a"
#define toysh_RL_BUFSIZE 1024

char *toysh_read_line(void)
{
  char *line = NULL;
  ssize_t bufsize = 0; // have getline allocate a buffer for us

  if (getline(&line, &bufsize, stdin) == -1){
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);  // We recieved an EOF
    } else  {
      perror("readline");
      exit(EXIT_FAILURE);
    }
  }

  return line;
}

char **toysh_split_line(char *line)
{
  int bufsize = toysh_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "toysh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, toysh_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += toysh_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "toysh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, toysh_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

int toysh_launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("toysh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("toysh");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

/*
  Function Declarations for builtin shell commands:
 */
int toysh_cd(char **args);
int toysh_help(char **args);
int toysh_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &toysh_cd,
  &toysh_help,
  &toysh_exit
};

int toysh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/
int toysh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "toysh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("toysh");
    }
  }
  return 1;
}

int toysh_help(char **args)
{
  int i;
  printf("Stephen Brennan's toysh\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < toysh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int toysh_exit(char **args)
{
  return 0;
}

int toysh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < toysh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return toysh_launch(args);
}

void toysh_loop() {
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line = toysh_read_line();
        args = toysh_split_line(line);
        status =  toysh_execute(args);

        free(line);
        free(args);
    } while(status);
}

int main() {
    // Load config files, if any.

  // Run command loop.
  toysh_loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}

