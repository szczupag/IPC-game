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
#define trening 4
#define sygnalStan 9
#define sygnalSerwer 20
#define atakoddo 29
#define surow 40
#define wynik 50

struct Wiadomosc msg;
int IPC_ID;
static struct sembuf buf;

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

typedef struct Wiadomosc{
    long mtype;
    char mtext[IPC_SIZE/2];
    int mnum;
}Wiadomosc;

typedef struct Walka{
    long mtype;
    int units[3];
}Walka;

void wyslijWiadomosc(int signal, char text[IPC_SIZE], int num){
    msg.mtype = signal;
    strcpy(msg.mtext,text);
    msg.mnum = num;
    msgsnd(IPC_ID,&msg,IPC_SIZE,0);
}

void lekkatrening(int nrgracza,int ile){
    if(ile > 0) wyslijWiadomosc(trening + nrgracza, "lekka", ile);
}

void ciezkatrening(int nrgracza,int ile){
    if(ile > 0) wyslijWiadomosc(trening + nrgracza, "ciezka", ile);
}

void jazdatrening(int nrgracza,int ile){
    if(ile > 0) wyslijWiadomosc(trening + nrgracza, "jazda", ile);
}

void robotrening(int nrgracza,int ile){
    if(ile > 0) wyslijWiadomosc(trening + nrgracza, "robotnicy", ile);
}

