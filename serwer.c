#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <ncurses.h>

#define IPC_SIZE 512
#define IPC_id 77
#define serwerGotowy 98
#define graczGotowy 99
#define SHM_id 100
#define SHM_SIZE 128
#define sygnalGracz1 1
#define sygnalGracz2 2
#define sygnalGracz3 3
#define treningGracz1 5
#define treningGracz2 6
#define treningGracz3 7
#define sygnalStan1 10
#define sygnalStan2 11
#define sygnalStan3 12
#define sygnalSerwer 20
#define atak1do2 30
#define atak1do3 31
#define atak2do1 32
#define atak2do3 33
#define atak3do1 34
#define atak3do2 35

//oid startNcurses(bool useRaw, bool noEcho);
//void printMenu(WINDOW * menu, string choices[], int size, int highlight);

typedef struct Wiadomosc{ 
    long mtype;
    char mtext[IPC_SIZE/2];
    int mnum;
}Wiadomosc;

typedef struct Walka{
    long mtype;
    int units[3];
}Walka;

struct Wiadomosc msg;
static struct sembuf buf;
int IPC_ID;
int IPC_gracz1;
int IPC_gracz2;
int IPC_gracz3;
int IPC_a1d2;

void wyslijWiadomosc(int signal, char text[IPC_SIZE], int num){
    msg.mtype = signal;
    strcpy(msg.mtext,text);
    msg.mnum = num;
    msgsnd(IPC_ID,&msg,IPC_SIZE,0);
}

void podniesSem(int semid, int semnum){
    buf.sem_num = semnum;
    buf.sem_op = 1;
    buf.sem_flg = 0;
    semop(semid, &buf, 1);
}

void opuscSem(int semid, int semnum){
    buf.sem_num = semnum;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    semop(semid, &buf, 1);
}
void prtd(double x){
    printw("%f",x);
}

void print(char text[]){
    printw("%s\n",text);
    refresh();
}
void printd(char text[],char text2[]){
    printw("%s%s\n",text,text2);
    refresh();
}
void printc(char text[],char text2[],int num,char text4[]){
    printw("%s%s%d%s\n",text,text2,num,text4);
    refresh();
}
void printt(char text[],char text2[],char text3[]){
    printw("%s%s%s\n",text,text2,text3);
    refresh();
}


