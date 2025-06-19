#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <signal.h>
#include <string.h>

#define ANSI_COLOR_ORANGE "\x1b[38;5;214m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m" // Boje za tekst
#define ANSI_COLOR_RESET   "\x1b[0m"

int maxIndex(int lista[]){ // Traži najveću podignutu zastavicu
    for(int i=4; i >= 0; i--){
        if(lista[i] == 1){
            return i;
        }
    }
    return -1;
}

void printer(int lista[], int stanje, int stog[]){ // Ispisuje stanja stoga, T_P i K_Z
    printf("\n=============== PRIKAZ STANJA ===============\n");
    printf("T_P: " ANSI_COLOR_YELLOW "%d" ANSI_COLOR_RESET "\n", stanje);
    printf("K_Z: [" ANSI_COLOR_GREEN "%d, %d, %d, %d, %d" ANSI_COLOR_RESET "]\n", lista[0], lista[1], lista[2], lista[3], lista[4]);
    printf("STOG: [" ANSI_COLOR_GREEN "%d, %d, %d, %d, %d, %d" ANSI_COLOR_RESET "]\n", stog[0], stog[1], stog[2], stog[3], stog[4], stog[5]);
    printf("=============================================\n\n");
}

void unos_prekid(int sig);
void obradi_sigterm(int sig);
void obradi_sigint(int sig);
void obrada_prekida(int T_P);
void selektor();

int nije_kraj = 1;
int K_Z[5] = {0};
int stog[6] = {0};
int SP = 0;
int T_P = 0;
int temp = 0;

int main()
{
    struct sigaction act;

    act.sa_handler = unos_prekid; /* kojom se funkcijom signal obrađuje SIGUSR1*/
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGTERM);
    act.sa_flags = SA_NODEFER;   
    sigaction(SIGUSR1, &act, NULL);

    act.sa_handler = obradi_sigterm; /* kojom se funkcijom signal obrađuje SIGTERM*/
    sigemptyset(&act.sa_mask);
    sigaction(SIGTERM, &act, NULL);

    act.sa_handler = obradi_sigint; /* kojom se funkcijom signal obrađuje SIGINT*/
    sigaction(SIGINT, &act, NULL);

    printf("Program s PID=%ld krenuo s radom\n", (long)getpid());
    int i = 1;

    while (nije_kraj) // Glavni program
    {
        printf("Program: iteracija %d\n", i++);
        T_P = 0;
        K_Z[0] = 0; K_Z[1] = 0; K_Z[2] = 0; K_Z[3] = 0; K_Z[4] = 0;
        sleep(1);
    }

    printf("Program s PID=%ld zavrsio s radom\n", (long)getpid());
    return 0;
}

void unos_prekid(int sig) // Unos prioriteta
{
    printf("-------------------------------------\n");
    printf(ANSI_COLOR_RED   "Unesi prekid (1 - 5); "     ANSI_COLOR_RESET);
    scanf("%d", &temp);

    if(temp > 5 || temp < 0){ // Provjera ispravnosti unosa
        printf("Neispravan unos\n");
        return;
    }

    printf("-------------------------------------\n");
    selektor();
}

void obradi_sigterm(int sig) // Obrada SIGTERM
{
    printf("Primio signal SIGTERM\n");
    nije_kraj = 0;
}

void obradi_sigint(int sig) // Obrada SIGINT
{
    printf("Primio signal SIGINT\n");
    exit(1);
}

void obrada_prekida(int T_P) // Ispis obrade prekida
{
    printf("\n");

    for(int i = 1; i <= 5; i++){
        printf(ANSI_COLOR_ORANGE     "Obrada signala %d, %d/5!"     ANSI_COLOR_RESET "\n", T_P, i);
        sleep(1.5);
    }

    printf("Kraj obrade signala %d\n", T_P);
}

void selektor() // Obrada SIGUSR1
{ 
    int tmp[5];
    int tmp2[6];

    if(temp > T_P){
        stog[SP] = T_P;
        SP++;
        T_P = temp;
        printer(K_Z, T_P, stog);
        obrada_prekida(T_P);
        SP--;
        T_P = stog[SP];
        if(maxIndex(K_Z) + 1 > stog[SP]){
            memcpy(tmp, K_Z, sizeof(K_Z));
            tmp[maxIndex(K_Z)] = 0;
            printer(tmp, maxIndex(K_Z) + 1, stog);
        }else{
            stog[SP] = 0;
            printer(K_Z, T_P, stog);
            return;
        }
        while(maxIndex(K_Z) + 1 > stog[SP]){
            T_P = maxIndex(K_Z) + 1;
            obrada_prekida(maxIndex(K_Z) + 1);
            K_Z[maxIndex(K_Z)] = 0;
            if(maxIndex(K_Z) + 1 > stog[SP]){
                memcpy(tmp, K_Z, sizeof(K_Z));
                tmp[maxIndex(K_Z)] = 0;
                printer(tmp, maxIndex(K_Z) + 1, stog);
            }else{
                T_P = stog[SP];
                stog[SP] = 0;
                printer(K_Z, T_P, stog);
            }
        }
        stog[SP] = 0;
    }
    else if(temp < T_P && T_P == maxIndex(K_Z) + 1){
        memcpy(tmp, K_Z, sizeof(K_Z));
        tmp[maxIndex(K_Z)] = 0;
        printer(tmp, maxIndex(K_Z) + 1, stog);
    }
    else if(temp < T_P){
        K_Z[temp - 1] = 1;
        printer(K_Z, T_P, stog);
    }
}