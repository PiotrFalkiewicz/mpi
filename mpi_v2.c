#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#define Q_OF_ORGS 3 //N

#define CONST_CITIES 1
#define CONST_ROOMS 2
#define CONST_PLACES 2

//mpi msg tags
#define TAG_EVENT_CREATE 0 //city, room
#define TAG_EVENT_INVITATION 1 //city,room
#define TAG_EVENT_RESPONSE 2

#define MSG_EVENT_INTERESTED 3
#define MSG_EVENT_NO_INTERESTED 4

#define TAG_EVENT_END 5 

#define TAG_PLACE_RESERVE 10 //city

//user types
#define USER_ORGANIZATOR 101
#define USER_PARTICIPANT 102






int main(int argc, char **argv) {
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);
    // Get the number of processes
    int world_size;
    int world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    //init cities etc

    int type[world_size];
    int numberOfOrganisators = Q_OF_ORGS;
    int numberOfParticipants = world_size - Q_OF_ORGS;
    int rooms[CONST_CITIES][CONST_ROOMS];
    int hotels[CONST_CITIES];
    int interestedOrganisators[numberOfOrganisators];
    int interestedParticipantsTAB[numberOfParticipants];
    for(int i = 0; i < CONST_CITIES; i++){
    	for (int j = 0; j < CONST_PLACES; j++){
    		rooms[i][j]=-1;
    	}
    	hotels[i]=-1;
    	
    }

    int it = 0;
    if (world_rank < Q_OF_ORGS) {
        type[world_rank] = USER_ORGANIZATOR;
    } else {
        type[world_rank] = USER_PARTICIPANT;
    }
    while (true) {
    	for(int i = 0; i < numberOfOrganisators; i++){
    		interestedOrganisators[i] = 0;
    	}
	    for(int i = 0; i < numberOfParticipants; i++){
	    	interestedParticipantsTAB[i] = 0;
	    }
    	it++;

        if (type[world_rank] == USER_ORGANIZATOR) {
        	srand(world_rank+it);
        	int cityAndRoom = rand()%(CONST_CITIES*CONST_ROOMS);
        	int city = cityAndRoom/CONST_ROOMS;
        	int room = cityAndRoom%CONST_ROOMS;


        	for(int i = 0; i < Q_OF_ORGS; i++){
        		if(i != world_rank){
        			MPI_Send(&cityAndRoom, 1,MPI_INT, i, TAG_EVENT_CREATE, MPI_COMM_WORLD);
        		}

        	}
        	bool imOrganiser = true;
        	for(int i = 0; i < numberOfOrganisators; i++){
        		if(i != world_rank){
	            	MPI_Status status;
	        		int cityAndRoomRCV;
	            	MPI_Recv(&cityAndRoomRCV, 1, MPI_INT, MPI_ANY_SOURCE, TAG_EVENT_CREATE, MPI_COMM_WORLD, &status);
					if(cityAndRoom == cityAndRoomRCV){
						interestedOrganisators[status.MPI_SOURCE] = 1;
						if(world_rank > status.MPI_SOURCE){
							imOrganiser = false;
						}
					}
				}

        	}
        	if (imOrganiser){
        		printf("E: ID %d CITY %d ROOM %d IT %d\n", world_rank, city, room, it); //org
			}
			else{
				printf("W: ID %d CITY %d ROOM %d IT %d\n", world_rank, city, room, it); //wait
				cityAndRoom = -1;
			}
        		
        	//send invitations

        	for(int i = numberOfOrganisators; i < world_size; i++){
        		MPI_Send(&cityAndRoom, 1,MPI_INT, i, TAG_EVENT_INVITATION, MPI_COMM_WORLD);
        	}
        	int interestedParticipants = 0;
        	//receive answers
        	for(int i = numberOfOrganisators; i < world_size; i++){
        		int response;
        		MPI_Recv(&response, 1, MPI_INT, MPI_ANY_SOURCE, TAG_EVENT_RESPONSE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        		if (response == cityAndRoom){
        			interestedParticipantsTAB[i - numberOfOrganisators] = 1;
        		}
        	}
        	for(int i = 0; i < numberOfParticipants; i++){
        		if(interestedParticipantsTAB[i-numberOfOrganisators] == 1){
        			interestedParticipants++;
        		}
        	}

        	//summary
        	printf("INTERESTED ID: %d Q: %d IT: %d\n",world_rank,interestedParticipants,it);



        	//finish
        	if(imOrganiser){
        		int a = 0;
	        	for(int i = 0; i < numberOfOrganisators; i++){
	        		if (interestedOrganisators[i] == 1) MPI_Send(&a, 1,MPI_INT, i, TAG_EVENT_END, MPI_COMM_WORLD);
	        	}
	        	
	        	for(int i = 0; i < numberOfParticipants; i++){
	        		if (interestedParticipantsTAB[i] == 1) {
	        			a = cityAndRoom;
	        		}
	        		MPI_Send(&a, 1,MPI_INT, i + numberOfOrganisators, TAG_EVENT_END, MPI_COMM_WORLD);
	        	}
        	}else{
        		int answer;
        		MPI_Recv(&answer, 1, MPI_INT, MPI_ANY_SOURCE, TAG_EVENT_END, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        	}



        } else {
        	srand(world_rank);
        	//choose one i'm interested in
        	int interested = rand()%numberOfOrganisators;
        	int cityAndRoom = -1;
			//wait for n invitations
			int otherInterestingID;
			int otherInteresting;
			int realOrganisatorsQ = 0;
			for(int i = 0; i < numberOfOrganisators; i++){
				MPI_Status status;
				int response;
				MPI_Recv(&response, 1, MPI_INT, MPI_ANY_SOURCE, TAG_EVENT_INVITATION, MPI_COMM_WORLD, &status);
				//printf("ID %d waiting IT %d\n",world_rank,it);
				if(interested == status.MPI_SOURCE){
					cityAndRoom = response;
				}
				if(response != -1){
					otherInterestingID = i;
					otherInteresting = response;
					realOrganisatorsQ++;
				}
			}
			if(cityAndRoom == -1){
				interested = otherInterestingID;
				cityAndRoom = otherInteresting;
			}

			int decision = -1;
			for(int i = 0; i < numberOfOrganisators; i++){
				if(i == interested){
					decision = cityAndRoom;
				}else{
					decision = -1;
				}
				MPI_Send(&decision, 1,MPI_INT, i, TAG_EVENT_RESPONSE, MPI_COMM_WORLD);
			}

			printf("INTERESTINGS ID: %d ORG: %d IT: %d\n",world_rank,interested,it);

			//fight for place at hotel
			//
			//make it here
			//

			//send to every participant city id
			//if my id < places in hotel -> take place
			//receive from every participants his city id
			//if number of id's interested < places in hotel -> take place
			//else resign from event and wait for another inviting


			int city = cityAndRoom/CONST_PLACES;
			int otherInterested = 0;
			bool trigger = false;
			for(int i = numberOfOrganisators; i < world_size; i++){
				MPI_Send(&city, 1,MPI_INT, i, TAG_PLACE_RESERVE, MPI_COMM_WORLD);
				//printf("PS ID: %d P: %d IT: %d \n",world_rank,city,it);
			}
			if(world_rank - numberOfOrganisators < CONST_PLACES){
				hotels[city]++;
				trigger = true;
			}

			for(int i = numberOfOrganisators; i < world_size; i++){
				if(!trigger){
					int answer;
	        		MPI_Recv(&answer, 1, MPI_INT, MPI_ANY_SOURCE, TAG_PLACE_RESERVE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	        		if(answer == city){
	        			otherInterested++;
	        		}
				}else{
					int answer;
	        		MPI_Recv(&answer, 1, MPI_INT, MPI_ANY_SOURCE, TAG_PLACE_RESERVE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				}
			}

			printf("INT ID: %d Q: %d IT: %d \n",world_rank,otherInterested,it);

			if((!trigger) && (otherInterested < CONST_PLACES)){
				hotels[city]++;
				trigger = true;
			}

			if(trigger){
				printf("HOTEL ID: %d CITY: %d IT: %d \n",world_rank,city,it);
			}else{
				printf("NO-HOTEL ID: %d Q: %d CITY: %d IT: %d \n",world_rank,otherInterested,city,it);
			}



			
			for(int i = 0; i < realOrganisatorsQ; i++){
				int answer;
        		MPI_Recv(&answer, 1, MPI_INT, MPI_ANY_SOURCE, TAG_EVENT_END, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        		if((answer == interested)&&(trigger)){
        			int time = rand()%3;
        			printf("WAITING ID: %d TIME: %d IT: %d\n",world_rank, time, it);
        			sleep(time);
        			hotels[city]--;
        		}
			}
		}
        
    
	
	}
    // Finalize the MPI environment.
    MPI_Finalize();
}