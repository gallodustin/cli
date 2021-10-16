#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

const char ERROR_MESSAGE[30] = "An error has occurred\n";

int main(int argc, char *argv[]) {
    int count = 0;
    while(1) {
        char str[128];
        count += 1;
        printf("mysh (%d)> ", count);
        fflush(stdout);
        fgets(str, 128, stdin);
        char *token;
        char *rest = str;
        token = strtok_r(rest, " \n\t", &rest);
        if(token != NULL) {
            if(strcmp(token, "pwd") == 0) {
                char buf[128];
                printf("%s\n", getcwd(buf, sizeof(char) * 128));
            } else if(strcmp(token, "cd") == 0) {
                token = strtok_r(rest, " \n\t", &rest);
                if(token == NULL) {
                    chdir(getenv("HOME"));
                } else {
                    chdir(token);
                }
            } else if(strcmp(token, "exit") == 0) {
                exit(0);
            } else {
                int rc = fork();
                if(rc == 0) {
                    char *myargs[128];
                    int index = 1;
                    myargs[0] = strdup(token);
                    while((token = strtok_r(rest, " \n\t", &rest))
                          && (strcmp(token, "<") != 0)
                          && (strcmp(token, ">") != 0)) {
                        myargs[index] = strdup(token);
                        index += 1;
                    }
                    if(token != NULL) {
                        if(strcmp(token, ">") == 0) {
                            token = strtok_r(rest, " \n\t", &rest);
                            int fd = open(token, O_WRONLY | O_CREAT | O_TRUNC);
                            if(fd == -1) {
                                write(STDERR_FILENO, ERROR_MESSAGE, strlen(ERROR_MESSAGE));
                            }
                            dup2(fd, 1);
                        }
                        if(strcmp(token, "<") == 0) {
                            token = strtok_r(rest, " \n\t", &rest);
                            int fd = open(token, O_RDONLY);
                            if(fd == -1) {
                                write(STDERR_FILENO, ERROR_MESSAGE, strlen(ERROR_MESSAGE));
                            }
                            dup2(fd, 0);
                        }
                    }
                    myargs[index] = NULL;
                    execvp(myargs[0], myargs);
                } else {
                    waitpid(rc, NULL, 0);
                }
            }
        }
    }
}
