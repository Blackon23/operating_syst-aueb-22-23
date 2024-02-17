#include <stdio.h>
#include "p3150266-p3180029-p3160168-pizzeria.h"
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> 
#include <time.h>

//Declaration of mutexes
pthread_mutex_t mutexCook = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexOven = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mutexPacker = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexDeliverer= PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mutexScreen = PTHREAD_MUTEX_INITIALIZER;

//Declaration of conditions
pthread_cond_t condCook = PTHREAD_COND_INITIALIZER;
pthread_cond_t condOven = PTHREAD_COND_INITIALIZER;
pthread_cond_t condPacker = PTHREAD_COND_INITIALIZER;
pthread_cond_t condDeliverer = PTHREAD_COND_INITIALIZER;

//Declaration of variables
int AvailableCooks = Ncook;
int AvailableOvens = Noven;
int AvailablePackers = Npacker;
int AvailableDeliverers = Ndeliverer;

int totalRev = 0;						//total pizza Sales Revenue
int sumPlain = 0, sumSpec = 0;			//number of Sales by Type of pizza
int sumSucc = 0, sumFail = 0;			//number of Successful and Failed Orders 	

double avOrder, maxOrder = -1;			//average and maximum Order Time
double avCold, maxCold = -1;			//average and maximum Colding Time

unsigned int seed = 0;

void *Order(void *x);
int Random(unsigned int seed, int low, int high); 
int Propability(unsigned int seed, int percentage);

