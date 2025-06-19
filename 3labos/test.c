#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>

int Stol1 = 0;
int Stol2 = 0;

// Named semaphores
sem_t *stolPraz;
sem_t *SunkaSir;
sem_t *SirKruh;
sem_t *KruhSunka;

void inicijalizacijaSemafora() {
    stolPraz = sem_open("/stolPraz", O_CREAT, 0644, 1);
    SunkaSir = sem_open("/SunkaSir", O_CREAT, 0644, 0);
    SirKruh = sem_open("/SirKruh", O_CREAT, 0644, 0);
    KruhSunka = sem_open("/KruhSunka", O_CREAT, 0644, 0);
}

void obrisiSemafore() {
    sem_unlink("/stolPraz");
    sem_unlink("/SunkaSir");
    sem_unlink("/SirKruh");
    sem_unlink("/KruhSunka");
}

void randomOdabir() {
    Stol1 = rand() % 3;
    Stol2 = rand() % 3;
    while (Stol1 == Stol2) {
        Stol2 = rand() % 3;
    }
}

void kupacKruh() {
    while (1) {
        sem_wait(SunkaSir);
        Stol1 = 0;
        Stol2 = 0;
        printf("Kupac Sendviča 1: uzima sastojke, izlazi iz trgovine, slaže si sendvič i jede\n");
        sem_post(stolPraz);
    }
}

void kupacSir() {
    while (1) {
        sem_wait(KruhSunka);
        Stol1 = 0;
        Stol2 = 0;
        printf("Kupac Sendviča 2: uzima sastojke, izlazi iz trgovine, slaže si sendvič i jede\n");
        sem_post(stolPraz);
    }
}

void kupacSunka() {
    while (1) {
        sem_wait(SirKruh);
        Stol1 = 0;
        Stol2 = 0;
        printf("Kupac Sendviča 3: uzima sastojke, izlazi iz trgovine, slaže si sendvič i jede\n");
        sem_post(stolPraz);
    }
}

void trgovac() {
    while (1) {
        sem_wait(stolPraz);
        randomOdabir();
        printf("\n");
        if ((Stol1 == 0 && Stol2 == 1) || (Stol1 == 1 && Stol2 == 0)) {
            printf("Trgovac: Kruh i sir\n");
            sem_post(SirKruh);
        } else if ((Stol1 == 0 && Stol2 == 2) || (Stol1 == 2 && Stol2 == 0)) {
            printf("Trgovac: Kruh i sunka\n");
            sem_post(KruhSunka);
        } else if ((Stol1 == 1 && Stol2 == 2) || (Stol1 == 2 && Stol2 == 1)) {
            printf("Trgovac: Sir i sunka\n");
            sem_post(SunkaSir);
        }
        sleep(1);
    }
}

int main() {
    srand(time(NULL));
    inicijalizacijaSemafora();

    printf("Kupac Sendviča 1: ima kruh\n");
    printf("Kupac Sendviča 2: ima sir\n");
    printf("Kupac Sendviča 3: ima sunku\n");

    if (fork()== 0) {
        kupacKruh();
        exit(0);
    }

    if (fork() == 0) {
        kupacSir();
        exit(0);
    }

    if (fork() == 0) {
        kupacSunka();
        exit(0);
    }

    if (fork() == 0) {
        trgovac();
        exit(0);
    }

    for (int i = 0; i < 4; ++i) {
        wait(NULL);
    }

    return 0;
}
