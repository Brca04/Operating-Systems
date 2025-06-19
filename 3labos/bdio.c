#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>

int n = 0;
int aktivniStudenti = 0;
int pointer = 0;
int idTracker = 1;
int StudentiUSobi[4096] = {0};
pthread_mutex_t kljuc;
sem_t partiBrejker;

int brojStudenataUSobi()
{
    int count = 0;
    for (int i = 0; i < 4096; i++)
    {
        if (StudentiUSobi[i] != 0)
        {
            count++;
        }
    }
    return count;
}

int finder(int id)
{
    for (int i = 0; i < 4096; i++)
    {
        if (StudentiUSobi[i] == id)
        {
            return i;
        }
    }
    return -1;
}

void *Student(void *arg)
{
    pthread_mutex_lock(&kljuc);
    int id = idTracker;
    idTracker++;
    pthread_mutex_unlock(&kljuc);

    usleep((rand() % 401 + 100) * 1000);

    for (int i = 0; i < 3; i++)
    {
        sem_wait(&partiBrejker);
        pthread_mutex_lock(&kljuc);
        StudentiUSobi[pointer] = id;
        if (pointer == 4095)
        {
            pointer = 0;
        }
        pointer++;
        printf("Student %d je usao u sobu\n", id);
        pthread_mutex_unlock(&kljuc);
        sem_post(&partiBrejker);

        usleep((rand() % 1001 + 1000) * 1000);

        pthread_mutex_lock(&kljuc);
        if (StudentiUSobi[finder(id)] != 0)
        {
            StudentiUSobi[finder(id)] = 0;
            printf("Student %d je izasao iz sobe\n", id);
        }
        pthread_mutex_unlock(&kljuc);

        usleep((rand() % 1001 + 1000) * 1000);
    }
    pthread_mutex_lock(&kljuc);
    aktivniStudenti--;
    pthread_mutex_unlock(&kljuc);
}

void *PartiBrejker(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&kljuc);
        if (aktivniStudenti <= 0)
        {
            pthread_mutex_unlock(&kljuc);
            break;
        }
        pthread_mutex_unlock(&kljuc);

        usleep((rand() % 901 + 100) * 1000);

        pthread_mutex_lock(&kljuc);
        int broj = brojStudenataUSobi();
        pthread_mutex_unlock(&kljuc);

        if (broj > 3)
        {
            sem_wait(&partiBrejker);
            pthread_mutex_lock(&kljuc);
            printf("Partibrejker je usao u sobu\n");
            for (int i = 0; i < 4096; i++)
            {
                int id = StudentiUSobi[i];
                if (id != 0)
                {
                    printf("Student %d je izasao iz sobe\n", id);
                    StudentiUSobi[finder(id)] = 0;
                }
            }
            printf("Partibrejker je izasao iz sobe\n");
            pthread_mutex_unlock(&kljuc);
            sem_post(&partiBrejker);
        }
    }
}

int main()
{
    pthread_t partibrejker;
    sem_init(&partiBrejker, 0, 1);
    pthread_mutex_init(&kljuc, NULL);

    printf("Unesite broj studenata: ");
    scanf("%d", &n);

    while (n <= 3)
    {
        printf("Unijeli ste pogresan broj studenata\n");
        printf("Unesite broj studenata: ");
        scanf("%d", &n);
    }

    aktivniStudenti = n;
    pthread_t studenti[n];

    for (int i = 0; i < n; i++)
    {
        pthread_create(&studenti[i], NULL, Student, NULL);
        usleep((rand() % 401 + 100) * 1000);
    }

    pthread_create(&partibrejker, NULL, PartiBrejker, NULL);

    for (int i = 0; i < n; i++)
    {
        pthread_join(studenti[i], NULL);
    }

    pthread_join(partibrejker, NULL);

    pthread_mutex_destroy(&kljuc);
    sem_destroy(&partiBrejker);
    return 0;
}