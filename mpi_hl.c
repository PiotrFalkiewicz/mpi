#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#define Q_OF_ORGS 3

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




void organize(int world_rank, int world_size) {
    int msg[2];
    srand(world_rank);
    msg[0] = rand() % CONST_CITIES;
    msg[1] = rand() % CONST_ROOMS;

    for (int i = 0; i < world_size; i++) {
        if (i != world_rank) MPI_Send(&msg, sizeof(msg), MPI_INT, i, TAG_EVENT_CREATE, MPI_COMM_WORLD);
    }
    for (int i = 0; i < world_size - 1; i++) {
        MPI_Status status;
        MPI_Recv(&msg, sizeof(msg), MPI_INT, MPI_ANY_SOURCE, TAG_EVENT_RESPONSE, MPI_COMM_WORLD, &status);

    }

}


int main(int argc, char **argv) {
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
    for(int i = 0; i < CONST_CITIES; i++){
    	for (int j = 0; j < CONST_PLACES; j++){
    		rooms[i][j]=-1;
    	}
    	for (int k = 0; k < CONST_ROOMS; k++){
    		hotels[i][k]=-1;
    	}
    }
    int type[world_size];


    while (true) {
        if (world_rank < Q_OF_ORGS) {
            type[world_rank] = USER_ORGANIZATOR;
        } else {
            type[world_rank] = USER_PARTICIPANT;
        }
        printf("%d type: %d \n",world_rank,type[world_rank]);
        if (type[world_rank] == USER_ORGANIZATOR) {
        	srand(time(NULL));
        	int cityAndRoom = rand()%(CONST_CITIES*CONST_ROOMS);
        	int city = cityAndRoom/CONST_ROOMS;
        	int room = cityAndRoom%CONST_ROOMS;
        	printf("I'm %d and I want to make my event in city %d room %d\n",world_rank,city,room);
        	if((world_rank == city)&&(world_rank== room)){
        		printf("I'm making event in %d city in %d room\n",city,room);
        		for (int i = Q_OF_ORGS; i < world_size; i++){
        			MPI_Send(&cityAndRoom, 1,MPI_INT, i, TAG_EVENT_CREATE, MPI_COMM_WORLD);	
        		}
        		
        	}
        	//get city
        	//get room
            //organize(world_rank, world_size);
            //do organizator stuff
        } else {
            //recv anysource, anytag
            MPI_Status status;
            int cityAndRoom;
            MPI_Recv(&cityAndRoom, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            int city = cityAndRoom/CONST_ROOMS;
            int room = cityAndRoom%CONST_ROOMS;
            printf("My ID: %d, Sender ID: %d, city: %d, room: %d\n",world_rank,status.MPI_SOURCE,city,room);
            //analyze(world_rank, status.MPI_SOURCE, status.MPI_SOURCE);
            //participant stuff
        }
        sleep(2);
    }

    // Finalize the MPI environment.
    MPI_Finalize();
}