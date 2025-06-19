#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <termios.h>
#include <ctype.h>

struct sigaction prije;
int procesi[1024] = {0};
char *naredbe[4096] = {0};
char *history[4096] = {0};
int ptprocesi = 0;
int ptnaredbe = 0;
int pthistory = 0;

int finder(int pid) {
    for (int i = 0; i < 1024; i++) {
        if (procesi[i] == pid) {
            return i;
        }
    }
    return -1;
}

char *join_argv(char *argv[]) {
    char temp[256] = "";
    for (int i = 0; argv[i] != NULL; i++) {
        strcat(temp, argv[i]);
        if (argv[i + 1] != NULL) {
            strcat(temp, " ");
        }
    }
    return strdup(temp);
}

pid_t pokreni_program(char *naredba[], int u_pozadini);
void obradi_dogadjaj(int sig);
void obradi_signal_zavrsio_neki_proces_dijete(int id);

int main() {
    struct sigaction act;
    pid_t pid_novi;
    int lchistory = 0;

    act.sa_handler = obradi_dogadjaj;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, &prije);

    act.sa_handler = obradi_signal_zavrsio_neki_proces_dijete;
    sigaction(SIGCHLD, &act, NULL);

    act.sa_handler = SIG_IGN;
    sigaction(SIGTTOU, &act, NULL);
    signal(SIGTSTP, SIG_IGN);

    struct termios shell_term_settings;
    tcgetattr(STDIN_FILENO, &shell_term_settings);
    tcsetpgrp(STDIN_FILENO, getpgid(0));

    size_t vel_buf = 128;
    char buffer[vel_buf];
    char path[1024];

    do {
        printf("\033[1;32m[roditelj]\033[0m:%s$ ", getcwd(path, sizeof(path)));
        fflush(stdout);

        if (fgets(buffer, vel_buf, stdin) != NULL) {
            char *argv[10];
            int argc = 0;
            int u_pozadini = 0;

            argv[argc] = strtok(buffer, " \t\n");
            while (argv[argc] != NULL) {
                if (strcmp(argv[argc], "&") == 0) {
                    u_pozadini = 1;
                    argv[argc] = NULL;
                    break;
                }
                argc++;
                argv[argc] = strtok(NULL, " \t\n");
            }

            if (argv[0] == NULL) continue;

            if (strcmp(argv[0], "exit") == 0) {
                printf("Shell zatvoren. Doviđenja 👋\n");
                exit(1);
            }

            if (strcmp(argv[0], "cd") == 0) {
                int argctmp = 1;
                char argvtmp[1024] = {0};
                while (argv[argctmp] != NULL) {
                    strcat(argvtmp, argv[argctmp]);
                    if (argv[argctmp + 1] != NULL) strcat(argvtmp, " ");
                    argctmp++;
                }
                if (chdir(argvtmp) != 0) {
                    fprintf(stderr, "Greška: Ne mogu otvoriti direktorij.\n");
                }
                lchistory = 0;
                history[pthistory++] = join_argv(argv);
                continue;
            }

            if (strcmp(argv[0], "ps") == 0) {
                printf("PID      IME\n");
                for (int i = 0; i < 1024; i++) {
                    if (procesi[i] != 0) {
                        printf("%-8d%-s\n", procesi[i], naredbe[i]);
                    }
                }
                lchistory = 0;
                history[pthistory++] = join_argv(argv);
                continue;
            }

            if (strcmp(argv[0], "kill") == 0 && argv[1] && argv[2]) {
                pid_t pid = atoi(argv[1]);
                int signal = atoi(argv[2]);
                int found = finder(pid);
                if (kill(pid, signal) == 0) {
                    printf("Poslan %d procesu %d\n", signal, pid);
                    if (signal == SIGINT || signal == SIGTERM) {
                        if (found != -1) {
                            procesi[found] = 0;
                            naredbe[found] = NULL;
                        }
                    }
                } else {
                    fprintf(stderr, "Greška: Ne mogu poslati signal procesu.\n");
                }
                lchistory = 0;
                history[pthistory++] = join_argv(argv);
                continue;
            }

            if (strcmp(argv[0], "history") == 0) {
                lchistory = 1;
                for (int i = 0; i < pthistory; i++) {
                    if (history[i] != NULL)
                        printf("%d\t%s\n", i + 1, history[i]);
                }
                continue;
            }

            if (argv[0][0] == '!' && isdigit(argv[0][1])) {
                int index = atoi(argv[0] + 1);
                if (index > 0 && index <= pthistory) {
                    char *prijenos[4096] = {0};
                    int c = 0;
                    char *history_entry = strdup(history[index - 1]);
                    prijenos[c] = strtok(history_entry, " \t\n");
                    while (prijenos[c] != NULL) {
                        if (strcmp(prijenos[c], "&") == 0) {
                            u_pozadini = 1;
                            prijenos[c] = NULL;
                            break;
                        }
                        c++;
                        prijenos[c] = strtok(NULL, " \t\n");
                    }

                    if (prijenos[0] != NULL) {
                        if (strcmp(prijenos[0], "cd") == 0) {
                            int argctmp = 1;
                            char argvtmp2[1024] = {0};
                            while (prijenos[argctmp] != NULL) {
                                strcat(argvtmp2, prijenos[argctmp]);
                                if (prijenos[argctmp + 1] != NULL) strcat(argvtmp2, " ");
                                argctmp++;
                            }
                            if (chdir(argvtmp2) != 0) {
                                fprintf(stderr, "Greška: Ne mogu otvoriti direktorij.\n");
                            }
                            free(history_entry);
                            continue;
                        }

                        if (strcmp(prijenos[0], "ps") == 0) {
                            printf("PID      IME\n");
                            for (int i = 0; i < 1024; i++) {
                                if (procesi[i] != 0)
                                    printf("%-8d%-s\n", procesi[i], naredbe[i]);
                            }
                            free(history_entry);
                            continue;
                        }

                        if (strcmp(prijenos[0], "kill") == 0 && prijenos[1] && prijenos[2]) {
                            pid_t pid = atoi(prijenos[1]);
                            int signal = atoi(prijenos[2]);
                            int found = finder(pid);
                            if (kill(pid, signal) == 0) {
                                printf("Poslan %d procesu %d\n", signal, pid);
                                if (signal == SIGINT || signal == SIGTERM) {
                                    if (found != -1) {
                                        procesi[found] = 0;
                                        naredbe[found] = NULL;
                                    }
                                }
                            } else {
                                fprintf(stderr, "Greška: Ne mogu poslati signal procesu.\n");
                            }
                            free(history_entry);
                            continue;
                        }

                        pid_novi = pokreni_program(prijenos, u_pozadini);
                        lchistory = 0;
                        procesi[ptprocesi++] = (int)pid_novi;
                        naredbe[ptnaredbe++] = strdup(history[index - 1]);
                        history[pthistory++] = join_argv(argv);

                        if (!u_pozadini) {
                            printf("[roditelj] čekam da završi\n");
                            pid_t pid_zavrsio;
                            do {
                                pid_zavrsio = waitpid(pid_novi, NULL, 0);
                                if (pid_zavrsio > 0 && kill(pid_novi, 0) == -1) {
                                    printf("[dijete %d] završilo s radom\n", pid_zavrsio);
                                    tcsetpgrp(STDIN_FILENO, getpgid(0));
                                    tcsetattr(STDIN_FILENO, 0, &shell_term_settings);
                                    int tmp = finder(pid_novi);
                                    if (tmp != -1) {
                                        procesi[tmp] = 0;
                                        naredbe[tmp] = NULL;
                                    }
                                }
                            } while (pid_zavrsio <= 0);
                        } else {
                            printf("[roditelj] pokrenuo pozadinski proces %d: %s\n", pid_novi, join_argv(argv));
                        }
                    } else {
                        printf("Ne važeća naredba.\n");
                    }
                    free(history_entry);
                } else {
                    printf("Izvan raspona naredbe history.\n");
                }
                continue;
            } else {
                pid_novi = pokreni_program(argv, u_pozadini);
                lchistory = 0;
                procesi[ptprocesi++] = (int)pid_novi;
                naredbe[ptnaredbe++] = join_argv(argv);
                history[pthistory++] = join_argv(argv);
            }

            if (!u_pozadini) {
                printf("[roditelj] čekam da završi\n");
                pid_t pid_zavrsio;
                do {
                    pid_zavrsio = waitpid(pid_novi, NULL, 0);
                    if (pid_zavrsio > 0 && kill(pid_novi, 0) == -1) {
                        printf("[dijete %d] završilo s radom\n", pid_zavrsio);
                        tcsetpgrp(STDIN_FILENO, getpgid(0));
                        tcsetattr(STDIN_FILENO, 0, &shell_term_settings);
                        int tmp = finder(pid_novi);
                        if (tmp != -1) {
                            procesi[tmp] = 0;
                            naredbe[tmp] = NULL;
                        }
                    }
                } while (pid_zavrsio <= 0);
            } else {
                printf("[roditelj] pokrenuo pozadinski proces %d: %s\n", pid_novi, join_argv(argv));
            }
        }
    } while (strncmp(buffer, "exit", 4) != 0);

    return 0;
}