void *Order(void* x){
	int threadid = *(int*)(x);
	
	struct timespec startorder; 
	struct timespec	finishorder;
	struct timespec startcolding;
	struct timespec	finishcolding;
	
	int num_pizzas = Random(seed*threadid, Norderlow, Norderhigh);		
	int num_prep = num_pizzas * Tprep;
	int num_pack = num_pizzas * Tpack;
	
	int type[num_pizzas];
	int tmp_plain = 0;
	int tmp_spec = 0;
	
	for (int i = 0; i < num_pizzas; i++){
		type[i] = Propability(seed*threadid, Pplain);		
		if (type[i] == 1) {			
			++tmp_plain;
		}else{
			++tmp_spec;
		}
	}
	
	int rc;
	
	clock_gettime(CLOCK_REALTIME, &startorder);
	if( clock_gettime( CLOCK_REALTIME, &startorder) == -1 ) {
			printf("ERROR: clock gettime = -1");
			exit(-1);
	}

	sleep(Random(seed*threadid, Tpaymentlow, Tpaymenthigh));
	
	if (Propability(seed*threadid, Pfail) == 1) {									
		printf("Order with number %d failed to charge the credit card and is canceled.\n", threadid);
		++sumFail;
		//exit tou thread tou pelath
		pthread_exit(NULL);
	} else {
		++sumSucc;
		totalRev += Cplain*tmp_plain + Cspecial*tmp_spec;
		sumPlain += tmp_plain;
		sumSpec += tmp_spec;
	}
	
	rc = pthread_mutex_lock(&mutexCook);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		exit(-1);		
	}
	
    while (AvailableCooks <= 0){
		
        rc = pthread_cond_wait(&condCook, &mutexCook); 
		if (rc != 0) {	
			printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
			pthread_exit(&rc);
		}
	}	
	AvailableCooks--; 
	
	rc = pthread_mutex_unlock(&mutexCook); 
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		exit(-1);		
	}
	
	sleep(num_prep);
	
	rc = pthread_mutex_lock(&mutexOven);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		exit(-1);		
	}
	
	
	while(AvailableOvens - (num_pizzas) < 0){

		rc = pthread_cond_wait(&condOven ,&mutexOven);
		if (rc != 0) {	
			printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
			pthread_exit(&rc);
		}
	}
	
	AvailableOvens -= (num_pizzas);	

		
	rc = pthread_mutex_unlock(&mutexOven);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		exit(-1);		
	}
	
	rc = pthread_mutex_lock(&mutexCook);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		exit(-1);		
	}

	++AvailableCooks;
	
	rc = pthread_cond_signal(&condCook);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
		pthread_exit(&rc);
	}	
	
	rc = pthread_mutex_unlock(&mutexCook);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		exit(-1);		
	}
		
	sleep(Tbake);
	
	clock_gettime(CLOCK_REALTIME, &startcolding);
	if( clock_gettime( CLOCK_REALTIME, &startcolding) == -1 ) {
		printf("ERROR: clock gettime = -1");
		exit(-1);
	}
	
	rc = pthread_mutex_lock(&mutexPacker);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		exit(-1);		
	}
	
	while(AvailablePackers <= 0){

		rc = pthread_cond_wait(&condPacker ,&mutexPacker);
		if (rc != 0) {	
			printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
			pthread_exit(&rc);
		}
	}
	--AvailablePackers;
		
	rc = pthread_mutex_unlock(&mutexPacker);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		exit(-1);		
	}
	
	sleep(num_pack);
	
	rc = pthread_mutex_lock(&mutexPacker);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		exit(-1);		
	}

	++AvailablePackers;
	
	rc = pthread_cond_signal(&condPacker);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
		pthread_exit(&rc);
	}
	
	rc = pthread_mutex_unlock(&mutexPacker);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		exit(-1);		
	}
	
	rc = pthread_mutex_lock(&mutexOven);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		exit(-1);		
	}

	AvailableOvens += (num_pizzas);		
	
	rc = pthread_cond_signal(&condOven);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
		pthread_exit(&rc);
	}
	
	rc = pthread_mutex_unlock(&mutexOven);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		exit(-1);		
	}

	rc = pthread_mutex_lock(&mutexDeliverer);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		exit(-1);		
	}
	
	while(AvailableDeliverers <= 0){

		rc = pthread_cond_wait(&condDeliverer ,&mutexDeliverer);
		if (rc != 0) {	
			printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
			pthread_exit(&rc);
		}
	}
	--AvailableDeliverers;
		
	rc = pthread_mutex_unlock(&mutexDeliverer);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		exit(-1);		
	}
	
    int time_delivery = Random(seed*threadid, Tdellow, Tdelhigh);

    sleep(time_delivery); 

    clock_gettime(CLOCK_REALTIME, &finishcolding);
	if( clock_gettime( CLOCK_REALTIME, &finishcolding) == -1 ) {
		printf("ERROR: clock gettime = -1");
		exit(-1);
	}	
 
    clock_gettime(CLOCK_REALTIME, &finishorder); 
	if( clock_gettime( CLOCK_REALTIME, &finishorder) == -1 ) {
		printf("ERROR: clock gettime = -1");
		exit(-1);
	}

    sleep(time_delivery); 
 
	rc = pthread_mutex_lock(&mutexDeliverer);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		exit(-1);		
	}

	++AvailableDeliverers;
	
	rc = pthread_cond_signal(&condDeliverer);			
	if (rc != 0) {	
		printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
		pthread_exit(&rc);
	}
	
	rc = pthread_mutex_unlock(&mutexDeliverer);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		exit(-1);		
	}


	long ordertime_sec = finishorder.tv_sec - startorder.tv_sec;  
	long ordertime_nsec = finishorder.tv_nsec - startorder.tv_nsec;
	long coldtime_sec = finishcolding.tv_sec - startcolding.tv_sec;
	long coldtime_nsec = finishcolding.tv_nsec - startcolding.tv_nsec;
	
	double ordertime = ( ordertime_sec / 60.0 ) + ( ordertime_nsec / 1e9 / 60.0 );
	double coldtime = ( coldtime_sec / 60.0 ) + ( coldtime_nsec / 1e9 / 60.0 );
	
	
	rc = pthread_mutex_lock(&mutexScreen);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		exit(-1);		
	}

	printf("The order with number %d delivered in %f minutes and got cold for %f minutes.\n", threadid, ordertime, coldtime);
	
	rc = pthread_mutex_unlock(&mutexScreen);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		exit(-1);		
	}

	//average and maximum Order Time
	avOrder += ordertime;
	
	if ( maxOrder < (ordertime)){
		maxOrder = ordertime;
	}
	//average and maximum Colding Time
	avCold += coldtime;
	
	if ( maxCold < (coldtime)){
		maxCold = coldtime;
	}
	
	pthread_exit(NULL);
			
}

