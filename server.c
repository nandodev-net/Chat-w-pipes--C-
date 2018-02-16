//fernando gonzalez
//giuli latella


#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h> 
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <unistd.h>
#include <string.h>
#include <linux/stat.h>
#define MAX_Slots 5
#define Green_Fifo "/tmp/servidor"

	char *PipeLink;
	char ReadBuf[2048];
	char Buffer[2048];
	int FilePipe;
	char *token;
	const char space[2] = " ";
	int SlotUsed = 0;
	int tam=0;

	char *auxName;
	char *auxRD;
	char *auxWR;
	
	char auxOnlined[2048];	//variable que almacena la lista de usuarios conectados
	char message[2048];		//variable que almacena el descriptor al que le escriben
	char multicast[2048];

	typedef struct Linked
	{
		int here;
		char *nameUser;
		char *status;
		char *contact; //lectura
		int DescriptorRd;
		int DescriptorWr;
	} OnlineUsers;
	OnlineUsers tabla[MAX_Slots];



	fd_set Inputs;
	fd_set InputCopy;

	struct timeval timeout;


int SearchUser(char *name)
{
	int pos = 0;
	for(pos; pos < SlotUsed; pos++)
	{
		if(!strcmp(tabla[pos].nameUser , name))
		{
			return pos;
		}
	}
	return -1;
}

int MaxDescriptor()
{
	int number = 0;
	int j =0;
	for (j ; j < SlotUsed; j++)
	{
		if(tabla[j].DescriptorRd > number)
		{
			number = tabla[j].DescriptorRd;
		}
	}
	return number;
}


void LogInUser(char *name, char *fifoR, char *fifoW)
{
	char failLogMess [60];
	int descrip2;
	int descrip1;
	int l;
	int heIs = SearchUser(name);


	descrip1=open(fifoR, O_RDONLY | O_NONBLOCK);
					printf("A FIFO READ LE ENTRA :%d  %s\n", descrip1, fifoR);

					printf("Uso/Max: %d  %d \n", SlotUsed, MAX_Slots );
	
	if (SlotUsed != MAX_Slots || heIs == -1)
	{
		
		if (SlotUsed > 0)
		{	
			


			char *NewUser = (char*) malloc((strlen(name) + strlen(" ha iniciado sesion... ") + 1)* sizeof(char));

			strcat(NewUser, name);
			strcat(NewUser, " ha iniciado sesion... ");
			for(l= 0; l < SlotUsed; l++)
			{
				write(tabla[l].DescriptorWr, NewUser, strlen(NewUser) +1 );
					
			}
		}
	}

	else
	{
		if (heIs != -1)
		{
			strcpy(failLogMess, "#$CODELOGERROR UsuarioExistente...\n");
		}
		else
		{
			strcpy(failLogMess, "#$CODELOGERROR SalaLLena...\n");
		}
			descrip2=open(fifoW, O_WRONLY | O_NONBLOCK);
			write(descrip2, failLogMess, strlen(failLogMess)+1);
			close(descrip2);
		
	}
	 
		tabla[SlotUsed].nameUser = (char*) malloc (strlen(name) * sizeof(char));
		strcpy(tabla[SlotUsed].nameUser, name);
		tabla[SlotUsed].here = 1;
		tabla[SlotUsed].status = (char*) malloc (40 * sizeof (char));
		strcpy(tabla[SlotUsed].status, "vacio");
		tabla[SlotUsed].contact = (char*) malloc (40 * sizeof (char));
		strcpy(tabla[SlotUsed].contact, "vacio");

							printf("guardo nombre: %s\n", tabla[SlotUsed].nameUser);

		tabla[SlotUsed].DescriptorRd = descrip1;

		FD_SET(tabla[SlotUsed].DescriptorRd, &Inputs);
		descrip2=open(fifoW, O_WRONLY | O_NONBLOCK);
							printf("A FIFO WRITE LE ENTRA :%d  %s\n", descrip2, fifoW);
		tabla[SlotUsed].DescriptorWr = descrip2;
		++SlotUsed;	
							printf("Uso/Max2: %d  %d \n", SlotUsed, MAX_Slots );


}

void LogOutUser(char *name)
{
	//envia un mensaje de logOut a quienes hablen con el
	//cierra los pipes
	name = strtok(NULL, space);
	printf("AAA\n");
	int w;
	char logoutMess[256];
	strcpy(logoutMess, "\0");
	strcat(logoutMess,"## ");
	strcat(logoutMess, name);
	strcat(logoutMess," ha cerrado sesion...##");
	printf("%s\n",logoutMess);
	int pos = SearchUser(name);
	close (tabla[pos].DescriptorRd);
	close(tabla[pos].DescriptorWr);

	FD_CLR(tabla[pos].DescriptorRd, &Inputs);

	tabla[pos].here = 0;
	for (w = 0; w<SlotUsed; w++)
	{
		if(!strcmp(tabla[w].contact, name))
		{
			strcpy(tabla[w].contact, "vacio");
			write(tabla[w].DescriptorWr, logoutMess, strlen(logoutMess)+1);
		}
	}

	free(tabla[pos].nameUser);
	free(tabla[pos].status);
	free(tabla[pos].contact);

	tabla[pos] = tabla[SlotUsed-1];	

	tabla[SlotUsed-1].nameUser = NULL;
	tabla[SlotUsed-1].status = NULL;
	tabla[SlotUsed-1].contact = NULL;
	SlotUsed --;



}
 