void obradi_dogadjaj(int sig) {
    printf("\n[signal SIGINT] proces %d primio signal %d\n", (int)getpid(), sig);
}

void obradi_signal_zavrsio_neki_proces_dijete(int id) {
    pid_t pid_zavrsio;
    while ((pid_zavrsio = waitpid(-1, NULL, WNOHANG)) > 0) {
        printf("\n[dijete %d] završilo s radom\n", pid_zavrsio);
        int tmp = finder((int)pid_zavrsio);
        if (tmp != -1) {
            procesi[tmp] = 0;
            naredbe[tmp] = NULL;
        }
    }
}

pid_t pokreni_program(char *naredba[], int u_pozadini) {
    pid_t pid_novi;
    if ((pid_novi = fork()) == 0) {
        printf("[dijete %d] pokrenuto u %s\n", (int)getpid(), u_pozadini ? "pozadini" : "prvom planu");
        fflush(stdout);
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_IGN);
        setpgid(0, 0);
        if (!u_pozadini)
            tcsetpgrp(STDIN_FILENO, getpid());
        if (execvp(naredba[0], naredba) == -1) {
            fprintf(stderr, "Greška: naredba '%s' nije prepoznata.\n", naredba[0]);
            exit(1);
        }
    }
    return pid_novi;
}

// BACKUP