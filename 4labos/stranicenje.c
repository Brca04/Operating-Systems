#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BROJ_OKVIRA 5
#define STR_VEL 256
#define MAX_PROCESA 5
#define MAX_STR 5

typedef struct
{
    int valjan;
    int okvir;
} Tablica;

typedef struct
{
    Tablica stranice[MAX_STR];
    int aktivan;
} Proces;

typedef struct
{
    int proces;
    char str;
} Okvir;

Proces procesi[MAX_PROCESA];
Okvir okviri[BROJ_OKVIRA];
int bitoviSat[BROJ_OKVIRA];
int satTmp[BROJ_OKVIRA];
int hitTmp[BROJ_OKVIRA];
int kazaljkaSat = 0;
int restart = 0;
unsigned char memory[BROJ_OKVIRA][STR_VEL];

void printOkviri()
{
    printf("frames:");
    for (int i = 0; i < BROJ_OKVIRA; ++i)
    {
        printf(" %d:[%d:%d]", i, okviri[i].proces, okviri[i].str);
    }
    printf("\n");
}

void printSat()
{
    printf("Clock: ");
    for (int i = 0; i < BROJ_OKVIRA; ++i)
    {
        printf("%d", bitoviSat[i]);
        if (i < BROJ_OKVIRA - 1)
        {
            printf("-");
        }
    }
    satTmp[kazaljkaSat] = 1;
    printf("\nHand: ");
    printf(" ");
    for (int i = 0; i < BROJ_OKVIRA; i++)
    {
        if (satTmp[i] == 1)
        {
            printf("^ ");
        }
        else
        {
            printf("  ");
        }
    }
    satTmp[kazaljkaSat] = 0;
    printf("\n");
}

void inicijalizacija()
{
    for (int i = 0; i < MAX_PROCESA; i++)
    {
        procesi[i].aktivan = 0;
        for (int j = 0; j < MAX_STR; j++)
        {
            procesi[i].stranice[j].valjan = 0;
        }
    }
    for (int i = 0; i < BROJ_OKVIRA; i++)
    {
        okviri[i].proces = -1;
        okviri[i].str = -1;
        bitoviSat[i] = 0;
        satTmp[i] = 0;
        hitTmp[i] = 0;
    }
}

int nadiOkvir(int proc, int page)
{
    if (procesi[proc].stranice[page].valjan)
    {
        return procesi[proc].stranice[page].okvir;
    }
    return -1;
}

int satniAlgoritam()
{
    while (1)
    {
        if (bitoviSat[kazaljkaSat] == 0)
        {
            int tmp = kazaljkaSat;
            if (restart == 0)
            {
                kazaljkaSat = (kazaljkaSat + 1) % BROJ_OKVIRA;
            }
            return tmp;
        }
        bitoviSat[kazaljkaSat] = 0;
        kazaljkaSat = (kazaljkaSat + 1) % BROJ_OKVIRA;
    }
}

void ucitajStr(int proc, int page, int *zamProc, int *zamStr, int *korOkv)
{

    printf("Clock before: ");
    for (int i = 0; i < BROJ_OKVIRA; i++)
    {
        printf("%d", bitoviSat[i]);
        if (i < BROJ_OKVIRA - 1)
        {
            printf("-");
        }
    }
    satTmp[kazaljkaSat] = 1;
    printf("\nHand before:");
    printf(" ");
    for (int i = 0; i < BROJ_OKVIRA; i++)
    {
        if (satTmp[i] == 1)
        {
            printf(" ^");
        }
        else
        {
            printf("  ");
        }
    }
    satTmp[kazaljkaSat] = 0;
    printf("\n");

    restart = 1;
    int okvir = satniAlgoritam();
    restart = 0;

    printf("Clock after: ");
    for (int i = 0; i < BROJ_OKVIRA; i++)
    {
        printf("%d", bitoviSat[i]);
        if (i < BROJ_OKVIRA - 1)
        {
            printf("-");
        }
    }
    satTmp[kazaljkaSat] = 1;
    printf("\nHand after:");
    printf(" ");
    for (int i = 0; i < BROJ_OKVIRA; i++)
    {
        if (satTmp[i] == 1)
        {
            printf(" ^");
        }
        else
        {
            printf("  ");
        }
    }
    satTmp[kazaljkaSat] = 0;
    printf("\n");

    *zamProc = okviri[okvir].proces;
    *zamStr = okviri[okvir].str;

    if (*zamProc != -1)
    {
        procesi[*zamProc].stranice[*zamStr].valjan = 0;
    }

    okviri[okvir].proces = proc;
    okviri[okvir].str = page;
    procesi[proc].stranice[page].valjan = 1;
    procesi[proc].stranice[page].okvir = okvir;
    bitoviSat[okvir] = 1;

    *korOkv = okvir;
}