void atak(int nrgracza, int przeciwnik, int ileL, int ileC, int ileJ){
    struct Walka walka;
    int type = atakoddo+nrgracza+przeciwnik;
    char text[4]={ileL,ileC,ileJ,'0'};
    wyslijWiadomosc(type,text,0);
    //msgsnd(IPC_ID,&walka,IPC_SIZE,0);

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

int main()
{
//-----INTERFEJS
    initscr();
    start_color();
    init_pair(1,COLOR_BLACK,COLOR_CYAN);
    init_pair(2,COLOR_WHITE,COLOR_BLACK);

    int y,x,begY,begX,maxY,maxX;
    getyx(stdscr,y,x);
    getbegyx(stdscr,begY,begX);
    getmaxyx(stdscr,maxY,maxX);

    WINDOW * win = newwin(3,maxX,0,0);
    bkgd(COLOR_PAIR(1));
    WINDOW * login = newwin(3,maxX/2,maxY/2-3,maxX/2-maxX/4);
    WINDOW * kom = newwin(50,2*maxX/3,maxY/2,maxX/2-maxX/4);
    wbkgd(kom,COLOR_PAIR(1));

    refresh();
    wrefresh(win);
    wrefresh(login);
    wrefresh(kom);
    keypad(login,true);

    mvwprintw(win,1,maxX/2-4.5,"LOGOWANIE");
    mvwprintw(login,1,1,"Nazwa gracza: ");
    wrefresh(win);
    wrefresh(login);

//-----LOGOWANIE
    IPC_ID = msgget(IPC_id,IPC_CREAT|0600);
    int idGracza = 0;
    char nazwaGracza[IPC_SIZE]="";
    int nid=0;
    char a;
    do{
        a = wgetch(login);
        if(isalnum(a)){
            nazwaGracza[nid]=a;
            nid++;
        }
        else if(a=='\n'&&nid>0)break;
    }while(1);
    mvwprintw(kom,1,1,"Witaj ");
    mvwprintw(kom,1,7,nazwaGracza);
    wrefresh(kom);
    wyslijWiadomosc(graczGotowy,nazwaGracza,1);
    mvwprintw(kom,2,1,"Czekam na serwer");
    wrefresh(kom);
    msgrcv(IPC_ID,&msg,IPC_SIZE,serwerGotowy,0);
    wyslijWiadomosc(serwerGotowy,"1",1);
    mvwprintw(kom,3,1,"Polaczono");
    wrefresh(kom);

//-----ID
    msgrcv(IPC_ID,&msg,IPC_SIZE,sygnalSerwer,0);
    idGracza = msg.mnum;
    char idCh[1]="";
    sprintf(idCh,"%d",idGracza);
    mvwprintw(kom,4,1,"Jestes graczem nr ");
    mvwprintw(kom,4,19,idCh);
    wrefresh(kom);

//-----PAMIEC SEMAFOR
    int shmemory = shmget(SHM_id - idGracza, SHM_SIZE, IPC_CREAT|0600);
    char* koniec = shmat(shmemory,NULL,0);
    int semid = semget(556+idGracza,2,IPC_CREAT|0600);
    semctl(semid,0,SETVAL,1);
    semctl(semid,1,SETVAL,1);
    opuscSem(semid,0);
    opuscSem(semid,1);

    mvwprintw(kom,5,1,"Oczekiwanie na pozostalych graczy...");
    wrefresh(kom);
    char nazwaPrzeciwnika1[IPC_SIZE]="",nazwaPrzeciwnika2[IPC_SIZE]="";
    msgrcv(IPC_ID,&msg,IPC_SIZE,idGracza,0);
    strcpy(nazwaPrzeciwnika1,msg.mtext);
    msgrcv(IPC_ID,&msg,IPC_SIZE,idGracza,0);
    strcpy(nazwaPrzeciwnika2,msg.mtext);
    mvwprintw(kom,6,1,"Twoi przeciwnicy:woi przeciwnicy:");
    mvwprintw(kom,7,1,nazwaPrzeciwnika1);
    mvwprintw(kom,8,1,nazwaPrzeciwnika2);
    wrefresh(kom);
    podniesSem(semid,0); 

//----ROZGRYWKA
    wclear(kom);
    wclear(login);
    wrefresh(kom);
    wrefresh(login);
    delwin(kom);
    delwin(login);
    bkgd(COLOR_PAIR(1));
    mvwprintw(win,1,maxX/2-4.5,"ROZGRYWKA");
    wbkgd(win,COLOR_PAIR(2));
    refresh();
    wrefresh(win);
    //1kolumna
    WINDOW * stanBox = newwin((maxY-8)/2,(maxX-4)/3,4,1);
    //2kolumna
    WINDOW * menuBox = newwin((maxY-8)/2,(maxX-4)/3,4,2+(maxX-4)/3);
    WINDOW * msgBox = newwin((maxY-8)/2,1+2*(maxX-4)/3,5+(maxY-8)/2,1);
    //3 kolumna
    WINDOW * wynikBox = newwin((maxY-8)/2,(maxX-4)/3,4,3+2*(maxX-4)/3);
    WINDOW * graczeBox = newwin((maxY-8)/2,(maxX-4)/3,5+(maxY-8)/2,3+2*(maxX-4)/3);

    //mvwprintw(msgBox,0,1," Informacje ");
    mvwprintw(msgBox,2,1,"                 |  cena  |  atak |  obrona  | czas ");
    mvwprintw(msgBox,3,1," Lekka piechota  |  100   |  1    |  1.2     | 2");
    mvwprintw(msgBox,4,1," Ciezka piechota |  250   |  1.5  |  3       | 3");
    mvwprintw(msgBox,5,1," Jazda           |  550   |  3.5  |  1.2     | 5");
    mvwprintw(msgBox,6,1," Robotnik        |  150   |  0    |  0       | 2");
    mvwprintw(stanBox,0,1," Stan surowcow ");
    mvwprintw(stanBox,2,1,"lekka piechota ");
    mvwprintw(stanBox,3,1,"ciezka piechota ");
    mvwprintw(stanBox,4,1,"jazda ");
    mvwprintw(stanBox,5,1,"robotnicy ");
    mvwprintw(stanBox,6,1,"surowce ");
    mvwprintw(wynikBox,0,1," Wynik ");
    mvwprintw(wynikBox,2,2,"0");
    mvwprintw(graczeBox,0,1," Przeciwnicy ");
    mvwprintw(graczeBox,2,2,nazwaPrzeciwnika1);
    mvwprintw(graczeBox,3,2,nazwaPrzeciwnika2);
    refresh();
    wrefresh(msgBox);
    wrefresh(stanBox);
    wrefresh(wynikBox);
    wrefresh(graczeBox);
    char *choices[5]={"LEKKA PIECHOTA","CIEZKA PIECHOTA","JAZDA","ROBOTNIK","ATAK"};
    char *submenu[2]={"POWROT DO MENU","TRENUJ"};
    char *przeciw[2]={nazwaPrzeciwnika1,nazwaPrzeciwnika2};
    char lekka[IPC_SIZE]="0",ciezka[IPC_SIZE]="0",jazda[IPC_SIZE]="0",robo[IPC_SIZE]="0",suro[IPC_SIZE]="300";
    
//-----MENU
    if(!fork()){
        while(1){
            mvwprintw(menuBox,0,1," Menu ");
            wrefresh(menuBox);
            
            keypad(menuBox,true);
            int choice,i,chi;
            int highL=0,ileL=0,ileC=0,ileJ=0;
            char ile;
            while(1){
                
                struct Wiadomosc wiad;
                wyslijWiadomosc(50+idGracza,"",0);
                msgrcv(IPC_ID,&wiad,IPC_SIZE,idGracza,0);
                mvwprintw(wynikBox,2,2,wiad.mtext);
                msgrcv(IPC_ID,&wiad,IPC_SIZE,idGracza,0);
                mvwprintw(stanBox,2,18,wiad.mtext);
                msgrcv(IPC_ID,&wiad,IPC_SIZE,idGracza,0);
                mvwprintw(stanBox,3,18,wiad.mtext);
                msgrcv(IPC_ID,&wiad,IPC_SIZE,idGracza,0);
                mvwprintw(stanBox,4,18,wiad.mtext);
                msgrcv(IPC_ID,&wiad,IPC_SIZE,idGracza,0);
                mvwprintw(stanBox,5,18,wiad.mtext);
                msgrcv(IPC_ID,&wiad,IPC_SIZE,idGracza,0);
                mvwprintw(stanBox,6,18,wiad.mtext);
                msgrcv(IPC_ID,&wiad,IPC_SIZE,idGracza,0);
                mvwprintw(graczeBox,2,10,wiad.mtext);
                msgrcv(IPC_ID,&wiad,IPC_SIZE,idGracza,0);
                mvwprintw(graczeBox,3,10,wiad.mtext);
                refresh();
                wrefresh(stanBox);
                wrefresh(wynikBox);

                for(i=0; i<5; i++){
                    if(i==highL)
                        wattron(menuBox,A_REVERSE);
                    mvwprintw(menuBox,i+2,1,choices[i]);
                    wattroff(menuBox,A_REVERSE);
                }
                choice = wgetch(menuBox);
                if(choice == KEY_UP){
                    highL--;
                    if(highL == -1)highL=0;
                }
                else if(choice == KEY_DOWN){
                    highL++;
                    if(highL == 5)highL=4;
                }else if(choice == '\n')break;
            }
            switch(highL){
                case 0: 
                    wclear(menuBox);
                    mvwprintw(menuBox,0,1," Lekka piechota ");
                    wrefresh(menuBox);
                    highL=0;
                    while(1){
                        for(i=0;i<2;i++){
                            if(i==highL)
                                wattron(menuBox,A_REVERSE);
                            mvwprintw(menuBox,i+2,1,submenu[i]);
                            wattroff(menuBox,A_REVERSE);
                        }
                        choice = wgetch(menuBox);
                        if(choice == KEY_UP){
                            highL--;
                            if(highL == -1)highL=0;
                        }
                        else if(choice == KEY_DOWN){
                            highL++;
                            if(highL == 2)highL=1;
                        }else if(choice == '\n')break;
                    }
                    if(highL==1){
                        wclear(menuBox);
                        mvwprintw(menuBox,0,1," Lekka piechota ");
                        mvwprintw(menuBox,2,1,"ile jendostek?:\n");
                        ile=wgetch(menuBox);
                        chi=ile-'0';
                        lekkatrening(idGracza,chi);
                    }
                    break;
                case 1: 
                    wclear(menuBox);
                    mvwprintw(menuBox,0,1," Ciezka piechota ");
                    wrefresh(menuBox);
                    highL=0;
                    while(1){
                        for(i=0;i<2;i++){
                            if(i==highL)
                                wattron(menuBox,A_REVERSE);
                            mvwprintw(menuBox,i+2,1,submenu[i]);
                            wattroff(menuBox,A_REVERSE);
                        }
                        choice = wgetch(menuBox);
                        if(choice == KEY_UP){
                            highL--;
                            if(highL == -1)highL=0;
                        }
                        else if(choice == KEY_DOWN){
                            highL++;
                            if(highL == 2)highL=1;
                        }else if(choice == '\n')break;
                    }
                    if(highL==1){
                        wclear(menuBox);
                        mvwprintw(menuBox,0,1," Ciezka piechota ");
                        mvwprintw(menuBox,2,1,"ile jendostek?:\n");
                        ile=wgetch(menuBox);
                        chi=ile-'0';
                        ciezkatrening(idGracza,chi);
                    }
                    break;
                case 2: 
                    wclear(menuBox);
                    mvwprintw(menuBox,0,1," Jazda ");
                    wrefresh(menuBox);
                    highL=0;
                    while(1){
                        for(i=0;i<2;i++){
                            if(i==highL)
                                wattron(menuBox,A_REVERSE);
                            mvwprintw(menuBox,i+2,1,submenu[i]);
                            wattroff(menuBox,A_REVERSE);
                        }
                        choice = wgetch(menuBox);
                        if(choice == KEY_UP){
                            highL--;
                            if(highL == -1)highL=0;
                        }
                        else if(choice == KEY_DOWN){
                            highL++;
                            if(highL == 2)highL=1;
                        }else if(choice == '\n')break;
                    }
                    if(highL==1){
                        wclear(menuBox);
                        mvwprintw(menuBox,0,1," Jazda ");
                        mvwprintw(menuBox,2,1,"ile jendostek?:\n");
                        ile=wgetch(menuBox);
                        chi=ile-'0';
                        jazdatrening(idGracza,chi);
                    }
                    break;
                case 3: 
                    wclear(menuBox);
                    mvwprintw(menuBox,0,1," Robotnicy ");
                    wrefresh(menuBox);
                    highL=0;
                    while(1){
                        for(i=0;i<2;i++){
                            if(i==highL)
                                wattron(menuBox,A_REVERSE);
                            mvwprintw(menuBox,i+2,1,submenu[i]);
                            wattroff(menuBox,A_REVERSE);
                        }
                        choice = wgetch(menuBox);
                        if(choice == KEY_UP){
                            highL--;
                            if(highL == -1)highL=0;
                        }
                        else if(choice == KEY_DOWN){
                            highL++;
                            if(highL == 2)highL=1;
                        }else if(choice == '\n')break;
                    }
                    if(highL==1){
                        wclear(menuBox);
                        mvwprintw(menuBox,0,1," Robotnicy ");
                        mvwprintw(menuBox,2,1,"ile jendostek?:\n");
                        ile=wgetch(menuBox);
                        chi=ile-'0';
                        robotrening(idGracza,chi);
                    }
                    break;
                case 4:
                    wclear(menuBox);
                    mvwprintw(menuBox,0,1," Atak ");
                    wrefresh(menuBox);
                    highL=0;
                    while(1){
                        for(i=0;i<2;i++){
                            if(i==highL)
                                wattron(menuBox,A_REVERSE);
                            mvwprintw(menuBox,i+2,1,przeciw[i]);
                            wattroff(menuBox,A_REVERSE);
                        }
                        choice = wgetch(menuBox);
                        if(choice == KEY_UP){
                            highL--;
                            if(highL == -1)highL=0;
                        }
                        else if(choice == KEY_DOWN){
                            highL++;
                            if(highL == 2)highL=1;
                        }else if(choice == '\n')break;
                    }
                    wclear(menuBox);
                    mvwprintw(menuBox,0,1," Atak ");
                    mvwprintw(menuBox,2,1,"ile lekkiej?: ");
                    ile=wgetch(menuBox);
                    ileL=ile-'0';
                    mvwprintw(menuBox,3,1,"ile ciezkiej?: ");
                    ile=wgetch(menuBox);
                    ileC=ile-'0';
                    mvwprintw(menuBox,4,1,"ile jazdy?: ");
                    ile=wgetch(menuBox);
                    ileJ=ile-'0';
                    //mvwprintw(menuBox,5,1,"press any key");
                    wgetch(menuBox);
                    atak(idGracza,highL,ileL,ileC,ileJ);
                    break;

                default:
                    break;
            }
            wclear(menuBox);

        }
    }

    

    wait(NULL); 
}