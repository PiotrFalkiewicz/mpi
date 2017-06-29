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
    int hotels[CONST_CITIES];
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

    int interestedOrganisators[numberOfOrganisators];
    int interestedParticipantsTAB[numberOfParticipants];
    for(int i = 0; i < CONST_CITIES; i++){
    	for (int j = 0; j < CONST_PLACES; j++){
    		rooms[i][j]=-1;
    	}
    	hotels[i]=-1;
    	
    }

    int it = 0;
    //CHOOSE TYPE OF USER
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
        	//CHOOSE CITY AND ROOM
        	int cityAndRoom = rand()%(CONST_CITIES*CONST_ROOMS);
        	int city = cityAndRoom/CONST_ROOMS;
        	int room = cityAndRoom%CONST_ROOMS;

        	//SEND TO OTHERS INFORMATION ABOUT CITY AND ROOM
        	for(int i = 0; i < Q_OF_ORGS; i++){
        		if(i != world_rank){
        			MPI_Send(&cityAndRoom, 1,MPI_INT, i, TAG_EVENT_CREATE, MPI_COMM_WORLD);
        		}
        	}

        	bool imOrganiser = true;
        	//SEND MSG TO EVERY OTHER ORGANISER
        	//IF OTHER PROCESS WANTS SAME PLACE -> FIGHT
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
        	//TELL ME SOMETHING ABOUT YOU, PLEASE
        	if (imOrganiser){
        		printf("O: ORGANISER: ID %d CITY %d ROOM %d IT %d\n", world_rank, city, room, it); //org
			}
			else{
				printf("O: WAITING: ID %d CITY %d ROOM %d IT %d\n", world_rank, city, room, it); //wait
				cityAndRoom = -1;
			}
        		
        	//SEND INVITATIONS TO EVERY PARTICIPANT
        	for(int i = numberOfOrganisators; i < world_size; i++){
        		MPI_Send(&cityAndRoom, 1,MPI_INT, i, TAG_EVENT_INVITATION, MPI_COMM_WORLD);
        	}
        	int interestedParticipants = 0;

        	//RECEIVE ANSWERS
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

        	//PRINT SUMMARY
        	printf("O: EVENT. ID: %d Q: %d IT: %d\n",world_rank,interestedParticipants,it);

        	
        	if(imOrganiser){
        		//IF I'M ORGANISER I HAVE TO FREE THESE ORGANISERS WHO ARE WAITING
        		//I HAVE ALSO TO FREE PARTICIPANTS
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
        		//I'VE TO BE FREED
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
			//CONTACT WITH ORGANISERS TO DETECT INVITATIONS
			for(int i = 0; i < numberOfOrganisators; i++){
				MPI_Status status;
				int response;
				MPI_Recv(&response, 1, MPI_INT, MPI_ANY_SOURCE, TAG_EVENT_INVITATION, MPI_COMM_WORLD, &status);
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
			//CHOOSE EVENT I WANT TO GO ON
			int decision = -1;
			for(int i = 0; i < numberOfOrganisators; i++){
				if(i == interested){
					decision = cityAndRoom;
				}else{
					decision = -1;
				}
				MPI_Send(&decision, 1,MPI_INT, i, TAG_EVENT_RESPONSE, MPI_COMM_WORLD);
			}

			printf("P: EVENT. ID: %d ORG: %d IT: %d\n",world_rank,interested,it);

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
			//SEND TO OTHER PARTICIPANTS MY BELOVED EVENT
			for(int i = numberOfOrganisators; i < world_size; i++){
				MPI_Send(&city, 1,MPI_INT, i, TAG_PLACE_RESERVE, MPI_COMM_WORLD);
			}
			//MAYBE I PREDESTINATED FOR PARTICIPATING IN EVENT?
			if(world_rank - numberOfOrganisators < CONST_PLACES){
				hotels[city]++;
				trigger = true;
			}
			//FIGHT FOR PLACE IN HOTEL
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

			//I'M FINALLY HERE
			if((!trigger) && (otherInterested < CONST_PLACES)){
				hotels[city]++;
				trigger = true;
			}

			if(trigger){
				//I HAVE PLACE FOR MYSELF
				printf("P: HOTEL ID: %d CITY: %d IT: %d \n",world_rank,city,it);
			}else{
				//I DON'T HAVE PLACE IN HOTEL
				printf("P: NO-HOTEL ID: %d Q: %d CITY: %d IT: %d \n",world_rank,otherInterested,city,it);
			}

			//RECEIVING MESSAGE FROM EVERY ORGANISATOR
			for(int i = 0; i < realOrganisatorsQ; i++){
				int answer;
        		MPI_Recv(&answer, 1, MPI_INT, MPI_ANY_SOURCE, TAG_EVENT_END, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        		if((answer == interested)&&(trigger)){
        			hotels[city]--;
        		}
			}
			//NOW I CAN WALK AROUND THESE BEAUTIFUL CITY
			int time = rand()%3;
        	printf("P: EVENT FINISHED. I'M WAITING ID: %d TIME: %d IT: %d\n",world_rank, time, it);
        	sleep(time);
		}
        
    
	
	}
    // Finalize the MPI environment.
    MPI_Finalize();
}