void pristupDisku(int proc, char mode, unsigned short la, unsigned char vri)
{
    int str = la >> 8;
    int offset = la & 0xFF;

    if (proc >= MAX_PROCESA || str >= MAX_STR)
    {
        printf("process %d WRITE(0x%02x) LA=0x%04x\n", proc, vri, la);
        printf("MEMORY FAULT: page %d not allocated, terminating process %d\n", str, proc);
        printf("\n");
        procesi[proc].aktivan = 0;
        return;
    }

    procesi[proc].aktivan = 1;

    if (mode == 'R')
    {
        printf("process %d READ LA=0x%04x\n", proc, la);
    }
    else
    {
        printf("process %d WRITE(0x%02x) LA=0x%04x\n", proc, vri, la);
    }

    int okvir = nadiOkvir(proc, str);
    if (okvir != -1)
    {
        printOkviri();
        printf("HIT:    ");
        for (int i = 0; i < BROJ_OKVIRA; i++)
        {
            if (i == okvir)
            {
                printf("^^^^^^^");
            }
            else
            {
                printf("        ");
            }
        }
        printf("\n");
        
        printf("paging: process %d, page=%d => frame=%d, 0x%04x => 0x%04x\n",
               proc, str, okvir, la, (okvir << 8) | offset);
        if (mode == 'R')
        {
            printf("read vri at 0x%04x => 0x%02x\n", (okvir << 8) | offset, memory[okvir][offset]);
        }
        else
        {
            memory[okvir][offset] = vri;
            printf("save (0x%02x) at 0x%04x\n", vri, (okvir << 8) | offset);
        }
        bitoviSat[okvir] = 1;
        kazaljkaSat = (okvir + 1) % BROJ_OKVIRA;
        printSat();
        printf("\n");
    }
    else
    {
        if (mode == 'R')
        {
            printf("MEMORY FAULT: page %d not allocated, terminating process %d\n", str, proc);
            printf("\n"); // Makni ako zeza
            return;
        }
        printf("MISS (page %d not in memory)\n", str);

        int stariProc, staraStr, noviOkvir;
        ucitajStr(proc, str, &stariProc, &staraStr, &noviOkvir);

        printf("use frame %d:\n", noviOkvir);
        if (stariProc != -1)
            printf("- save to disk:   process %d, page %d\n", stariProc, staraStr);
        printf("- load from disk: process %d, page %d\n", proc, str);
        printOkviri();
        printf("restarting instruction:\n");

        pristupDisku(proc, mode, la, vri);
    }
}

int main()
{
    inicijalizacija();

    // Citanje nepostojeceg procesa
    pristupDisku(0, 'R', 0x0300, 0x00);

    // Genericni primjeri
    /*pristupDisku(1, 'W', 0x0120, 0xCD);
    pristupDisku(1, 'R', 0x0120, 0x00);*/

    // Memory Fault primjeri, paznja na macro vrijednosti
    /*pristupDisku(0, 'W', 0x0801, 0xAA);
    pristupDisku(9, 'R', 0x0001, 0x00);*/

    // Primjer sve frameove napuni jednim procesom pa pregazi drugim procesom i procitaj
    /*pristupDisku(0, 'W', 0x0001, 0xA0);
    pristupDisku(0, 'W', 0x0101, 0xA1);
    pristupDisku(0, 'W', 0x0201, 0xA2);
    pristupDisku(0, 'W', 0x0301, 0xA3);
    pristupDisku(0, 'W', 0x0401, 0xA4);
    pristupDisku(0, 'W', 0x0501, 0xA5);
    pristupDisku(0, 'W', 0x0601, 0xA6);
    pristupDisku(0, 'W', 0x0701, 0xA7);
    pristupDisku(0, 'W', 0x0801, 0xA8);
    pristupDisku(2, 'W', 0x0321, 0x73);
    pristupDisku(2, 'R', 0x0321, 0x73);*/

    return 0;
}
