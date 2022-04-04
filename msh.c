//  MSH main file
// Write your msh source code here
//export LD_LIBRARY_PATH=/home/username/path:$LD_LIBRARY_PATH
//#include "parser.h"
#include <stddef.h>			/* NULL */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_COMMANDS 8


// ficheros por si hay redirección
char filev[3][64];

//to store the execvp second parameter
char *argv_execvp[8];

void siginthandler(int param)
{
	printf("****  Saliendo del MSH **** \n");
	//signal(SIGINT, siginthandler);
        exit(0);
}

/**
 * Get the command with its parameters for execvp
 * Execute this instruction before run an execvp to obtain the complete command
 * @param argvv
 * @param num_command
 * @return
 */
void getCompleteCommand(char*** argvv, int num_command) {
    //reset first
    for(int j = 0; j < 8; j++)
        argv_execvp[j] = NULL;

    int i = 0;
    for ( i = 0; argvv[num_command][i] != NULL; i++)
        argv_execvp[i] = argvv[num_command][i];
}


/**
 * Main shell  Loop  
 */
int main(int argc, char* argv[])
{
    /**** Do not delete this code.****/
    int end = 0; 
    int executed_cmd_lines = -1;
    char *cmd_line = NULL;
    char *cmd_lines[10];

    if (!isatty(STDIN_FILENO)) {
        cmd_line = (char*)malloc(100);
        while (scanf(" %[^\n]", cmd_line) != EOF){
            if(strlen(cmd_line) <= 0) return 0;
            cmd_lines[end] = (char*)malloc(strlen(cmd_line)+1);
            strcpy(cmd_lines[end], cmd_line);
            end++;
            fflush (stdin);
            fflush(stdout);
        }
    }

    /*********************************/

    	char ***argvv = NULL;
    	int num_commands, status;
	//seteamos la variable de entorno Acc
	setenv("Acc","0",1);

	while (1) 
	{
		start: ;
		int status = 0;
	        int command_counter = 0;
		int in_background = 0;
		signal(SIGINT, siginthandler);

		// Prompt 
		write(STDERR_FILENO, "MSH>>", strlen("MSH>>"));

		// Get command
                //********** DO NOT MODIFY THIS PART. IT DISTINGUISH BETWEEN NORMAL/CORRECTION MODE***************
                executed_cmd_lines++;
                if( end != 0 && executed_cmd_lines < end) {
                    command_counter = read_command_correction(&argvv, filev, &in_background, cmd_lines[executed_cmd_lines]);
                }else if( end != 0 && executed_cmd_lines == end)
                    return 0;
                else
                    command_counter = read_command(&argvv, filev, &in_background); //NORMAL MODE
                //************************************************************************************************


              /************************ STUDENTS CODE ********************************/
	      if (command_counter > 0) {
                if (command_counter > MAX_COMMANDS)
                      printf("Error: Numero máximo de comandos es %d \n", MAX_COMMANDS);
                else 
		{	//guardamos los descriptores para al final reajustarlos en caso de que los modifiquemos en el padre por un filev
			int desc_in = dup(0);
        		int desc_out = dup(1);
        		int desc_error = dup(2);
			if(strcmp(argvv[0][0],"mycalc")==0)
			{//comprobamos si el comando corresponde al mycalc 
				char mensaje_error[100] = "[ERROR] La estructura del comando es <operando 1> <add/mod> <operando 2>\n";
				for (int i=1; i<5; i++)
				{
					if (i<4)
					{//si encontramos algun null saltamos error de estructura
						if (argvv[0][i] == NULL)
						{//mandamos el error a la "salida de errores" definida en el enunciado de la practica (1)
							if((write(1, mensaje_error, strlen(mensaje_error))) <0){
								perror ("Error write");
								goto start;
							}
							goto start;
						}
					}
					else
					{
						//si no encontramos null saltamos error de estructura
						if (argvv[0][i] != NULL)
						{//mandamos el error a la "salida de errores" definida en el enunciado de la practica (1)
							if((write(1, mensaje_error, strlen(mensaje_error))) <0){
								perror ("Error write");
								goto start;
							}
							goto start;
						}
					}
				}
				//preparamos las variables para la operación
    				char msg[100],nya[100];
				int suma;
    				int operando_1 = atoi(argvv[0][1]);
   				int operando_2 = atoi(argvv[0][3]);
				if(strcmp(argvv[0][2],"add")==0)
				{
					suma = operando_1 + operando_2;

					//pasamos a tipo char la suma entera + Acc para poder guardarla de nuevo en Acc
					sprintf(nya,"%i",suma + atoi(getenv("Acc")));

					//seteamos el valor nuevo de Acc guardado en la variable felina
       					if (setenv("Acc",nya,1) < 0){

						perror("Error al dar valor a la variable de entorno\n");
						goto start;
					}
        				sprintf(msg,"[OK] %i + %i = %i; Acc %s\n",operando_1,operando_2,suma,getenv("Acc"));
					//sacamos el valor del mensaje por la "salida estandar" definida en el enunciado de la practica (2)
        				if((write(2, msg, strlen(msg))) <0){

						perror ("Error write");
						goto start;
					}
		
				}
				
				else if(strcmp(argvv[0][2],"mod")==0)
				{//comprobamos que op2 no sea 0
					if (operando_2 != 0)
					{//preparamos el mensaje de salida para el descriptor 2
						snprintf(msg, 100, "[OK] %d %% %d = %d * %d + %d\n", operando_1, operando_2, operando_2, (operando_1 / operando_2), (operando_1 % operando_2));
						if((write(2, msg, strlen(msg))) <0){
							perror ("Error write");
							goto start;
						}
						
					}
					else
					{
						if((write(1, "Mod with Zero in op2 not allowed\n", strlen("Mod with Zero in op2 not allowed\n"))) <0){
							perror ("Error write");
							goto start;
						}	
					}
				}
				else
				{//mandamos el error a la "salida de errores" definida en el enunciado de la practica (1)
					if((write(1, mensaje_error, strlen(mensaje_error))) <0){
						perror ("Error write");
						goto start;
					}
					goto start;
				}
			}
			else if(strcmp(argvv[0][0],"mycp")==0)
			{//muy similar al del examen
				char kpachao[100];
				if (argvv[0][1]!=NULL && argvv[0][2]!=NULL && argvv[0][3]==NULL)
				{
				//comprobamos que no son nulos y que empiece la fiesta bby
					char texto[1024];
					int file_copy, file_paste;
					if ((file_copy = open(argvv[0][1], O_RDONLY, 0644))< 0)
					{
						if((write(1, "[ERROR] Error al abrir el fichero origen : No such file or directory\n",
                            			strlen("[ERROR] Error al abrir el fichero origen : No such file or directory\n"))) < 0){

							//sacamos el error por la salida estandar
							perror ("Error en write");
							goto start;
						}
						goto start;
					}
					if ((file_paste = open(argvv[0][2], O_TRUNC | O_WRONLY | O_CREAT, 0644))< 0)
					{//que el fichero lo cree en modo escritura, si existe ya el fichero lo borra completamente y lo crea de cero el O_CREAT
	
						if((write(1, "[ERROR] Error al abrir el fichero destino : No such file or directory\n",
                            			strlen("[ERROR] Error al abrir el fichero destino : No such file or directory\n"))) < 0){

							//sacamos el error por la salida estandar
							perror ("Error en write");
						}
						if (close(file_copy)<0){
							perror ("Error al cerrar fichero 1");
						}
						goto start;
					}
					int readf;
					while ((readf=read(file_copy,texto,sizeof(texto)))>0)
					{
						if (write(file_paste,texto,readf) <0)
						{//error de escritura
							if (close(file_copy)<0){
								perror ("Error al cerrar fichero 1");
							}
							if (close(file_paste)<0){
								perror ("Error al cerrar fichero 2");
							}
							perror ("Error de escritura");
							goto start;
							
						}
					}
					if (close(file_copy)<0){
						perror ("Error al cerrar fichero 1");
					}
					if (close(file_paste)<0){
						perror ("Error al cerrar fichero 2");
					}
					if (readf < 0){
						perror ("Error de lectura");
						goto start;
					}
                      			snprintf(kpachao, 100, "[OK] Copiado con exito el fichero %s a %s\n", argvv[0][1], argvv[0][2]);
					if (write(1, kpachao, strlen(kpachao))<0){
						perror ("Error en write");
						goto start;
					}
				}
				else
				{
					if((write(1, "[ERROR] La estructura del comando es mycp <fichero origen> <fichero destino>\n",
                          		strlen("[ERROR] La estructura del comando es mycp <fichero origen> <fichero destino>\n"))) <0){
						perror("Error en write");
						goto start;
					}
				}	
			}

			else
			{//procesos multiples con pipes y redirecciones (n)
				int p_desc[2];
				int pipe_ok, p10, desc;
				pid_t pid;
				//vamos a hacer las redirecciones en el padre ya que luego las van a heredar los hijos directamente
				//para asegurar que cada hijo reciba la que le corresponda crearemos redirecciones condicionales mas adelante
				if(strcmp(filev[0],"0")!=0)
				{
	        			if ((desc = open(filev[0],O_RDONLY))<0){
						perror("Error open file desc_in\n");
						goto start;
					}
	        			if (dup2(desc,0)<0){
						perror("Error dup2 desc_in\n");
						if (close(desc)<0){
							perror("Error cerrando desc_out\n");
						}
						goto start;
					}
	        			if (close(desc)<0){
						perror("Error cerrando desc_in\n");
						goto start;
					}
		    		}
			    	if(strcmp(filev[1],"0")!=0)
				{
					if ((desc = open(filev[1],O_TRUNC | O_CREAT| O_WRONLY, 0644))<0){
						perror("Error open file desc_out\n");
						goto start;
					}
					if (dup2(desc,1)<0){
						perror("Error dup2 desc_out\n");
						if (close(desc)<0){
							perror("Error cerrando desc_out\n");
						}
						goto start;
					}
					if (close(desc)<0){
						perror("Error cerrando desc_out\n");
						goto start;
					}
			   	}
			    	if(strcmp(filev[2],"0")!=0)
				{
					if ((desc = open(filev[2],O_TRUNC | O_CREAT| O_WRONLY, 0644))<0){
						perror("Error open file desc_error\n");
						goto start;
					}
					if (dup2(desc,2)<0){
						perror("Error dup2 desc_error\n");
						if (close(desc)<0){
							perror("Error cerrando desc_out\n");
						}
						goto start;
					}
					if (close(desc)<0){
						perror("Error cerrando desc_error\n");
						goto start;
					}
			    	}
					
				for (int i=0; i<command_counter; i++)
				{
					//mientras que no estemos en el ultimo proceso, crearemos una pipe nueva por iteración
					if (i != (command_counter-1)){
						if ((pipe_ok = pipe(p_desc))<0){
							perror("Error creando pipe");
							goto start;
						}
					}
					//2. Creamos el fork
					pid = fork();
					if (pid<0)
					{
						perror("fork: ");
						if((close(p_desc[0])) < 0 && command_counter>1){
							perror("Error al cerrar descriptor");
							goto start;
						}
						if((close(p_desc[1])) <0 && command_counter>1){
							perror("Error al cerrar descriptor");
							goto start;
						}
					return (-1); //################################################################
					//cerramos el proceso entero, padre e hijo	
					}

					//3. redirecciones y 4. limpieza
					if (0 == pid){
						if (i!=0){
							//redireccion de entrada
							if (dup2(p10,0)<0){
								perror("Error dup2 en redireccion de entrada\n");
								exit(-1);//#################################################
							}
							if (close(p10)<0){
								perror("Error cerrando pipe\n");
								exit(-1);//#################################################
							}

						}
						if (i!=command_counter-1){	
							//redireccion de salida
							if (dup2(p_desc[1],1)<0){
								perror("Error dup2 en redireccion de salida\n");
								exit(-1);//#################################################
							}		
							//limpieza en todas las pipes menos para el ultimo proceso
							if (close(p_desc[0])<0){
								perror("Error cerrando pipe\n");
								exit(-1);//#################################################
							}
							if (close(p_desc[1])<0){
								perror("Error cerrando pipe\n");
								exit(-1);//#################################################
							}
						}

						//ejecutamos proceso en hijo
						execvp(argvv[i][0], argvv[i]);
						//si pasamos de esta linea es xq algo ha ido mal como vimos en clase	
						perror("Error al ejecutar\n");
						exit(-1);
						

					//final proceso hijo
					}
					else{
						if (i!=command_counter-1){
							if (close(p_desc[1])<0){
								perror("Error cerrando pipe\n");
								exit(-1);//#################################################
							}
							p10 = p_desc[0];		
						}
						else if(command_counter>1){
							//solo se ejecuta si se han creado pipes
							if (close(p_desc[0])<0){
								perror("Error cerrando pipeeeeee\n");
								exit(-1);//#################################################
							}
						}
					if(in_background == 0){
		        			while (pid != wait(&status));
						if (status <0){
							perror("Error en proceso hijo");
						}
		    			}
					else{
						printf("[%d]\n",pid);
					}
					//final proceso padre
					}
			
				//final del bucle for	
				}
			dup2(desc_in,0);
                    	dup2(desc_out,1);
                    	dup2(desc_error,2);
                    	close(desc_in);
                    	close(desc_out);
                    	close(desc_error);
			//final del else
			}					
                }

              }
        }
	return 0;
}