void quien (char *origin)
{	int y;
	int p;

	origin = strtok(NULL, space);
	p = SearchUser(origin);
	strcpy(auxOnlined, "\0");

	for ( y=0; y < SlotUsed; y++)
	{		
		if (tabla[y].here = 1)
		{	
			strcat(auxOnlined, "\n");
			strcat(auxOnlined, tabla[y].nameUser);
			strcat(auxOnlined, " | status: ");
			strcat(auxOnlined, tabla[y].status);
			strcat(auxOnlined,"\n");
		}
	} 

	write(tabla[p].DescriptorWr, auxOnlined, strlen(auxOnlined)+1);
}

void escribir (char *origin)
{
	char message[256];
	strcpy(message,"...Hey, no puedes hablar contigo mismo!!!...\n");
	origin = strtok(NULL, space);
	int p = SearchUser(origin);		//busco el usuario que se quiere comunicar
	origin = strtok(NULL, space);
	if (strcmp(tabla[p].nameUser, origin))
	{	
		strcpy(tabla[p].contact, origin);	//agrego en su estructura el user con el que habla
	}
	else
	{
		write(tabla[p].DescriptorWr, message, strlen(message)+1);
	}
}

void estoy (char *origin)
{	
	origin = strtok(NULL, space);
	int i = 0;
	int p = SearchUser(origin);


	strcpy(tabla[p].status,"\0");
	while (origin != NULL)
	{
			origin = strtok(NULL, space);


			if (origin !=NULL)
			{
				strcat(tabla[p].status,origin);
				strcat(tabla[p].status," ");
			}
	}

			strcpy(multicast, "\0");
			strcat(multicast, "## ");
			strcat(multicast, tabla[p].nameUser);
			strcat(multicast, " | nuevo estado: ");
			strcat(multicast, tabla[p].status);
			strcat(multicast, " ##");
			strcat(multicast, "\n");


	for (i; i < SlotUsed; i++)
	{
		if (!strcmp(tabla[p].nameUser, tabla[i].contact))
		{

			printf("%s\n",tabla[i].nameUser );
			printf("%d\n", tabla[i].DescriptorWr);
			write(tabla[i].DescriptorWr, multicast, strlen(multicast)+1);

		}
	}


		
}


int main(int argc, char *argv[])
{		

	int comunication;
	int comunication2;
	int comDescriptor;

		if (argc==2)
		{
			PipeLink = (char*) malloc(strlen(argv[1])* sizeof(char));
			strcpy(PipeLink, argv[1]);
		}
		else if(argc == 1)
		{
			PipeLink = (char*) malloc(strlen(Green_Fifo)* sizeof(char));
			strcpy(PipeLink, Green_Fifo);			
		}
		else
		{
			printf("Entrada erronea...\n");
		}

		unlink(PipeLink);
		mknod(PipeLink, S_IFIFO,0);
		chmod(PipeLink, 0660);
		FilePipe = open(PipeLink, O_RDONLY | O_NONBLOCK);


		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		FD_ZERO(&Inputs);
		FD_ZERO(&InputCopy);

		while (1)
		{
			int w = 0;
			InputCopy = Inputs;
			int max_desc = (MaxDescriptor());
			select(max_desc +1, &InputCopy, NULL, NULL, &timeout);
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;			

			for(w; w < SlotUsed ; w++)
			{
				if (FD_ISSET(tabla[w].DescriptorRd, &InputCopy))
				{
					read(tabla[w].DescriptorRd, Buffer, 2048);

					token = strtok(Buffer,space);
					printf("este es token lectura: %s\n", token );
					fflush(stdout);

					if (strcmp("-quien",token)==0)
					{	

						quien(token);
						continue;
				    }

				    else if (strcmp("-escribir",token)==0)
				    {
				    							
						escribir(token);
											
						continue;
				    }

				    else if (strcmp("-estoy",token)==0)
				    {  						
						estoy(token);
												
						continue;
				    }

				    else if (strcmp("-salir",token)==0)
				    {
						LogOutUser(token);
				        continue;
				    }

				    strcpy(message, "\0");
				    comunication = SearchUser(token);
				    if (!strcmp(tabla[comunication].contact, "vacio"))
				    {	
				    	strcpy(message, "... no estas hablando con nadie!!... Usa -escribir...\n");
				    	write(tabla[comunication].DescriptorWr, message, strlen(message)+1);
				    	continue;
				    }
				    comunication2 = SearchUser(tabla[comunication].contact);
				    comDescriptor = tabla[comunication2].DescriptorWr;


					while (token != NULL)
					{ 
						if (token != NULL)
						{
							strcat(message, token);
							strcat(message, " ");
						}
						token = strtok(NULL, space);
					}
					strcat(message, "\n");
					printf("este es el mensaje:%s\n",message );
					write(comDescriptor, message, strlen(message)+1);		
				}
			}

			//token = "holis";





			if (read(FilePipe, ReadBuf, 2048) > 0)
			{
				//printf(" creando datos de estructura...\n");
				//printf("este es el BUFFER:  %s\n", ReadBuf );
				token = strtok(ReadBuf, space);
				auxName = (char*) malloc((strlen(token)+1) * sizeof(char));
				strcpy(auxName, token);
				token = strtok(NULL,space);
//sscanf(readbuff, "%s %s %s", auxName, auxRD, auxWR);
				auxRD = (char*) malloc((strlen(token)+1) * sizeof(char));
				strcpy(auxRD, token);
				token = strtok(NULL, space);
				auxWR = (char*) malloc((strlen(token) +1)* sizeof(char));
				strcpy(auxWR, token);
				//printf("le estoy pasando %s %s %s\n",auxName, auxRD, auxWR );
				LogInUser(auxName, auxRD, auxWR);
				free(auxRD);
				free(auxWR);

						//printf("LO LOGRA!!!\n");



			}
		}


}
