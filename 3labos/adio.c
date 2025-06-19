#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>

int Stol1 = 0;
int Stol2 = 0;
sem_t stolPraz;
sem_t SunkaSir;
sem_t SirKruh;
sem_t KruhSunka;

void inicijalizacijaSemafora()
{
    sem_init(&stolPraz, 0, 1);
    sem_init(&SunkaSir, 0, 0);
    sem_init(&SirKruh, 0, 0);
    sem_init(&KruhSunka, 0, 0);
}

void randomOdabir()
{
    Stol1 = rand() % 3;
    Stol2 = rand() % 3;
    while (Stol1 == Stol2)
    {
        Stol2 = rand() % 3;
    }
}

void *KupacKruh(void *arg)
{
    while (1)
    {
        sem_wait(&SunkaSir);
        Stol1 = 0;
        Stol2 = 0;
        printf("Kupac Sendviča 1: uzima sastojke, izlazi iz trgovine, slaže si sendvič i jede\n");
        sem_post(&stolPraz);
    }
}

void *KupacSir(void *arg)
{
    while (1)
    {
        sem_wait(&KruhSunka);
        Stol1 = 0;
        Stol2 = 0;
        printf("Kupac Sendviča 2: uzima sastojke, izlazi iz trgovine, slaže si sendvič i jede\n");
        sem_post(&stolPraz);
    }
}

void *KupacSunka(void *arg)
{
    while (1)
    {
        sem_wait(&SirKruh);
        Stol1 = 0;
        Stol2 = 0;
        printf("Kupac Sendviča 3: uzima sastojke, izlazi iz trgovine, slaže si sendvič i jede\n");
        sem_post(&stolPraz);
    }
}

void *Trgovac(void *arg)
{
    while (1)
    {
        sem_wait(&stolPraz);
        randomOdabir();
        printf("\n");
        if ((Stol1 == 0 && Stol2 == 1) || (Stol1 == 1 && Stol2 == 0))
        {
            printf("Trgovac: Kruh i sir\n");
            sem_post(&SirKruh);
        }
        else if ((Stol1 == 0 && Stol2 == 2) || (Stol1 == 2 && Stol2 == 0))
        {
            printf("Trgovac: Kruh i sunka\n");
            sem_post(&KruhSunka);
        }
        else if ((Stol1 == 1 && Stol2 == 2) || (Stol1 == 2 && Stol2 == 1))
        {
            printf("Trgovac: Sir i sunka\n");
            sem_post(&SunkaSir);
        }
        sleep(1);
    }
}

int main()
{
    inicijalizacijaSemafora();
    pthread_t t_trgovac, t_sunka, t_kruh, t_sir;

    printf("Kupac Sendviča 1: ima kruh\n");
    printf("Kupac Sendviča 2: ima sir\n");
    printf("Kupac Sendviča 3: ima sunku\n");

    pthread_create(&t_trgovac, NULL, Trgovac, NULL);
    pthread_create(&t_kruh, NULL, KupacKruh, NULL);
    pthread_create(&t_sir, NULL, KupacSir, NULL);
    pthread_create(&t_sunka, NULL, KupacSunka, NULL);

    pthread_join(t_trgovac, NULL);
    return 0;
}