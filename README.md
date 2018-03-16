# IPC-game
Simple battle game using inter proccess communication on linux system

ipcrm --all 
gcc klient1.c -o kli -lncurses
./kli
gcc serwer.c -o ser -lncurses
./ser
