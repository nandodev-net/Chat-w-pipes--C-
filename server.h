# ----------------------------------------------------------------------------#
#                                  server.h                                   #
# ----------------------------------------------------------------------------#
#                         Universidad Simón Bolívar                           #
#            Departamento de Computación y Tecnología de la Información       #
#                     Laboratorio de Sistemas Operativos                      #
#              Profesora Angela Di Serio y Profesor Carlos Gomez              #
#                       Fernando Gonzalez carné: 10-10464                     #
#                          Giuli Latella carné: 08-10596                      #
#                                 Grupo: C                                    #
# ----------------------------------------------------------------------------#
# ----------------------------------------------------------------------------#

#ifndef server_h
#define server_h

int SearchUser(char *name);
int MaxDescriptor();
void LogInUser(char *name, char *fifoR, char *fifoW);
void LogOutUser(char *name);
void quien (char *origin);
void escribir (char *origin);
void estoy (char *origin);

#endif