int main(int argc, char** argv) {
	
	if (argc != 3) {
		printf("ERROR: the program should take two arguments, the number of customers and the original seed.\n");
		exit(-1);
	}
	
	printf("\nWelcome to our pizzeria!\nSelect costumers and seed.\n");

	int Ncust = atoi(argv[1]);
	seed = atoi(argv[2]);
	
	if (Ncust < 0 || seed < 0) {
		printf("ERROR: the number of customers and the seed should be positive numbers. Current numbers given %d.\n", Ncust);
		exit(-1);
	}	
		
	srand(seed);
	
	pthread_t *threads;
	threads = malloc(Ncust * sizeof(pthread_t));
	
	if (threads == NULL) {
		printf("Not enough memory!\n");
		return -1;
	}
	
	int * threadid;
	threadid = malloc(Ncust * sizeof(int));
	
	int rc;
	
	pthread_mutex_init(&mutexCook, NULL);
	pthread_mutex_init(&mutexOven, NULL);
	pthread_mutex_init(&mutexPacker, NULL);
	pthread_mutex_init(&mutexDeliverer, NULL);
	pthread_mutex_init(&mutexScreen, NULL);
	
	pthread_cond_init(&condCook, NULL);
	pthread_cond_init(&condPacker, NULL);
    pthread_cond_init(&condOven, NULL);
	pthread_cond_init(&condDeliverer, NULL);
	
	for(int i = 0; i < Ncust; i++){
		
		threadid[i] = i + 1;										 
		rc = pthread_create(&threads[i], NULL, Order, &threadid[i]);
		if(rc != 0){
			printf("ERROR : creating thread. Return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
		sleep(Random(seed+i, Torderlow, Torderhigh));
	}
	
	// Wait all threads to finish 
	void* status;
	for(int i = 0; i < Ncust; i++){
		
		rc = pthread_join(threads[i], &status);
		if(rc != 0){
			printf("ERROR: joining thread. Return code from pthread_join() is %d\n", rc);
			exit(-1);
		}
	}
	
	printf("Total pizza Sales Revenue: %d euros.\n", totalRev);
	printf("Number of Sales for plain pizzas: %d.\n", sumPlain);
	printf("Number of Sales for special pizzas: %d.\n", sumSpec);
	printf("Total number of successful oreders: %d.\n", sumSucc);
	printf("Total number of failed oreders: %d.\n", sumFail);
	
	avOrder = avOrder / Ncust;
	printf("Average orger time: %f.\n", avOrder);
	printf("Maximum order time: %f.\n", maxOrder);

	avCold = avCold / Ncust;
	printf("Average colding time: %f.\n", avCold);
	printf("Maximum colding time: %f.\n", maxCold);
	
	rc = pthread_mutex_destroy(&mutexCook);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
	}
	rc = pthread_mutex_destroy(&mutexOven);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
	}
	rc = pthread_mutex_destroy(&mutexPacker);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
	}
	rc = pthread_mutex_destroy(&mutexDeliverer);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
	}
	rc = pthread_mutex_destroy(&mutexScreen);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
	}
	rc = pthread_cond_destroy(&condCook);
	if (rc != 0) {
		printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
		exit(-1);		
	}
	rc = pthread_cond_destroy(&condPacker);
	if (rc != 0) {
		printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
		exit(-1);		
	} 
	rc = pthread_cond_destroy(&condOven);
	if (rc != 0) {
		printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
		exit(-1);		
	}
	rc = pthread_cond_destroy(&condDeliverer);
	if (rc != 0) {
		printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
		exit(-1);		
	}

	free(threads); 
    free(threadid); 
	
	return 0;
}

int Random(unsigned int seed, int low, int high) { 

    return (rand_r(&seed) % (high - low + 1)) + low; 
	
} 	

int Propability(unsigned int seed, int percentage) { 

    //Random number calculation from 0 to 100
    int randomValue = Random(seed, 0, 100);

    if (randomValue <= percentage) {
        return 1;  
    } else {
        return 0;  
    }
}
