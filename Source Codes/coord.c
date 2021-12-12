//1900003421
//CANER EREN ÇALIŞKAN

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <threads.h> //library to use thrd_current() function , to get the thread id.
#include <semaphore.h> //library to use semaphores.
#define nbslots 4 //number of slots in the Circuler Queue
typedef struct { //log record layout
char type; 	 //A: ATM , P: PAY, B:BILL
int amount; 	 //to be deposit or debited.
int oldbal; 	 //account balance before transaction.
int newbal;	 //account balance after transaction.

}LogRec , *pLogRec;

LogRec CirQ[nbslots];
sem_t s_cirq;
sem_t s_full;
sem_t s_free;
int in = 0;
int out = 0;


int atmstatus , paystatus , billstatus;
int amount , account = 0;

void *atmThread() {
int amount;
printf("atm > started\n");
printf("atm > Enter amount (+/-) or `ctrl+d`to exit = ");
while(scanf("%d", &amount) != EOF) {
  sem_wait(&s_free); //waiting for free slot signal.
  CirQ[in].type = 'A';
  CirQ[in].amount = amount;
  CirQ[in].oldbal = account;

  if(amount < 0 && (amount + account) < 0) {
	perror("atm > Insufficient Balance\n");
	printf("atm > Enter amount (+/-) or `ctrl+d`to exit = ");
     }

  else {
     account += amount;
     CirQ[in].newbal = account;
     in = (in+1) % nbslots;
     sleep(1);
     sem_post(&s_full); //posting full signal , that means a record has been created and available for display.
     printf("\natm > new balance is %d\n", account);
     printf("atm > Enter amount (+/-) or `ctrl+d`to exit = ");
     }     
    
}   
printf("\natm > EOF entered, signing off.\n"); //printing termination message.
atmstatus = 0; //setting the atmThread's status to 'off'
pthread_exit(NULL); //thread exists.
}


void *payThread(void *ptw) {
fprintf(ptw , "\t\tpay > started\n");

do {
 sem_wait(&s_free); //waiting for free slot signal.
 sleep(12);
 CirQ[in].oldbal = account;
 account += 10;
 CirQ[in].type = 'P';
 CirQ[in].amount = 10;
 CirQ[in].newbal = account;
 in = (in+1) % nbslots;
 fprintf(ptw , "\t\tpay > +10 , new balance = %d\n" , account);
 sem_post(&s_full); //posting full signal , that means a record has been created and available for display.

}while(atmstatus > 0); //above codes will continue until atmThread's status is 'off'.
fprintf(ptw , "\t\tpay > EOF entered , signing off.\n"); //printing termination message.
paystatus = 0; //setting the paythread's status to 'off'

pthread_exit(NULL); //thread exits.
}



void *billThread(void *ptw) {
fprintf(ptw , "bill > started\n");

do {
sem_wait(&s_free); //waiting for free slot signal.
sleep(3);
  CirQ[in].oldbal = account;
account -= 1;
  CirQ[in].type = 'B';
  CirQ[in].amount = -1;
  CirQ[in].newbal = account;
in = (in+1) % nbslots;
sem_post(&s_full); //posting full signal , that means a record has been created and available for display.
fprintf(ptw , "bill > -1 new balance = %d\n" , account);
}while(atmstatus > 0);

fprintf(ptw , "bill > EOF entered, signing off.\n");
billstatus = 0;
pthread_exit(NULL);
}

//producer threadlerinden her post sinyali geldiğinde , arch thread'i record'u display edecek.//
//her bir display'den sonra , producer threadlerine s_free sinyali gönderecek.

void *archThread(void *ptw2) {
fprintf(ptw2 , "arch > started\n");
do {
    while(sem_trywait(&s_full) == 0) {
	char type = CirQ[out].type;
	int amount = CirQ[out].amount;
	int old = CirQ[out].oldbal;
	int new = CirQ[out].newbal;

	fprintf(ptw2 , "arch> %c\t\t%d\t%d\t%d\n" , type , amount , old , new );
	out = (out+1) % nbslots;
	sem_post(&s_free);
        }
}while(atmstatus > 0 || paystatus > 0 || billstatus > 0); //arch thread will continue its execution until all of the producer threads are terminated.

fprintf(ptw2 , "arch> EOF entered and no more records, signing off.\n");
pthread_exit(NULL);
}

int main() {
FILE *ptw;
FILE *ptw2;
//opening the second terminal window /dev/pts/1
while((ptw = fopen("/dev/pts/1" , "w")) == NULL) {
	perror("please open dev/pts/1 terminal\n");
	sleep(1);	
 }
 //opening the third terminal window /dev/pts/2
while((ptw2 = fopen("/dev/pts/2" , "w")) == NULL) {
	perror("please open dev/pts/2 terminal\n");
	sleep(1);	
 }

 //creating 'atm' thread.
 pthread_t atm_thread_id;
 pthread_create(&atm_thread_id , NULL , atmThread , NULL);
 atmstatus = 1;

 //creating 'pay' thread
 pthread_t pay_thread_id;
 pthread_create(&pay_thread_id , NULL , payThread , ptw);
 paystatus = 1;
 //creating bill thread.
 pthread_t bill_thread_id;
 pthread_create(&bill_thread_id , NULL , billThread , ptw);
 billstatus = 1;
 
  //creating consumer thread 'arch'
 pthread_t arch_thread_id;
 pthread_create(&arch_thread_id , NULL , archThread , ptw2);


 printf("main > atm<%lu> thread has been created\n" , atm_thread_id);
 printf("main > pay<%lu> thread has been created\n" , pay_thread_id);
 printf("main > arch<%lu> thread has been created\n\n", arch_thread_id);
 sem_init(&s_cirq , 0 , 1);
 sem_init(&s_full , 0 , 0);
 sem_init(&s_free , 0 , nbslots);

 //making the main thread wait for termination of 'atm', 'pay' and 'bill' threads.
 pthread_join(atm_thread_id , NULL);
 pthread_join(bill_thread_id , NULL);
 pthread_join(pay_thread_id , NULL); 
 pthread_join(arch_thread_id , NULL);
 


 fclose(ptw);
 fclose(ptw2);
 printf("\nmain > signing off\n");
return 0;
}
