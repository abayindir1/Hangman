/*
 * read dictionary file to array of words & get ready to play the hangman!
 * wait for a request to come in (unique pipename)
 * respond with another unique pipename 
 * send a bunch of stars (indicating the word length)
 * fork() to enable games to proceed in parallel.
 * for each userGuess the client sends in, respond with a message 
 * and send updated display word.
 */
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define MAXNUMWORDS 84095
#define MAXWORDLENGTH 64
#define MAXLEN 1000

int main (){
                      
  // read dictionary.txt and get a random word
  char userGuessWords[MAXNUMWORDS][MAXWORDLENGTH];
	int WordsReadIn = 0;
	FILE *dictionary = fopen("dictionary.txt","r");    
  if(!dictionary){
    printf("Failed to open dictionary.txt");
    exit(1);
  }       
	char input[64];
	while(fgets(input, 63, dictionary)) {
		sscanf(input, "%s", userGuessWords[WordsReadIn]);
		WordsReadIn++;
	}
  srand (time(0) * getpid());
	int randomIndex = rand() % MAXNUMWORDS;
  char *word = userGuessWords[randomIndex];
  fclose(dictionary);
  printf("The word is: %s\n\n", word);
   //convert the word into bunch of stars
   int n = strlen(word);
  char display[MAXLEN];
  strcpy(display, word);
  for(int i=0; i<n; i++){
    display[i] = '*';
  }
  printf("The word will be displayed as: %s\n\n", display);
  
  //create main named pipe to get request from client
  char mainPipe[MAXLEN];
  sprintf(mainPipe, "/tmp/%s", getenv("USER"));
  mkfifo(mainPipe, 0666);
  printf("Send request to %s\n", mainPipe);
  
  FILE *publicfp = fopen(mainPipe, "r");
  if(!publicfp){
    printf("Failed to open public pipe");
  }
  
  char line[MAXLEN], serverfifo[MAXLEN];
  while(fgets(line, MAXLEN, publicfp)){       //client sent request
  
    printf("publicfp succesfully read\n");
    //if(fork()==0){
    
      FILE *clientfp = fopen(line, "w");
      sprintf(serverfifo, "/tmp/%s-%d", getenv("USER"), getpid());
      mkfifo(serverfifo, 0666);
      fprintf(clientfp, "%s\n", serverfifo);
      fflush(clientfp);
      
      FILE *serverfp = fopen(serverfifo, "r");
        if(clientfp){
          int userScore = 0;
          int userMiss = 0;
          int prevScore = userScore;
          fprintf(clientfp, "(userGuess) Enter a letter in word %s >\n", display);
          fflush(clientfp);
          
	  char temp[100];
          char userGuess = 0, *repeated;
	  while (fgets(temp, 100, serverfp)) {
	   userGuess = temp[0];
	    printf("Guess: %c \n", userGuess);
            repeated = strchr(display, userGuess);
            if(repeated){
              fprintf(clientfp, "You already guessed this. ");
              fflush(clientfp);
            }else{
              
              for(int i =0; i<strlen(word); i++){
                if(word[i] == userGuess){
                  display[i] = userGuess;
                  userScore++;
                }
              }
              
              if(userScore == prevScore){
                fprintf(clientfp, "Your guess '%c' is not in the word ", userGuess);
                fflush(clientfp);
                userMiss++;
              }else if(userScore == strlen(word)){
                fprintf(clientfp, "The word is complete. You missed %d times\n", userMiss);
                fflush(clientfp);
                break;
              }

                fprintf(clientfp, "(Guess) Enter a letter in word %s >\n", display);
                fflush(clientfp);
              
            }
            
          }
          fclose(clientfp);
          unlink(serverfifo);
        //}
    }
    
    
  }
  
  fclose(publicfp);
  unlink(mainPipe);
}
