#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <fstream>

#define MAX 20

using namespace std;
sem_t donePhase;
sem_t oddWait;
sem_t evenWait;
sem_t mutex;

int content[MAX];
int count;
//For Debugging...
//int eValue;
//int oValue;
//int dValue;

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
    cout << "File Contents:" << endl;
    for(int i = 0; i < count; i++)
    {
        cout << content[i] << " ";
    }  
    cout << endl;
}

void *sortContent(void *tid)
{
    int id = (long)tid;
    for(int i = 0; i < count; i++)
    {
        if(((i + 1) % 2) != 0)                                            //Odd Phase
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
	//For Debugging
	//sem_getvalue(&evenWait, &eValue)
	//sem_getvalue(&oddWait, &oValue)
	//sem_getvalue(&donePhase, &dValue)
	//cout << "Even: " << eValue << 
	//endl << "Odd:  " << oValue << 
	//endl << "Done: " << dValue <<
	//endl;
    }
}

int readFile(char *fileName)
{
    int value;
    int count = 0;
    ifstream file(fileName, fstream::in);
    if(file.is_open())
    {
        while((file >> value) && (count < MAX))
	{
	  content[count++] = value;      
	}
    } else {
      cout << "Invalid file name or directory!" << endl;
    }
  return count;
}

int main(int argc, char *argv[])
{
    if(argc == 2)
    {
      //cout << "Filename " << argv[2] << endl;
        count = readFile(argv[1]);
	
	int noWorkers = (count / 2);
	pthread_t worker[noWorkers];
	
	sem_init(&donePhase, 0, noWorkers);
	sem_init(&oddWait, 0, noWorkers);
	sem_init(&evenWait, 0, 0);
	sem_init(&mutex, 0, 1);
      
	displayContents();

	for(int i = 0; i < noWorkers; i++)
	{
	    pthread_create(&worker[i], NULL, sortContent, (void*)i);
	}

	for(int i = 0; i < noWorkers; i++)
        {
	  pthread_join(worker[i], NULL);
        }

	sem_destroy(&donePhase);
	sem_destroy(&oddWait);
	sem_destroy(&evenWait);
    }
    else
    {
        cout << "File name not included!" << endl;
    }
    return 0;
}
