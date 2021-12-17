/* make up unique pipename
 * send it to the server
 * get another unique pipename from the server
 * repeat
 * 	get the starred word from the new pipe
 *  display to the user 
 *  get the user's guess letter
 *  send to the server 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAXLEN 100


int main(int argc, char *argv[]) {	
   char clientfifo[MAXLEN], serverfifo[MAXLEN], mainPipe[MAXLEN];
 
   //forming client named pipe
   sprintf(clientfifo, "/tmp/%s-%d", getenv("USER"), getpid());
   mkfifo(clientfifo, 0666);
   
   //opening public pipe to write, and send client named pipe string to server
   FILE *publicfp = fopen(argv[1], "w");
   fprintf(publicfp, "%s", clientfifo);
   //fflush(publicfp);
   fclose(publicfp);
   
   FILE *clientfp = fopen(clientfifo, "r");
   fgets(serverfifo, MAXLEN, clientfp);
   char *cptr = strchr(serverfifo, '\n');
      if(cptr){
        *cptr='\0';
      }
   FILE *serverfp = fopen(serverfifo, "w");
   
   
   
   while(1){
     char line[MAXLEN];
     if(fgets(line, MAXLEN, clientfp)){
       puts(line);
     }
     
     if(strstr(line, "The word is")){
       break;
     }
     
     char input = getchar();
     getchar();
     printf("Guess %c\n", input);
     fprintf(serverfp, "%c\n", input);
     fflush(serverfp);
   }
   
   fclose(clientfp);
   unlink(clientfifo);
   exit(0);
 
}
