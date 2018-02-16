// fernando gonzalez
//giuli latella

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <unistd.h>
#include <string.h>
#include <linux/stat.h>
#include <signal.h>
#include "chat.h"
#define Red_Fifo "FifoW"
#define Blue_Fifo "FifoR"
#define Green_Fifo "/tmp/servidor"
#define ALTO 5 // Alto de la ventana 2
#define LINES_MIN 10 // Alto mínimo que debe tener el terminal
#define COLS_MIN 25 // Ancho mínimo que debe tener el terminal
#define TECLA_RETURN 0xD
#define TAM 2048 // Tamaño de buffer

int salida;

void quit()
{
    salida = 1;
}

WINDOW *ventana1, *ventana2;

int main(int argc, char *argv[])
{
    int FilePipe;
    int FilePipeR;
    int FilePipeW;
    char Buffer[2048];
    char input [2048];                  
    char WriteBuf[2048];
    //char contact[40];
    char *link;                 
    char *nameUser;                     
    char *PipeRead;                     
    char *PipeWrite;
    char *PipeLink;     

    char *path = getenv("PWD");
    char *hostname = getenv("USER");
    const char space[2] = " ";
    char *token;
    char *token2;
    char *auxPath;
    int ch;

    //apartando espacio en memoria para el nombre del usuario y de los
    //pipes de lectura y escritura segun sea el caso del nombre de usuario
    //por argumento de entrada o por defecto en nombre del usuario del equipo...
    auxPath = (char*) malloc(strlen(path) + strlen("/"));
    strcat(auxPath,path);
    strcat(auxPath,"/");
    if (argc==2)
    {

        nameUser = (char*) malloc(strlen(argv[1])* sizeof(char));
        PipeRead = (char*) malloc((strlen(argv[1]) + strlen(Blue_Fifo) + strlen(auxPath) +1)* sizeof(char));
        PipeWrite = (char*) malloc((strlen(argv[1]) + strlen(Red_Fifo)+ strlen(auxPath) +1)* sizeof(char));
        PipeLink = (char*) malloc(strlen(Green_Fifo)* sizeof(char));
        strcpy(nameUser,argv[1]);
        strcpy(PipeLink, Green_Fifo);
    }
    else if(argc == 1)
    {
        nameUser = (char*) malloc(strlen(hostname)* sizeof(char));
        PipeRead= (char*) malloc((strlen(hostname) + strlen(Blue_Fifo)+ strlen(auxPath) +1)* sizeof(char));
        PipeWrite= (char*) malloc((strlen(hostname) + strlen(Red_Fifo)+ strlen(auxPath) +1)* sizeof(char));
        PipeLink = (char*) malloc((strlen(Green_Fifo)* sizeof(char)));
        strcpy(nameUser,hostname);
        strcpy(PipeLink, Green_Fifo);
    }
    else if (argc == 3)
    {
        
        if (strcmp(argv[1], "-p"))
        {
            printf("Entrada erronea...\n");         
        }
        else
        {
            //printf("HERE2\n");
            nameUser = (char*) malloc(strlen(hostname)* sizeof(char));
            PipeRead= (char*) malloc((strlen(hostname) + strlen(Blue_Fifo)+ strlen(auxPath) +1)* sizeof(char));
            PipeWrite= (char*) malloc((strlen(hostname) + strlen(Red_Fifo)+ strlen(auxPath) +1)* sizeof(char));
            PipeLink = (char*) malloc(strlen(argv[2])* sizeof(char));
            strcpy(nameUser,hostname);
            strcpy(PipeLink, argv[2]);
            //cambia nombre pipe server 
        }
    }
    else if (argc==4)
    {
        
        if (strcmp(argv[1], "-p"))
        {
            printf("Entrada erronea...\n");         
        }
        else
        {
        nameUser = (char*) malloc(strlen(argv[3])* sizeof(char));
        PipeRead= (char*) malloc((strlen(argv[3]) + strlen(Blue_Fifo)+ strlen(auxPath) +1)* sizeof(char));
        PipeWrite= (char*) malloc((strlen(argv[3]) + strlen(Red_Fifo)+ strlen(auxPath) +1)* sizeof(char));
        PipeLink = (char*) malloc(strlen(argv[2])* sizeof(char));
        strcpy(nameUser,argv[3]);
        strcpy(PipeLink, argv[2]);
        }       
    }
    else{
        printf("Entrada erronea...\n");
    }


    strcpy(PipeRead,auxPath);
    strcat(PipeRead,nameUser);
    strcat(PipeRead,Blue_Fifo);
    strcpy(PipeWrite,auxPath);
    strcat(PipeWrite,nameUser);
    strcat(PipeWrite,Red_Fifo);
    unlink(PipeWrite);
    unlink(PipeRead);
    mknod(PipeRead, S_IFIFO,0);
    chmod(PipeRead, 0660);
    FilePipeR = open(PipeRead,O_RDONLY | O_NONBLOCK);
    mknod(PipeWrite, S_IFIFO,0);
    chmod(PipeWrite, 0660);
    FilePipe = -1;
    while (FilePipe == -1)
    {
        FilePipe = open(PipeLink, O_WRONLY | O_NDELAY);      
    }
    link = (char*) malloc(2048 * sizeof(char));
    sprintf(link, "%s %s %s", nameUser, PipeWrite, PipeRead);
    write(FilePipe,link, strlen(link)+1);
    close(FilePipe);


    FilePipeW = -1;
    while (FilePipeW == -1)
    {
        FilePipeW = open(PipeWrite, O_WRONLY | O_NDELAY);
    }
    signal(SIGINT, quit);

    initscr(); // Inicializar la biblioteca ncurses
    
    if (LINES < LINES_MIN || COLS < COLS_MIN) {
        endwin(); // Restaurar la operación del terminal a modo normal
        printf("El terminal es muy pequeño para correr este programa.\n");
        exit(0);
    }

    // Opciones de la biblioteca ncurses
    cbreak();
    nonl();
    //keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    int alto1 = LINES - ALTO; // Alto de la ventana 1
    ventana1 = newwin(alto1, 0, 0, 0); // Crear la ventana 1
    ventana2 = newwin(ALTO, 0, alto1, 0); // Crear la ventana 2
    scrollok(ventana1, TRUE); //Activar el corrimiento automático en la ventana 1
    scrollok(ventana2, TRUE);
    limpiarVentana2(); // Dibujar la línea horizontal

    while(1) {

        if (read(FilePipeR, Buffer, 2048) > 0)
                { 
                    
                    token2 = strtok(Buffer, space);
                    if (!strcmp(token2, "#$CODELOGERROR"))
                    {
                        token2 = strtok(NULL, space);
                        wprintw(ventana1,"%s", token2 );
                        wrefresh(ventana1);
                        salida = 1;
                        continue;
                    }

///////////////////////////////////////////////////////////////////
                    wprintw(ventana1,"%s",token2);
                    wprintw(ventana1, ": ");
                    wrefresh(ventana1);
                    limpiarVentana2();
                    enfocarVentana2();


                    token2 = strtok(NULL, space);
                    while (token2 != NULL)
                    {
                        if (token2 != NULL)
                        {
                            wprintw(ventana1,"%s",token2);
                            wprintw(ventana1, " ");
                            wrefresh(ventana1);
                            enfocarVentana2();

                        }
                        token2 = strtok(NULL, space);
                    }
                    wprintw(ventana1,"\n");
                    wrefresh(ventana1); 
                    limpiarVentana2();
                    enfocarVentana2();
                    sleep(1);
                }
   

           /* if ((ch = getch()) != ERR)
            {
                while (ch != KEY_ENTER)
                {
                strcat(input, ch);
                }*/
                wgetnstr(ventana2, input, 2048);
                wprintw(ventana1, " %s",nameUser);
                wprintw(ventana1,": ");
                wprintw(ventana1, " %s\n", input);
                wrefresh(ventana1);
                limpiarVentana2();
                enfocarVentana2();
            //}




       /* char buffer[TAM];
        wgetnstr(ventana2, buffer, TAM); // Leer una línea de la entrada
        if (strcmp(buffer, "-salir") == 0) {
            break;
        }
        wprintw(ventana1, "carlos: %s\n", buffer);
        wrefresh(ventana1);
        limpiarVentana2();*/


                token = strtok(input,space);

                strcpy(WriteBuf, "\0"); 

            if (strcmp(token, "-salir")==0 || salida == 1)
            {


                if (salida =1)
                {
                    strcpy(WriteBuf, "-salir");
                    strcat(WriteBuf, " ");
                    strcat(WriteBuf, nameUser);
                    write(FilePipeW,WriteBuf, strlen(WriteBuf)+1);

                }
                else
                {
                    strcat(WriteBuf, token);
                    strcat(WriteBuf, " ");
                    strcat(WriteBuf, nameUser);
                    write(FilePipeW,WriteBuf, strlen(WriteBuf)+1);
                }

                close(FilePipeW);
                close(FilePipeR);
                unlink(PipeWrite);
                unlink(PipeRead);
                endwin(); // Restaurar la operación del terminal a modo normal
                exit(0);

            }
                    if (memcmp(token, "-",1)==0)
        {   

            if(strcmp(token, "-quien")!=0 && strcmp(token, "-escribir")!=0 
            && strcmp(token, "-estoy")!=0 && strcmp(token, "-salir")!=0)
            {
                wprintw(ventana1,"comando inexistente...\n");
                wrefresh(ventana1);
                limpiarVentana2();
                enfocarVentana2();
                continue;
            } 
            strcat(WriteBuf,token);
            strcat(WriteBuf," ");
            strcat(WriteBuf, nameUser);
            strcat(WriteBuf, " ");
            token = strtok(NULL, space);
            while (token != NULL)
            { 
                    if (token !=NULL)
                    { 
                        strcat(WriteBuf,token);
                        strcat(WriteBuf," ");
                    }
                token = strtok(NULL, space);            
            }
        }

        else
        {
            strcat(WriteBuf, nameUser);
            strcat(WriteBuf, " ");
            while (token != NULL)
            {
                if (token !=NULL)
                {

                    strcat(WriteBuf,token);
                    strcat(WriteBuf," ");

                }
                token = strtok(NULL, space);
            }
        }


        strcat(WriteBuf, "\n");

        write(FilePipeW,WriteBuf, strlen(WriteBuf)+1);
        sleep(1);



    }


}

/* Mueve el cursor al punto de inserción actual de la ventana 2. */
void enfocarVentana2() {
    int y, x;
    getyx(ventana2, y, x);
    wmove(ventana2, y, x);
    wrefresh(ventana2);
}

/* Borra el contenido de la ventana 2 y ubica el cursor en la esquina 
 * superior izquierda de esta ventana.
 */
void limpiarVentana2() {
    wclear(ventana2);
    mvwhline(ventana2, 0, 0, 0, 20); // Dibujar la línea horizontal
    wmove(ventana2, 1, 0);
    wrefresh(ventana2);
}
