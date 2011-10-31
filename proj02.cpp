#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX 4

using namespace std;
sem_t donePhase;
sem_t oddWait;
sem_t evenWait;
sem_t mutex;

int content[] = {98, 5, 75, 90};
int eValue;
int oValue;
int dValue;

void doSwap(int l, int r)
{
  sem_wait(&mutex);                                                 //Lock the C-Region
  cout << "Entering C-Region..." << endl;
  if(content[l] > content[r])
  {
    cout << "Swapping " << content[l] << " with " << content[r] << endl;
    int tmp = content[l];
    content[l] = content[r];
    content[r] = tmp;
  }
  cout << "Leaving C-Region..." << endl;
  sem_post(&mutex);                                                 //Unlock the C-Region
  sem_post(&donePhase);                                             //Increment Phase Done
}

void displayContents()
{
  for(int i = 0; i < MAX; i++)
  {
    //cout << content[i] << endl;
  }  
}

void *sortContent(void *tid)
{
  int id = (long)tid;
  for(int i = 0; i < MAX; i++)
  {
    if(((i + 1)%2) != 0)                                            //Odd Phase
    {
      int l = (2 * id);                                             //left index always even; l = 2K
      int r = l + 1;
      sem_wait(&oddWait);
      cout << "Odd " << id << ": executing... " << endl;
      doSwap(l, r);
      sem_post(&evenWait);
    }
    else                                                            //Even Phase
    {
      if(id != 1)                                                   //Skip the last thread in the even phase (N/2)
      {	                                            
	int l = (2 * id) + 1;                                       //left index always odd: l = 2K + 1
	int r = l + 1;
	sem_wait(&evenWait);
	cout << "Even " << id << ": executing... " << endl;
	doSwap(l, r);
	sem_post(&oddWait);
      }
    }  
    //sem_getvalue(&evenWait, &eValue)
    //sem_getvalue(&oddWait, &oValue)
    //sem_getvalue(&donePhase, &dValue)
    //cout << "Even: " << eValue << 
    //endl << "Odd:  " << oValue << 
    //endl << "Done: " << dValue <<
    //endl;
  }
  displayContents();
}

int main(int argc, char *argv[])
{
  pthread_t worker[2];
  
  sem_init(&donePhase, 0, 0);
  sem_init(&oddWait, 0, 2);
  sem_init(&evenWait, 0, 0);
  sem_init(&mutex, 0, 1);
  
  for(int i = 0; i < 2; i++)
  {
    pthread_create(&worker[i], NULL, sortContent, (void*)i);
  }

  for(int i = 0; i < 2; i++)
  {
    pthread_join(worker[i], NULL);
  }
  return 0;
}