int main()
{
    initscr();
    start_color();
    init_pair(1,COLOR_CYAN,COLOR_BLACK);
    init_pair(2,COLOR_BLACK,COLOR_CYAN);
    int y,x,begY,begX,maxY,maxX;
    getyx(stdscr,y,x);
    getbegyx(stdscr,begY,begX);
    getmaxyx(stdscr,maxY,maxX);
    WINDOW * win = newwin(3,maxX,0,0);
    wbkgd(win,COLOR_PAIR(2));
    bkgd(COLOR_PAIR(1));
    //box(win,0,0);
    refresh();
    wrefresh(win);
    mvwprintw(win,1,maxX/2-3,"SERWER");
    wrefresh(win);
    move(4,0);

//-----INICJACJA
    IPC_ID = msgget(IPC_id,IPC_CREAT|0600);
    IPC_gracz1 = msgget(IPC_id+1,IPC_CREAT|0600);
    IPC_gracz2 = msgget(IPC_id+2,IPC_CREAT|0600);
    IPC_gracz3 = msgget(IPC_id+3,IPC_CREAT|0600);
    IPC_a1d2 = msgget(IPC_id+4,IPC_CREAT|0600);
    wyslijWiadomosc(serwerGotowy,"1",1);
    print("Serwer gotowy");
    //printw("Serwer gotowy");refresh();

//-----SEMAFORY, PAMIEC
    int semid = semget(555,3,IPC_CREAT|0600); //3
    semctl(semid,0,SETVAL,1);
    semctl(semid,1,SETVAL,1);
    semctl(semid,2,SETVAL,1);
    opuscSem(semid,0);
    opuscSem(semid,1);
    opuscSem(semid,2);
    //nazwygraczy
    int nazwaGracza1SH = shmget(SHM_id+1,SHM_SIZE,IPC_CREAT|0666);
    char* nazwaGracza1 = shmat(nazwaGracza1SH,NULL,0);
    int nazwaGracza2SH = shmget(SHM_id+2,SHM_SIZE,IPC_CREAT|0666);
    char* nazwaGracza2 = shmat(nazwaGracza2SH,NULL,0);
    int nazwaGracza3SH = shmget(SHM_id+3,SHM_SIZE,IPC_CREAT|0666);
    char* nazwaGracza3 = shmat(nazwaGracza3SH,NULL,0);
    //jednostki
    int lekkaGracz1SH = shmget(SHM_id+4,SHM_SIZE,IPC_CREAT|0666);
    char* lekkaGracz1 = shmat(lekkaGracz1SH,NULL,0);
    strcpy(lekkaGracz1,"0");
    int lekkaGracz2SH = shmget(SHM_id+5,SHM_SIZE,IPC_CREAT|0666);
    char* lekkaGracz2 = shmat(lekkaGracz2SH,NULL,0);
    strcpy(lekkaGracz2,"0");
    int lekkaGracz3SH = shmget(SHM_id+6,SHM_SIZE,IPC_CREAT|0666);
    char* lekkaGracz3 = shmat(lekkaGracz3SH,NULL,0);
    strcpy(lekkaGracz3,"0");

    int ciezkaGracz1SH = shmget(SHM_id+7,SHM_SIZE,IPC_CREAT|0666);
    char* ciezkaGracz1 = shmat(ciezkaGracz1SH,NULL,0);
    strcpy(ciezkaGracz1,"0");
    int ciezkaGracz2SH = shmget(SHM_id+8,SHM_SIZE,IPC_CREAT|0666);
    char* ciezkaGracz2 = shmat(ciezkaGracz2SH,NULL,0);
    strcpy(ciezkaGracz2,"0");
    int ciezkaGracz3SH = shmget(SHM_id+9,SHM_SIZE,IPC_CREAT|0666);
    char* ciezkaGracz3 = shmat(ciezkaGracz3SH,NULL,0);
    strcpy(ciezkaGracz3,"0");

    int jazdaGracz1SH = shmget(SHM_id+10,SHM_SIZE,IPC_CREAT|0666);
    char* jazdaGracz1 = shmat(jazdaGracz1SH,NULL,0);
    strcpy(jazdaGracz1,"0");
    int jazdaGracz2SH = shmget(SHM_id+11,SHM_SIZE,IPC_CREAT|0666);
    char* jazdaGracz2 = shmat(jazdaGracz2SH,NULL,0);
    strcpy(jazdaGracz2,"0");
    int jazdaGracz3SH = shmget(SHM_id+12,SHM_SIZE,IPC_CREAT|0666);
    char* jazdaGracz3 = shmat(jazdaGracz3SH,NULL,0);
    strcpy(jazdaGracz3,"0");

    int roboGracz1SH = shmget(SHM_id+13,SHM_SIZE,IPC_CREAT|0666);
    char* roboGracz1 = shmat(roboGracz1SH,NULL,0);
    strcpy(roboGracz1,"0");
    int roboGracz2SH = shmget(SHM_id+14,SHM_SIZE,IPC_CREAT|0666);
    char* roboGracz2 = shmat(roboGracz2SH,NULL,0);
    strcpy(roboGracz2,"0");
    int roboGracz3SH = shmget(SHM_id+15,SHM_SIZE,IPC_CREAT|0666);
    char* roboGracz3 = shmat(roboGracz3SH,NULL,0);
    strcpy(roboGracz3,"0");

    int surowceGracz1SH = shmget(SHM_id+16,SHM_SIZE,IPC_CREAT|0666);
    char* surowceGracz1 = shmat(surowceGracz1SH,NULL,0);
    strcpy(surowceGracz1,"300");
    int surowceGracz2SH = shmget(SHM_id+17,SHM_SIZE,IPC_CREAT|0666);
    char* surowceGracz2 = shmat(surowceGracz2SH,NULL,0);
    strcpy(surowceGracz2,"300");
    int surowceGracz3SH = shmget(SHM_id+18,SHM_SIZE,IPC_CREAT|0666);
    char* surowceGracz3 = shmat(surowceGracz3SH,NULL,0);
    strcpy(surowceGracz3,"300");

    //punkty
    int pktGracz1SH = shmget(SHM_id+19,SHM_SIZE,IPC_CREAT|0666);
    char* pktGracz1 = shmat(pktGracz1SH,NULL,0);
    strcpy(pktGracz1,"0");
    int pktGracz2SH = shmget(SHM_id+20,SHM_SIZE,IPC_CREAT|0666);
    char* pktGracz2 = shmat(pktGracz2SH,NULL,0);
    strcpy(pktGracz2,"0");
    int pktGracz3SH = shmget(SHM_id+21,SHM_SIZE,IPC_CREAT|0666);
    char* pktGracz3 = shmat(pktGracz3SH,NULL,0);
    strcpy(pktGracz3,"0");

//-----LOGOWANIE GRACZY
    podniesSem(semid,0);
    podniesSem(semid,1);
    podniesSem(semid,2);
    print("Czekam na graczy");

    msgrcv(IPC_ID,&msg,IPC_SIZE,graczGotowy,0);
    opuscSem(semid,0);
    strcpy(nazwaGracza1,msg.mtext);
    printd("Dolaczyl pierwszy gracz ",nazwaGracza1);
    podniesSem(semid,0);
    wyslijWiadomosc(sygnalSerwer,"1",sygnalGracz1);
    
    msgrcv(IPC_ID,&msg,IPC_SIZE,graczGotowy,0);
    opuscSem(semid,1);
    strcpy(nazwaGracza2,msg.mtext);
    printd("Dolaczyl drugi gracz ",nazwaGracza2);
    podniesSem(semid,1);
    wyslijWiadomosc(sygnalSerwer,"2",sygnalGracz2);

    msgrcv(IPC_ID,&msg,IPC_SIZE,graczGotowy,0);
    opuscSem(semid,2);
    strcpy(nazwaGracza3,msg.mtext);
    printd("Dolaczyl trzeci gracz ",nazwaGracza3);
    podniesSem(semid,2);
    wyslijWiadomosc(sygnalSerwer,"3",sygnalGracz3);

//-----WYMIANA NAZW GRACZY
    
    wyslijWiadomosc(sygnalSerwer,"wszyscy",0);
    opuscSem(semid,0);
    wyslijWiadomosc(sygnalGracz1,nazwaGracza2,1);
    wyslijWiadomosc(sygnalGracz1,nazwaGracza3,1);
    podniesSem(semid,0);
    opuscSem(semid,1);
    wyslijWiadomosc(sygnalGracz2,nazwaGracza1,1);
    wyslijWiadomosc(sygnalGracz2,nazwaGracza3,1);
    podniesSem(semid,1);
    opuscSem(semid,2);
    wyslijWiadomosc(sygnalGracz3,nazwaGracza1,1);
    wyslijWiadomosc(sygnalGracz3,nazwaGracza2,1);
    podniesSem(semid,2);

    print("Zakonczono logowanie");




//-----ZGLOSZENIE TRENINGU
    //Gracz1
    if(!fork()){
        while(1){
            struct Wiadomosc wiad;
            struct Walka walka;
            char jednostka[IPC_SIZE]="";
            int ile = 0;
            if(msgrcv(IPC_ID,&wiad,IPC_SIZE,treningGracz1,0)==IPC_SIZE){
                strcpy(jednostka, wiad.mtext);
                ile = wiad.mnum;
                int surowce1 = atoi(surowceGracz1);
                opuscSem(semid,0);
                if(strcmp(jednostka,"lekka")==0){
                    if(ile * 100 <= surowce1){
                        surowce1 -= ile * 100;
                        sprintf(surowceGracz1,"%d",surowce1);
                        msgsnd(IPC_gracz1, &wiad, IPC_SIZE,0);
                        printc(nazwaGracza1,": trenuje ", ile, " jednostek lekkiej piechoty");
                    }
                    else wyslijWiadomosc(56,"Za malo surowcow",1);
                } else if(strcmp(jednostka,"ciezka")==0){
                    if(ile * 250 <= surowce1){
                        surowce1 -= ile * 250;
                        sprintf(surowceGracz1,"%d",surowce1);
                        msgsnd(IPC_gracz1, &wiad, IPC_SIZE,0);
                        printc(nazwaGracza1,": trenuje ", ile, " jednostek ciezkiej piechoty");
                    }
                    //else wyslijWiadomosc(sygnalGracz1,"Za malo surowcow\n",1);
                } else if(strcmp(jednostka,"jazda")==0){
                    if(ile * 550 <= surowce1){
                        surowce1 -= ile * 550;
                        sprintf(surowceGracz1,"%d",surowce1);
                        msgsnd(IPC_gracz1, &wiad, IPC_SIZE,0);
                        printc(nazwaGracza1,": trenuje ", ile, " jednostek jazdy");
                    }
                    //else wyslijWiadomosc(sygnalGracz1,"Za malo surowcow\n",1);
                } else if(strcmp(jednostka,"robotnicy")==0){
                    if(ile * 150 <= surowce1){
                        surowce1 -= ile * 150;
                        sprintf(surowceGracz1,"%d",surowce1);
                        msgsnd(IPC_gracz1, &wiad, IPC_SIZE,0);
                        printc(nazwaGracza1,": trenuje ", ile, " jednostek robotnikow");
                    }
                    //else wyslijWiadomosc(sygnalGracz1,"Za malo surowcow\n",1);
                } podniesSem(semid,0);

            }
        }
        exit(1);
    }
    //Gracz2
    if(!fork()){
        while(1){
            struct Wiadomosc wiad;
            char jednostka[IPC_SIZE]="";
            int ile = 0;
            if(msgrcv(IPC_ID,&wiad,IPC_SIZE,treningGracz2,0)==IPC_SIZE){
                strcpy(jednostka, wiad.mtext);
                ile = wiad.mnum;
                int surowce2 = atoi(surowceGracz2);
                opuscSem(semid,1);
                if(strcmp(jednostka,"lekka")==0){
                    if(ile * 100 <= surowce2){
                        surowce2 -= ile * 100;
                        sprintf(surowceGracz2,"%d",surowce2);
                        msgsnd(IPC_gracz2, &wiad, IPC_SIZE,0);
                        printc(nazwaGracza2,": trenuje ", ile, " jednostek lekkiej piechoty");
                    }
                    //else wyslijWiadomosc(sygnalGracz2,"Za malo surowcow\n",1);
                } else if(strcmp(jednostka,"ciezka")==0){
                    if(ile * 250 <= surowce2){
                        surowce2 -= ile * 250;
                        sprintf(surowceGracz2,"%d",surowce2);
                        msgsnd(IPC_gracz2, &wiad, IPC_SIZE,0);
                        printc(nazwaGracza2,": trenuje ", ile, " jednostek ciezkiej piechoty");
                    }
                    //else wyslijWiadomosc(sygnalGracz2,"Za malo surowcow\n",1);
                } else if(strcmp(jednostka,"jazda")==0){
                    if(ile * 550 <= surowce2){
                        surowce2 -= ile * 550;
                        sprintf(surowceGracz2,"%d",surowce2);
                        msgsnd(IPC_gracz2, &wiad, IPC_SIZE,0);
                        printc(nazwaGracza2,": trenuje ", ile, " jednostek jazdy");
                    }
                    //else wyslijWiadomosc(sygnalGracz2,"Za malo surowcow\n",1);
                } else if(strcmp(jednostka,"robotnicy")==0){
                    if(ile * 150 <= surowce2){
                        surowce2 -= ile * 150;
                        sprintf(surowceGracz2,"%d",surowce2);
                        msgsnd(IPC_gracz2, &wiad, IPC_SIZE,0);
                        printc(nazwaGracza2,": trenuje ", ile, " robotnikow");
                    }
                    //else wyslijWiadomosc(sygnalGracz2,"Za malo surowcow\n",1);
                } podniesSem(semid,1);

            }
        }
        exit(1);
    }
    //Gracz3
    if(!fork()){
        while(1){
            struct Wiadomosc wiad;
            char jednostka[IPC_SIZE]="";
            int ile = 0;
            if(msgrcv(IPC_ID,&wiad,IPC_SIZE,treningGracz3,0)==IPC_SIZE){
                strcpy(jednostka, wiad.mtext);
                ile = wiad.mnum;
                int surowce3 = atoi(surowceGracz3);
                opuscSem(semid,2);
                if(strcmp(jednostka,"lekka")==0){
                    if(ile * 100 <= surowce3){
                        surowce3 -= ile * 100;
                        sprintf(surowceGracz3,"%d",surowce3);
                        msgsnd(IPC_gracz3, &wiad, IPC_SIZE,0);
                        printc(nazwaGracza3,": trenuje ", ile, " jednostek lekkiej piechoty");
                    }
                    //else wyslijWiadomosc(sygnalGracz3,"Za malo surowcow\n",1);
                } else if(strcmp(jednostka,"ciezka")==0){
                    if(ile * 250 <= surowce3){
                        surowce3 -= ile * 250;
                        sprintf(surowceGracz3,"%d",surowce3);
                        msgsnd(IPC_gracz3, &wiad, IPC_SIZE,0);
                        printc(nazwaGracza3,": trenuje ", ile, " jednostek ciezkiej piechoty");
                    }
                    //else wyslijWiadomosc(sygnalGracz3,"Za malo surowcow\n",1);
                } else if(strcmp(jednostka,"jazda")==0){
                    if(ile * 550 <= surowce3){
                        surowce3 -= ile * 550;
                        sprintf(surowceGracz3,"%d",surowce3);
                        msgsnd(IPC_gracz3, &wiad, IPC_SIZE,0);
                        printc(nazwaGracza3,": trenuje ", ile, " jednostek jazdy");
                    }
                    //else wyslijWiadomosc(sygnalGracz3,"Za malo surowcow\n",1);
                } else if(strcmp(jednostka,"robotnicy")==0){
                    if(ile * 150 <= surowce3){
                        surowce3 -= ile * 150;
                        sprintf(surowceGracz3,"%d",surowce3);
                        msgsnd(IPC_gracz3, &wiad, IPC_SIZE,0);
                        printc(nazwaGracza3,": trenuje ", ile, " jednostek robotnikow");
                    }
                    //else wyslijWiadomosc(sygnalGracz3,"Za malo surowcow\n",1);
                } podniesSem(semid,2);

            }
        }
        exit(1);
    }


//-----TRENING
    //Gracz1
    if(!fork()){
        while(1){
            struct Wiadomosc wiad;
            char jednostka[IPC_SIZE]="";
            int ile = 0, i;
            if(msgrcv(IPC_gracz1,&wiad,IPC_SIZE,treningGracz1,0)==IPC_SIZE){
                strcpy(jednostka, wiad.mtext);
                ile = wiad.mnum;
                if(strcmp(jednostka,"lekka")==0){
                    for(i=1;i<=ile;i++){
                        sleep(2);
                        opuscSem(semid,0);
                        int lekka1 = atoi(lekkaGracz1);
                        lekka1 += 1;
                        sprintf(lekkaGracz1,"%d",lekka1);
                        podniesSem(semid,0);
                    }
                    printd(nazwaGracza1,": ukonczono trening piechoty lekkiej");
                    //wiad.mtext="ukonczono";
                    //wyslijWiadomosc(sygnalGracz1,"ukonczono trening",0);
                }else if(strcmp(jednostka,"ciezka")==0){
                    for(i=1;i<=ile;i++){
                        sleep(3);
                        opuscSem(semid,0);
                        int ciezka1 = atoi(ciezkaGracz1);
                        ciezka1 += 1;
                        sprintf(ciezkaGracz1,"%d",ciezka1);
                        podniesSem(semid,0);
                    }
                    printd(nazwaGracza1,": ukonczono trening piechoty ciezkiej");
                }else if(strcmp(jednostka,"jazda")==0){
                    for(i=1;i<=ile;i++){
                        sleep(5);
                        opuscSem(semid,0);
                        int jazda1 = atoi(jazdaGracz1);
                        jazda1 += 1;
                        sprintf(jazdaGracz1,"%d",jazda1);
                        podniesSem(semid,0);
                    }
                    printd(nazwaGracza1,": ukonczono trening jazdy");
                }else if(strcmp(jednostka,"robotnicy")==0){
                    for(i=1;i<=ile;i++){
                        sleep(2);
                        opuscSem(semid,0);
                        int robo1 = atoi(roboGracz1);
                        robo1 += 1;
                        sprintf(roboGracz1,"%d",robo1);
                        podniesSem(semid,0);
                    }
                    printd(nazwaGracza1,": ukonczono trening robotnikow");
                }
            }
        }
    }
    //Gracz2
    if(!fork()){
        while(1){
            struct Wiadomosc wiad;
            char jednostka[IPC_SIZE]="";
            int ile = 0, i;
            if(msgrcv(IPC_gracz2,&wiad,IPC_SIZE,treningGracz2,0)==IPC_SIZE){
                strcpy(jednostka, wiad.mtext);
                ile = wiad.mnum;
                if(strcmp(jednostka,"lekka")==0){
                    for(i=1;i<=ile;i++){
                        sleep(2);
                        opuscSem(semid,1);
                        int lekka2 = atoi(lekkaGracz2);
                        lekka2 += 1;
                        sprintf(lekkaGracz2,"%d",lekka2);
                        podniesSem(semid,1);
                    }
                    printd(nazwaGracza2,": ukonczono trening piechoty lekkiej");
                }else if(strcmp(jednostka,"ciezka")==0){
                    for(i=1;i<=ile;i++){
                        sleep(3);
                        opuscSem(semid,1);
                        int ciezka2 = atoi(ciezkaGracz2);
                        ciezka2 += 1;
                        sprintf(ciezkaGracz2,"%d",ciezka2);
                        podniesSem(semid,1);
                    }
                    printd(nazwaGracza2,": ukonczono trening piechoty ciezkiej");
                }else if(strcmp(jednostka,"jazda")==0){
                    for(i=1;i<=ile;i++){
                        sleep(5);
                        opuscSem(semid,1);
                        int jazda2 = atoi(jazdaGracz2);
                        jazda2 += 1;
                        sprintf(jazdaGracz2,"%d",jazda2);
                        podniesSem(semid,1);
                    }
                    printd(nazwaGracza2,": ukonczono trening jazdy");
                }else if(strcmp(jednostka,"robotnicy")==0){
                    for(i=1;i<=ile;i++){
                        sleep(2);
                        opuscSem(semid,1);
                        int robo2 = atoi(roboGracz2);
                        robo2 += 1;
                        sprintf(roboGracz2,"%d",robo2);
                        podniesSem(semid,1);
                    }
                    printd(nazwaGracza2,": ukonczono trening robotnikow");
                }
            }
        }
    }
    //Gracz3
    if(!fork()){
        while(1){
            struct Wiadomosc wiad;
            char jednostka[IPC_SIZE]="";
            int ile = 0, i;
            if(msgrcv(IPC_gracz3,&wiad,IPC_SIZE,treningGracz3,0)==IPC_SIZE){
                strcpy(jednostka, wiad.mtext);
                ile = wiad.mnum;
                if(strcmp(jednostka,"lekka")==0){
                    for(i=1;i<=ile;i++){
                        sleep(2);
                        opuscSem(semid,2);
                        int lekka3 = atoi(lekkaGracz3);
                        lekka3 += 1;
                        sprintf(lekkaGracz3,"%d",lekka3);
                        podniesSem(semid,2);
                    }
                    printd(nazwaGracza3,": ukonczono trening piechoty lekkiej");
                }else if(strcmp(jednostka,"ciezka")==0){
                    for(i=1;i<=ile;i++){
                        sleep(3);
                        opuscSem(semid,2);
                        int ciezka3 = atoi(ciezkaGracz3);
                        ciezka3 += 1;
                        sprintf(ciezkaGracz3,"%d",ciezka3);
                        podniesSem(semid,2);
                    }
                    printd(nazwaGracza3,": ukonczono trening piechoty ciezkiej");
                }else if(strcmp(jednostka,"jazda")==0){
                    for(i=1;i<=ile;i++){
                        sleep(5);
                        opuscSem(semid,2);
                        int jazda3 = atoi(jazdaGracz3);
                        jazda3 += 1;
                        sprintf(jazdaGracz3,"%d",jazda3);
                        podniesSem(semid,2);
                    }
                    printd(nazwaGracza3,": ukonczono trening jazdy");
                }else if(strcmp(jednostka,"robotnicy")==0){
                    for(i=1;i<=ile;i++){
                        sleep(2);
                        opuscSem(semid,2);
                        int robo3 = atoi(roboGracz3);
                        robo3 += 1;
                        sprintf(roboGracz3,"%d",robo3);
                        podniesSem(semid,2);
                    }
                    printd(nazwaGracza3,": ukonczono trening robotnikow");
                }
            }
        }
    }

//-----SUROWCE
    if(!fork()){
        while(1){
            sleep(1);
            //Gracz1
            opuscSem(semid,0);
            int surowce1=atoi(surowceGracz1);
            int robo1=atoi(roboGracz1);
            surowce1 += (50 + robo1 * 5);
            sprintf(surowceGracz1,"%d",surowce1);
            podniesSem(semid,0); 
            //Gracz2
            opuscSem(semid,1);
            int surowce2=atoi(surowceGracz2);
            int robo2=atoi(roboGracz2);
            surowce2 += (50 + robo2 * 5);
            sprintf(surowceGracz2,"%d",surowce2);
            podniesSem(semid,1); 
            //Gracz3
            opuscSem(semid,2);
            int surowce3=atoi(surowceGracz3);
            int robo3=atoi(roboGracz3);
            surowce3 += (50 + robo3 * 5);
            sprintf(surowceGracz3,"%d",surowce3);
            podniesSem(semid,2); 
        }
    }

//-----ATAK
    //gracz1 atakuje gracza2
    if(!fork()){
        while(1){
            struct Wiadomosc wiad;
            
            //if(msgrcv(IPC_ID,&wiad,IPC_SIZE,treningGracz2,0)==IPC_SIZE)
            if(msgrcv(IPC_ID,&wiad,IPC_SIZE,30,0)==IPC_SIZE){
                print("Walka");
                int lek = wiad.mtext[0];
                int cie = wiad.mtext[1];
                int jaz = 0;
                int e = wiad.mtext[3];
                opuscSem(semid,0);
                opuscSem(semid,1);
                int lekka1 = atoi(lekkaGracz1);
                int ciezka1 = atoi(ciezkaGracz1);
                int jazda1 = atoi(jazdaGracz1);
                int lekka2 = atoi(lekkaGracz2);
                int ciezka2 = atoi(ciezkaGracz2);
                int jazda2 = atoi(jazdaGracz2);
                if(lek<=lekka1 && cie<=ciezka1 && jaz<=jazda1){
                    printt(nazwaGracza1,": atakuje ",nazwaGracza2);
                    lekka1 -= lek;
                    ciezka1 -= cie;
                    jazda1 -= jaz;
                    sprintf(lekkaGracz1,"%d",lekka1);
                    sprintf(ciezkaGracz1,"%d",ciezka1);
                    sprintf(jazdaGracz1,"%d",jazda1);
                    //print(lekkaGracz1);
                    podniesSem(semid,0);
                    podniesSem(semid,1);
                    sleep(5);
                    opuscSem(semid,0);
                    opuscSem(semid,1);
                    double odAtak = lek*1 + cie*1.5 + jaz*3.5;
                    double doObr = lekka2*1.2 + ciezka2*3 + jazda2*1.2;
                    //atak udany
                    if(odAtak - doObr > 0){
                        double ao;
                        if(doObr>0)ao = odAtak/doObr;
                        else ao=1;
                        lek*=(int)ao;
                        cie*=(int)ao;
                        jaz*=(int)ao;
                        lekka1+=lek;
                        ciezka1+=cie;
                        jazda1+=jaz;
                        sprintf(lekkaGracz2,"%d",0);
                        sprintf(ciezkaGracz2,"%d",0);
                        sprintf(jazdaGracz2,"%d",0);
                        sprintf(lekkaGracz1,"%d",lekka1);
                        sprintf(ciezkaGracz1,"%d",ciezka1);
                        sprintf(jazdaGracz1,"%d",jazda1);
                        //wyslijWiadomosc(sygnalGracz2,"Przegrales obrone",1);
                        int pkt1 = atoi(pktGracz1);
                        pkt1 += 1;
                        sprintf(pktGracz1,"%d",pkt1);
                        //wyslijWiadomosc(sygnalGracz1,"Wygrales atak",1);
                        printd(nazwaGracza1,": Atak udany, zdobywa 1 pkt");
                        printd(nazwaGracza2,": Przegral atak");
                    } 
                    //atak nieudany
                    else {
                        double oa=doObr/odAtak;
                        double ao;
                        if(doObr>0)ao = odAtak/doObr;
                        lek*=(int)oa;
                        cie*=(int)oa;
                        jaz*=(int)oa;
                        lekka1+=lek;
                        ciezka1+=cie;
                        jazda1+=jaz;
                        lekka2*=(int)ao;
                        ciezka2*=(int)ao;
                        jazda2*=(int)ao;
                        sprintf(lekkaGracz1,"%d",lekka1);
                        sprintf(ciezkaGracz1,"%d",ciezka1);
                        sprintf(jazdaGracz1,"%d",jazda1);
                        sprintf(lekkaGracz2,"%d",lekka2);
                        sprintf(ciezkaGracz2,"%d",ciezka2);
                        sprintf(jazdaGracz2,"%d",jazda2);
                        printd(nazwaGracza1,": Atak nieudany");
                        printd(nazwaGracza2,": Atak odparty");
                        //wyslijWiadomosc(sygnalGracz1,"Atak nieudany",1);
                        //wyslijWiadomosc(sygnalGracz2,"Atak odparty",1);
                    }
                    podniesSem(semid,0);
                    podniesSem(semid,1);

                }else {
                    printd(nazwaGracza1,"Brak jednostek do ataku");
                    podniesSem(semid,0);
                    podniesSem(semid,1);
                }
            }
        }
    }

    //gracz1 atakuje gracza3
    if(!fork()){
        while(1){
            struct Wiadomosc wiad;
            if(msgrcv(IPC_ID,&wiad,IPC_SIZE,31,0)==IPC_SIZE){
                print("Walka");
                int lek = wiad.mtext[0];
                int cie = wiad.mtext[1];
                int jaz = 0;
                opuscSem(semid,0);
                opuscSem(semid,1);
                int lekka1 = atoi(lekkaGracz1);
                int ciezka1 = atoi(ciezkaGracz1);
                int jazda1 = atoi(jazdaGracz1);
                int lekka2 = atoi(lekkaGracz3);
                int ciezka2 = atoi(ciezkaGracz3);
                int jazda2 = atoi(jazdaGracz3);
                if(lek<=lekka1 && cie<=ciezka1 && jaz<=jazda1){
                    printt(nazwaGracza1,": atakuje ",nazwaGracza3);
                    lekka1 -= lek;
                    ciezka1 -= cie;
                    jazda1 -= jaz;
                    sprintf(lekkaGracz1,"%d",lekka1);
                    sprintf(ciezkaGracz1,"%d",ciezka1);
                    sprintf(jazdaGracz1,"%d",jazda1);
                    //print(lekkaGracz1);
                    podniesSem(semid,0);
                    podniesSem(semid,1);
                    sleep(5);
                    opuscSem(semid,0);
                    opuscSem(semid,1);
                    double odAtak = lek*1 + cie*1.5 + jaz*3.5;
                    double doObr = lekka2*1.2 + ciezka2*3 + jazda2*1.2;
                    //atak udany
                    if(odAtak - doObr > 0){
                        double ao;
                        if(doObr>0)ao = odAtak/doObr;
                        else ao=1;
                        lek*=(int)ao;
                        cie*=(int)ao;
                        jaz*=(int)ao;
                        lekka1+=lek;
                        ciezka1+=cie;
                        jazda1+=jaz;
                        sprintf(lekkaGracz3,"%d",0);
                        sprintf(ciezkaGracz3,"%d",0);
                        sprintf(jazdaGracz3,"%d",0);
                        sprintf(lekkaGracz1,"%d",lekka1);
                        sprintf(ciezkaGracz1,"%d",ciezka1);
                        sprintf(jazdaGracz1,"%d",jazda1);
                        //wyslijWiadomosc(sygnalGracz2,"Przegrales obrone",1);
                        int pkt1 = atoi(pktGracz1);
                        pkt1 += 1;
                        sprintf(pktGracz1,"%d",pkt1);
                        //wyslijWiadomosc(sygnalGracz1,"Wygrales atak",1);
                        printd(nazwaGracza1,": Atak udany, zdobywa 1 pkt");
                        printd(nazwaGracza3,": Przegral atak");
                    } 
                    //atak nieudany
                    else {
                        double oa=doObr/odAtak;
                        double ao;
                        if(doObr>0)ao = odAtak/doObr;
                        lek*=(int)oa;
                        cie*=(int)oa;
                        jaz*=(int)oa;
                        lekka1+=lek;
                        ciezka1+=cie;
                        jazda1+=jaz;
                        lekka2*=(int)ao;
                        ciezka2*=(int)ao;
                        jazda2*=(int)ao;
                        sprintf(lekkaGracz1,"%d",lekka1);
                        sprintf(ciezkaGracz1,"%d",ciezka1);
                        sprintf(jazdaGracz1,"%d",jazda1);
                        sprintf(lekkaGracz3,"%d",lekka2);
                        sprintf(ciezkaGracz3,"%d",ciezka2);
                        sprintf(jazdaGracz3,"%d",jazda2);
                        printd(nazwaGracza1,": Atak nieudany");
                        printd(nazwaGracza3,": Atak odparty");
                        //wyslijWiadomosc(sygnalGracz1,"Atak nieudany",1);
                        //wyslijWiadomosc(sygnalGracz2,"Atak odparty",1);
                    }
                    podniesSem(semid,0);
                    podniesSem(semid,1);

                }else {
                    print("Brak surowcow");
                    podniesSem(semid,0);
                    podniesSem(semid,1);
                }
            }
        }
    }

    //gracz2 atakuje gracza1
    if(!fork()){
        while(1){
            struct Wiadomosc wiad;
            if(msgrcv(IPC_ID,&wiad,IPC_SIZE,32,0)==IPC_SIZE){
                print("Walka");
                int lek = wiad.mtext[0];
                int cie = wiad.mtext[1];
                int jaz = 0;
                opuscSem(semid,0);
                opuscSem(semid,2);
                int lekka1 = atoi(lekkaGracz2);
                int ciezka1 = atoi(ciezkaGracz2);
                int jazda1 = atoi(jazdaGracz2);
                int lekka2 = atoi(lekkaGracz1);
                int ciezka2 = atoi(ciezkaGracz1);
                int jazda2 = atoi(jazdaGracz1);
                if(lek<=lekka1 && cie<=ciezka1 && jaz<=jazda1){
                    printt(nazwaGracza2,": atakuje ",nazwaGracza1);
                    lekka1 -= lek;
                    ciezka1 -= cie;
                    jazda1 -= jaz;
                    sprintf(lekkaGracz2,"%d",lekka1);
                    sprintf(ciezkaGracz2,"%d",ciezka1);
                    sprintf(jazdaGracz2,"%d",jazda1);
                    //print(lekkaGracz1);
                    podniesSem(semid,0);
                    podniesSem(semid,2);
                    sleep(5);
                    opuscSem(semid,0);
                    opuscSem(semid,2);
                    double odAtak = lek*1 + cie*1.5 + jaz*3.5;
                    double doObr = lekka2*1.2 + ciezka2*3 + jazda2*1.2;
                    //atak udany
                    if(odAtak - doObr > 0){
                        double ao;
                        if(doObr>0)ao = odAtak/doObr;
                        else ao=1;
                        lek*=(int)ao;
                        cie*=(int)ao;
                        jaz*=(int)ao;
                        lekka1+=lek;
                        ciezka1+=cie;
                        jazda1+=jaz;
                        sprintf(lekkaGracz1,"%d",0);
                        sprintf(ciezkaGracz1,"%d",0);
                        sprintf(jazdaGracz1,"%d",0);
                        sprintf(lekkaGracz2,"%d",lekka1);
                        sprintf(ciezkaGracz2,"%d",ciezka1);
                        sprintf(jazdaGracz2,"%d",jazda1);
                        //wyslijWiadomosc(sygnalGracz2,"Przegrales obrone",1);
                        int pkt1 = atoi(pktGracz2);
                        pkt1 += 1;
                        sprintf(pktGracz2,"%d",pkt1);
                        //wyslijWiadomosc(sygnalGracz1,"Wygrales atak",1);
                        printd(nazwaGracza2,": Atak udany, zdobywa 1 pkt");
                        printd(nazwaGracza1,": Przegral atak");
                    } 
                    //atak nieudany
                    else {
                        double oa=doObr/odAtak;
                        double ao;
                        if(doObr>0)ao = odAtak/doObr;
                        lek*=(int)oa;
                        cie*=(int)oa;
                        jaz*=(int)oa;
                        lekka1+=lek;
                        ciezka1+=cie;
                        jazda1+=jaz;
                        lekka2*=(int)ao;
                        ciezka2*=(int)ao;
                        jazda2*=(int)ao;
                        sprintf(lekkaGracz1,"%d",lekka2);
                        sprintf(ciezkaGracz1,"%d",ciezka2);
                        sprintf(jazdaGracz1,"%d",jazda2);
                        sprintf(lekkaGracz2,"%d",lekka1);
                        sprintf(ciezkaGracz2,"%d",ciezka1);
                        sprintf(jazdaGracz2,"%d",jazda1);
                        printd(nazwaGracza1,": Atak nieudany");
                        printd(nazwaGracza2,": Atak odparty");
                        //wyslijWiadomosc(sygnalGracz1,"Atak nieudany",1);
                        //wyslijWiadomosc(sygnalGracz2,"Atak odparty",1);
                    }
                    podniesSem(semid,0);
                    podniesSem(semid,2);

                }else {
                    print("Brak surowcow");
                    podniesSem(semid,0);
                    podniesSem(semid,2);
                }
            }
        }
    }

    //gracz2 atakuje gracza3
    if(!fork()){
        while(1){
            struct Wiadomosc wiad;
            if(msgrcv(IPC_ID,&wiad,IPC_SIZE,33,0)==IPC_SIZE){
                print("Walka");
                int lek = wiad.mtext[0];
                int cie = wiad.mtext[1];
                int jaz = 0;
                opuscSem(semid,2);
                opuscSem(semid,1);
                int lekka1 = atoi(lekkaGracz2);
                int ciezka1 = atoi(ciezkaGracz2);
                int jazda1 = atoi(jazdaGracz2);
                int lekka2 = atoi(lekkaGracz3);
                int ciezka2 = atoi(ciezkaGracz3);
                int jazda2 = atoi(jazdaGracz3);
                if(lek<=lekka1 && cie<=ciezka1 && jaz<=jazda1){
                    printt(nazwaGracza2,": atakuje ",nazwaGracza3);
                    lekka1 -= lek;
                    ciezka1 -= cie;
                    jazda1 -= jaz;
                    sprintf(lekkaGracz2,"%d",lekka1);
                    sprintf(ciezkaGracz2,"%d",ciezka1);
                    sprintf(jazdaGracz2,"%d",jazda1);
                    //print(lekkaGracz1);
                    podniesSem(semid,2);
                    podniesSem(semid,1);
                    sleep(5);
                    opuscSem(semid,2);
                    opuscSem(semid,1);
                    double odAtak = lek*1 + cie*1.5 + jaz*3.5;
                    double doObr = lekka2*1.2 + ciezka2*3 + jazda2*1.2;
                    //atak udany
                    if(odAtak - doObr > 0){
                        double ao;
                        if(doObr>0)ao = odAtak/doObr;
                        else ao=1;
                        lek*=(int)ao;
                        cie*=(int)ao;
                        jaz*=(int)ao;
                        lekka1+=lek;
                        ciezka1+=cie;
                        jazda1+=jaz;
                        sprintf(lekkaGracz3,"%d",0);
                        sprintf(ciezkaGracz3,"%d",0);
                        sprintf(jazdaGracz3,"%d",0);
                        sprintf(lekkaGracz2,"%d",lekka1);
                        sprintf(ciezkaGracz2,"%d",ciezka1);
                        sprintf(jazdaGracz2,"%d",jazda1);
                        //wyslijWiadomosc(sygnalGracz2,"Przegrales obrone",1);
                        int pkt1 = atoi(pktGracz2);
                        pkt1 += 1;
                        sprintf(pktGracz2,"%d",pkt1);
                        //wyslijWiadomosc(sygnalGracz1,"Wygrales atak",1);
                        printd(nazwaGracza2,": Atak udany, zdobywa 1 pkt");
                        printd(nazwaGracza3,": Przegral atak");
                    } 
                    //atak nieudany
                    else {
                        double oa=doObr/odAtak;
                        double ao;
                        if(doObr>0)ao = odAtak/doObr;
                        lek*=(int)oa;
                        cie*=(int)oa;
                        jaz*=(int)oa;
                        lekka1+=lek;
                        ciezka1+=cie;
                        jazda1+=jaz;
                        lekka2*=(int)ao;
                        ciezka2*=(int)ao;
                        jazda2*=(int)ao;
                        sprintf(lekkaGracz2,"%d",lekka1);
                        sprintf(ciezkaGracz2,"%d",ciezka1);
                        sprintf(jazdaGracz2,"%d",jazda1);
                        sprintf(lekkaGracz3,"%d",lekka2);
                        sprintf(ciezkaGracz3,"%d",ciezka2);
                        sprintf(jazdaGracz3,"%d",jazda2);
                        printd(nazwaGracza1,": Atak nieudany");
                        printd(nazwaGracza3,": Atak odparty");
                        //wyslijWiadomosc(sygnalGracz1,"Atak nieudany",1);
                        //wyslijWiadomosc(sygnalGracz2,"Atak odparty",1);
                    }
                    podniesSem(semid,2);
                    podniesSem(semid,1);

                }else {
                    print("nieatakuje");
                    podniesSem(semid,2);
                    podniesSem(semid,1);
                }
            }
        }
    }
    //gracz3 atakuje gracza1
    if(!fork()){
        while(1){
            struct Wiadomosc wiad;
            if(msgrcv(IPC_ID,&wiad,IPC_SIZE,34,0)==IPC_SIZE){
                print("Walka");
                int lek = wiad.mtext[0];
                int cie = wiad.mtext[1];
                int jaz = 0;
                opuscSem(semid,0);
                opuscSem(semid,2);
                int lekka1 = atoi(lekkaGracz3);
                int ciezka1 = atoi(ciezkaGracz3);
                int jazda1 = atoi(jazdaGracz3);
                int lekka2 = atoi(lekkaGracz1);
                int ciezka2 = atoi(ciezkaGracz1);
                int jazda2 = atoi(jazdaGracz1);
                if(lek<=lekka1 && cie<=ciezka1 && jaz<=jazda1){
                    printt(nazwaGracza3,": atakuje ",nazwaGracza1);
                    lekka1 -= lek;
                    ciezka1 -= cie;
                    jazda1 -= jaz;
                    sprintf(lekkaGracz3,"%d",lekka1);
                    sprintf(ciezkaGracz3,"%d",ciezka1);
                    sprintf(jazdaGracz3,"%d",jazda1);
                    //print(lekkaGracz1);
                    podniesSem(semid,0);
                    podniesSem(semid,2);
                    sleep(5);
                    opuscSem(semid,0);
                    opuscSem(semid,2);
                    double odAtak = lek*1 + cie*1.5 + jaz*3.5;
                    double doObr = lekka2*1.2 + ciezka2*3 + jazda2*1.2;
                    //atak udany
                    if(odAtak - doObr > 0){
                        double ao;
                        if(doObr>0)ao = odAtak/doObr;
                        else ao=1;
                        lek*=(int)ao;
                        cie*=(int)ao;
                        jaz*=(int)ao;
                        lekka1+=lek;
                        ciezka1+=cie;
                        jazda1+=jaz;
                        sprintf(lekkaGracz1,"%d",0);
                        sprintf(ciezkaGracz1,"%d",0);
                        sprintf(jazdaGracz1,"%d",0);
                        sprintf(lekkaGracz3,"%d",lekka1);
                        sprintf(ciezkaGracz3,"%d",ciezka1);
                        sprintf(jazdaGracz3,"%d",jazda1);
                        //wyslijWiadomosc(sygnalGracz2,"Przegrales obrone",1);
                        int pkt1 = atoi(pktGracz3);
                        pkt1 += 1;
                        sprintf(pktGracz3,"%d",pkt1);
                        //wyslijWiadomosc(sygnalGracz1,"Wygrales atak",1);
                        printd(nazwaGracza3,": Atak udany, zdobywa 1 pkt");
                        printd(nazwaGracza1,": Przegral atak");
                    } 
                    //atak nieudany
                    else {
                        double ao;
                        if(doObr>0)ao = odAtak/doObr;
                        double oa=doObr/odAtak;
                        lek*=(int)oa;
                        cie*=(int)oa;
                        jaz*=(int)oa;
                        lekka1+=lek;
                        ciezka1+=cie;
                        jazda1+=jaz;
                        lekka2*=(int)ao;
                        ciezka2*=(int)ao;
                        jazda2*=(int)ao;
                        sprintf(lekkaGracz1,"%d",lekka2);
                        sprintf(ciezkaGracz1,"%d",ciezka2);
                        sprintf(jazdaGracz1,"%d",jazda2);
                        sprintf(lekkaGracz3,"%d",lekka1);
                        sprintf(ciezkaGracz3,"%d",ciezka1);
                        sprintf(jazdaGracz3,"%d",jazda1);
                        printd(nazwaGracza1,": Atak nieudany");
                        printd(nazwaGracza2,": Atak odparty");
                        //wyslijWiadomosc(sygnalGracz1,"Atak nieudany",1);
                        //wyslijWiadomosc(sygnalGracz2,"Atak odparty",1);
                    }
                    podniesSem(semid,0);
                    podniesSem(semid,2);

                }else {
                    print("nieatakuje");
                    podniesSem(semid,0);
                    podniesSem(semid,2);
                }
            }
        }
    }

    //gracz3 atakuje gracza2
    if(!fork()){
        while(1){
            struct Wiadomosc wiad;
            if(msgrcv(IPC_ID,&wiad,IPC_SIZE,35,0)==IPC_SIZE){
                print("Walka");
                int lek = wiad.mtext[0];
                int cie = wiad.mtext[1];
                int jaz = 0;
                opuscSem(semid,2);
                opuscSem(semid,1);
                int lekka1 = atoi(lekkaGracz3);
                int ciezka1 = atoi(ciezkaGracz3);
                int jazda1 = atoi(jazdaGracz3);
                int lekka2 = atoi(lekkaGracz2);
                int ciezka2 = atoi(ciezkaGracz2);
                int jazda2 = atoi(jazdaGracz2);
                if(lek<=lekka1 && cie<=ciezka1 && jaz<=jazda1){
                    printt(nazwaGracza3,": atakuje ",nazwaGracza2);
                    lekka1 -= lek;
                    ciezka1 -= cie;
                    jazda1 -= jaz;
                    sprintf(lekkaGracz3,"%d",lekka1);
                    sprintf(ciezkaGracz3,"%d",ciezka1);
                    sprintf(jazdaGracz3,"%d",jazda1);
                    //print(lekkaGracz1);
                    podniesSem(semid,2);
                    podniesSem(semid,1);
                    sleep(5);
                    opuscSem(semid,2);
                    opuscSem(semid,1);
                    double odAtak = lek*1 + cie*1.5 + jaz*3.5;
                    double doObr = lekka2*1.2 + ciezka2*3 + jazda2*1.2;
                    //atak udany
                    if(odAtak - doObr > 0){
                        double ao;
                        if(doObr>0)ao = odAtak/doObr;
                        else ao=1;
                        lek*=(int)ao;
                        cie*=(int)ao;
                        jaz*=(int)ao;
                        lekka1+=lek;
                        ciezka1+=cie;
                        jazda1+=jaz;
                        sprintf(lekkaGracz2,"%d",0);
                        sprintf(ciezkaGracz2,"%d",0);
                        sprintf(jazdaGracz2,"%d",0);
                        sprintf(lekkaGracz3,"%d",lekka1);
                        sprintf(ciezkaGracz3,"%d",ciezka1);
                        sprintf(jazdaGracz3,"%d",jazda1);
                        //wyslijWiadomosc(sygnalGracz2,"Przegrales obrone",1);
                        int pkt1 = atoi(pktGracz3);
                        pkt1 += 1;
                        sprintf(pktGracz3,"%d",pkt1);
                        //wyslijWiadomosc(sygnalGracz1,"Wygrales atak",1);
                        printd(nazwaGracza3,": Atak udany, zdobywa 1 pkt");
                        printd(nazwaGracza2,": Przegral atak");
                    } 
                    //atak nieudany
                    else {
                        double oa=doObr/odAtak;
                        double ao;
                        if(doObr>0)ao = odAtak/doObr;
                        lek*=(int)oa;
                        cie*=(int)oa;
                        jaz*=(int)oa;
                        lekka1+=lek;
                        ciezka1+=cie;
                        jazda1+=jaz;
                        lekka2*=(int)ao;
                        ciezka2*=(int)ao;
                        jazda2*=(int)ao;
                        sprintf(lekkaGracz3,"%d",lekka1);
                        sprintf(ciezkaGracz3,"%d",ciezka1);
                        sprintf(jazdaGracz3,"%d",jazda1);
                        sprintf(lekkaGracz2,"%d",lekka2);
                        sprintf(ciezkaGracz2,"%d",ciezka2);
                        sprintf(jazdaGracz2,"%d",jazda2);
                        printd(nazwaGracza1,": Atak nieudany");
                        printd(nazwaGracza3,": Atak odparty");
                        //wyslijWiadomosc(sygnalGracz1,"Atak nieudany",1);
                        //wyslijWiadomosc(sygnalGracz2,"Atak odparty",1);
                    }
                    podniesSem(semid,2);
                    podniesSem(semid,1);

                }else {
                    print("Brak surowcow");
                    podniesSem(semid,2);
                    podniesSem(semid,1);
                }
            }
        }
    }
//-----STAN JEDNOSTEK
    if(!fork()){
        while(1){
            struct Wiadomosc wiad;
            if(msgrcv(IPC_ID,&wiad,IPC_SIZE,51,0)==IPC_SIZE){
                wyslijWiadomosc(sygnalGracz1,pktGracz1,0);
                wyslijWiadomosc(sygnalGracz1,lekkaGracz1,0);
                wyslijWiadomosc(sygnalGracz1,ciezkaGracz1,0);
                wyslijWiadomosc(sygnalGracz1,jazdaGracz1,0);
                wyslijWiadomosc(sygnalGracz1,roboGracz1,0);
                wyslijWiadomosc(sygnalGracz1,surowceGracz1,0);
                wyslijWiadomosc(sygnalGracz1,pktGracz2,0);
                wyslijWiadomosc(sygnalGracz1,pktGracz3,0);
                
            }
        }
    }
    if(!fork()){
        while(1){
            struct Wiadomosc wiad;
            if(msgrcv(IPC_ID,&wiad,IPC_SIZE,52,0)==IPC_SIZE){
                wyslijWiadomosc(sygnalGracz2,pktGracz2,0);
                wyslijWiadomosc(sygnalGracz2,lekkaGracz2,1);
                wyslijWiadomosc(sygnalGracz2,ciezkaGracz2,1);
                wyslijWiadomosc(sygnalGracz2,jazdaGracz2,1);
                wyslijWiadomosc(sygnalGracz2,roboGracz2,1);
                wyslijWiadomosc(sygnalGracz2,surowceGracz2,1);
                wyslijWiadomosc(sygnalGracz2,pktGracz1,0);
                wyslijWiadomosc(sygnalGracz2,pktGracz3,0);
            }
        }
    }
    if(!fork()){
        while(1){
            struct Wiadomosc wiad;
            if(msgrcv(IPC_ID,&wiad,IPC_SIZE,53,0)==IPC_SIZE){
                wyslijWiadomosc(sygnalGracz3,pktGracz3,0);
                wyslijWiadomosc(sygnalGracz3,lekkaGracz3,0);
                wyslijWiadomosc(sygnalGracz3,ciezkaGracz3,1);
                wyslijWiadomosc(sygnalGracz3,jazdaGracz3,1);
                wyslijWiadomosc(sygnalGracz3,roboGracz3,1);
                wyslijWiadomosc(sygnalGracz3,surowceGracz3,1);
                wyslijWiadomosc(sygnalGracz3,pktGracz1,0);
                wyslijWiadomosc(sygnalGracz3,pktGracz2,0);
            }
        }
    }

    if(!fork()){
        while(1){
            opuscSem(semid,0);
            opuscSem(semid,1);
            opuscSem(semid,2);
            if(strcmp(pktGracz1,"5")==0){
                printd("WYGRANA: ",nazwaGracza1);
            }else if(strcmp(pktGracz2,"5")==0){
                printd("WYGRANA: ",nazwaGracza2);
            }else if(strcmp(pktGracz2,"5")==0){
                printd("WYGRANA: ",nazwaGracza3);
            }
            podniesSem(semid,0);
            podniesSem(semid,1);
            podniesSem(semid,2);
        }
        exit(0);
    }

    while(1);
    print("Zamkykanie serwera...");
    sleep(2);
    endwin();
}