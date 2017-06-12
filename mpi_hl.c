#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#define CONST_CITIES 3
#define CONST_ROOMS 5
#define CONST_PLACES 20

//mpi msg tags
#define TAG_EVENT_CREATE 0 //city, room
#define TAG_EVENT_RESPONSE 1 //event msg
#define TAG_EVENT_FINISH 2 //city, room
#define TAG_PLACE_RESERVE 3 //city
#define TAG_PLACE_RESPONSE 4 //place 
#define TAG_PLACE_RELEASE 5 //city
 
//user types
#define USER_ORGANIZATOR 6
#define USER_PARTICIPANT 7
 
//event msgs
#define MSG_EVENT_FREE 8 //
#define MSG_EVENT_INTERESTED 9
#define MSG_EVENT_NO_INTERESTED 10
 
//place msgs
#define MSG_PLACE_PRESENCE 11
#define MSG_PLACE_INTERESTED 12
#define MSG_PLACE_NO_INTERESTED 13

int main(int argc, char** argv) {
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);
    // Get the number of processes
    int world_size;
    int world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	//init cities etc

	int rooms[CONST_CITIES][CONST_ROOMS];
	int hotels[CONST_CITIES][CONST_PLACES];

	int type[world_size];


   	while(true){
   		srand(world_rank);
   		type[world_rank] = rand()%(world_rank+1);
	   	if (type[world_rank] == 0){
	   		type[world_rank] = USER_ORGANIZATOR;
	   	} else {
	   		type[world_rank] = USER_PARTICIPANT;
	   	}

	   	if(type[world_rank] == USER_ORGANIZATOR){
	   		organize(world_rank);
	   		//do organizator stuff
		}
		else{
			//recv anysource, anytag
			MPI_Status status;
	   		MPI_Recv(&places, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	   		analyze(world_rank,status.MPI_SOURCE,status.MPI_SOURCE);
	   		//participant stuff
		}
	}  		
	
    // Finalize the MPI environment.
    MPI_Finalize();
}