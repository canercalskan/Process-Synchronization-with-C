//1900003421
//CANER EREN ÇALIŞKAN

/* ## NOTE ##

- atmThread is waiting for payThread's termination, to display "atm > signing" off message. 
- in the race.png screenshot , bill thread displays termination message when EOF entered, but atm didn't because it is waiting.

i wanted to explain this because you may think that the race.png output is not correct , thank you.

 */


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

pthread_mutex_t accountmutex;
sem_t coordination;
sem_t payterminates;
int atmstatus , paystatus , billstatus;
int amount , account = 0;

void *atmThread() {
int amount;
printf("atm > started\n");
printf("atm > Enter amount (+/-) or `ctrl+d`to exit = ");
while(scanf("%d", &amount) != EOF) {

  if(amount < 0) {
     if((amount + account) < 0) {
	perror("atm > Insufficient Balance\n");
	printf("atm > Enter amount (+/-) or `ctrl+d`to exit = ");
	}
     else {
	pthread_mutex_lock(&accountmutex); //locking account mutex, because we will make changes on shared account variable.
     	account += amount;
	printf("\natm > new balance is %d\n", account);
	pthread_mutex_unlock(&accountmutex); //unlocking account mutex.
	sem_post(&coordination);
	printf("atm > Enter amount (+/-) or `ctrl+d`to exit = ");
      }     
    }
  else if(amount > 0) {
     pthread_mutex_lock(&accountmutex); //locking account mutex, because we will make changes on shared account variable.
     account += amount;
     printf("\natm > new balance is %d\n", account);
     pthread_mutex_unlock(&accountmutex); //unlocking account mutex.
     sem_post(&coordination);
     printf("atm > Enter amount (+/-) or `ctrl+d`to exit = ");
    }
}   
atmstatus = 0; //setting the atmThread's status to 'off'

/*    ## NOTE ## There was no 'wait for pay threads termination then sign off atm thread' in the example program "prj5-p1" , but i think the
   atm machine shouldn't close itself unless every other threads has been finished.
*/

sem_wait(&payterminates); //waiting for payThread's termination.
printf("\natm > EOF entered, signing off.\n"); //printing termination message.

pthread_exit(NULL); //thread exists.
}


void *payThread(void *ptw) {
fprintf(ptw , "\t\tpay > started\n");

sem_wait(&coordination);
do {
 sleep(12);
 pthread_mutex_lock(&accountmutex); //locking account mutex, because we will make changes on shared account variable.
 account += 10;
 pthread_mutex_unlock(&accountmutex); //unlocking account mutex.
 fprintf(ptw , "\t\tpay > +10 , new balance = %d\n" , account);

}while(atmstatus > 0); //above codes will continue until atmThread's status is 'off'.

paystatus = 0; //setting the paythread's status to 'off'
fprintf(ptw , "\t\tpay > EOF entered , signing off.\n"); //printing termination message.
sem_post(&payterminates);
pthread_exit(NULL); //thread exits.
}


void *billThread(void *ptw) {
fprintf(ptw , "bill > started\n");

do {
sleep(3);
pthread_mutex_lock(&accountmutex); //locking account mutex, because we will make changes on shared account variable.
account -= 1;
pthread_mutex_unlock(&accountmutex); //unlocking account mutex.
fprintf(ptw , "bill > -1 new balance = %d\n" , account);
}while(atmstatus > 0);

billstatus = 0;
fprintf(ptw , "bill > EOF entered , signing off.\n");
pthread_exit(NULL);
}




int main() {
FILE *ptw;
//opening the second terminal window /dev/pts/1
while((ptw = fopen("/dev/pts/1" , "w")) == NULL) {
	perror("please open dev/pts/1 terminal\n");
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

 printf("main > atm<%lu> thread has been created\n" , atm_thread_id);
 printf("main > pay<%lu> thread has been created\n" , pay_thread_id);
 
 pthread_mutex_init(&accountmutex , NULL);
 sem_init(&coordination , 0 , 0);
 sem_init(&payterminates , 0 , 0);
 //making the main thread wait for termination of 'atm', 'pay' and 'bill' threads.
 pthread_join(atm_thread_id , NULL);
 pthread_join(bill_thread_id , NULL);
 pthread_join(pay_thread_id , NULL); 


 fclose(ptw);

 printf("\nmain > signing off\n");
return 0;
}
