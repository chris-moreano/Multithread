// Prasamsha Pradham
// Christian Moreano
// CS 149 , Spring 2015

// Build and link by: gcc -g -Wall -pthread Multithread.c -lpthread -o anyname
// execute by: ./anyname

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define L_QUEUE_SIZE ( (N/2) + 1 ) //Waiting Queue size
#define N 5     //Riders
#define M 1     //Drivers
#define T 40    //Time in seconds

// Defining mutex for preventing access at certain variable at the same time
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_rider = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_driver = PTHREAD_COND_INITIALIZER;


const int MAX_SLEEP = T / 2;

int riderQ[L_QUEUE_SIZE]; //Instantiate array of integers size L_QUEUE_SIZE
int rider_processed[N];        //Instantiate Rider array.
int qSize = 0;             //Initial Queue size
int first = 0;
int last = 0;

int totalRidersServed = 0;     //Initial Riders served


void *Cliente();        //Function Prototype of Riders
void *Manejador();      //Function Prototype of Drivers

int main()
{
    printf( "==========================================");
    printf("\n      PMServices ");
    printf("\n     ___________  By             ");
    printf("\n    //   |||   \\\\   Prasamsha Pradhan        ");
    printf("\n __//____|||____\\\\____  Christian Moreano       ");
    printf("\n| _|      |       _  ||         ");
    printf("\n|/ \\______|______/ \\_||         ");
    printf("\n_\\_/_____________\\_/_____\n");
    printf( "==========================================\n");

    printf("Riders(N)= %d ,Drivers(M)= %d ,Time(S)= %d\n\n", N , M ,T);

	srand( time( NULL ) ); //random number generator, call rand()

    //EACH DRIVER/RIDER IS A SEPARATE THREAD
	pthread_t riders[N];
	pthread_t drivers[M];

	int i = 0; // Needed to loop through
	for( i = 0; i < T; ++i )
		{
		    riderQ[i] = 0;
		}

//Each Rider is identified by an ID (from 1 to N)
	int rider_id[N];
	int driver_id[M];

	for( i = 0; i < N; ++i )
	{
		rider_processed[i] = 0;
		rider_id[i] = i + 1;
		pthread_create( &riders[i], NULL, Cliente, rider_id + i );
	}

//Driver is identified by an ID (fromN+1 to N+M)
	for( i = 0; i < M; ++i )
	{
		driver_id[i] = i + N + 1;
		pthread_create( &drivers[i], NULL, Manejador, driver_id + i );
	}

//Sleep for T seconds
	sleep( T );

//Lock thread
	pthread_mutex_lock( &queue_mutex );
	sleep( 1 );

    printf( "========================================================");
	printf( "\nTotal number of Riders served: %i\n", totalRidersServed );
	printf( "Total number of Riders remains in the wait queue: %i\n", qSize );
	printf( "Average number of Riders each Driver served: %f\n",
	        (float)totalRidersServed / M );
    printf( "========================================================\n");


	return 0;
}

//RIDER
void *Cliente( void* dummy )
{
	int* id_ptr = (int *) dummy;
	int id = *id_ptr;

	srand( time( NULL ) * id );
	sleep( rand() % MAX_SLEEP );



	for( ; ; ) // run forever
	{
		if( rider_processed[id - 1] == 1 )
			sleep( 1 );
		else if( rider_processed[id - 1] == 2 )
		{
			rider_processed[id - 1] = 0;
			printf( "Rider %i: completed riding. Queue size: %i.\n",
			         id, qSize );
			srand( time( NULL ) * id );
			sleep( ( rand() % MAX_SLEEP ) + 1 );
		}

		if( rider_processed[id - 1] != 0 )
			continue;

		pthread_mutex_lock( &queue_mutex );

			while( qSize == L_QUEUE_SIZE )
				pthread_cond_wait( &cond_rider, &queue_mutex );

			riderQ[first] = id;
			rider_processed[id - 1] = 1;
			first = ( first + 1 ) % L_QUEUE_SIZE;
			++qSize;
			printf( "Rider %i: arrived. Queue size: %i.\n", id, qSize );

			pthread_cond_signal( &cond_driver );
		pthread_mutex_unlock( &queue_mutex );
	}

	pthread_exit( 0 );
}

//DRIVER
void *Manejador( void* dummy )
{
	int* id_ptr = (int *) dummy;
	int id = *id_ptr;

	srand( time( NULL ) * id );
	sleep( rand() % MAX_SLEEP );

	for( ; ; )
	{
		pthread_mutex_lock( &queue_mutex );
			printf( "Driver %i: arrived. Queue size: %i.\n", id, qSize );

			while( qSize == 0 )
				pthread_cond_wait( &cond_driver, &queue_mutex );

			rider_processed[ riderQ[last] - 1 ] = 2;
			riderQ[last] = 0;
			last = ( last + 1 ) % L_QUEUE_SIZE;
			--qSize;
			++totalRidersServed;
			printf( "Driver %i: riding. Queue size: %i.\n", id, qSize );
			pthread_cond_signal( &cond_rider );
		pthread_mutex_unlock( &queue_mutex );

		srand( time( NULL ) * id );
		sleep( ( rand() % MAX_SLEEP ) + 1 );
	}

	pthread_exit( 0 );
}

