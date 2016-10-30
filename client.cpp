//
//  main.cpp
//  NetworkProject
//  Client
//
//  Blake Johnson
//  Eric Jackson
//

#include <iostream>
#include <fstream>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <string.h>
#include <sstream>

using namespace std;

void writeFile(char buf[]);
int errorDetection(char buf[], char acknak[]);

//declaring outsream
ofstream outStream;

int main(int argc, char **argv) {
    //creating variables
    int sd, rc, numCheck, errorCheckResult;
    char buf[256];
    char acknak[2];
    
    
    numCheck = 0;
    
    //structure for creating the socket
    struct sockaddr_in server;
    struct hostent *hp;
    
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    
    //getting the hostname
    hp = gethostbyname("tux192.eng.auburn.edu");
    bcopy(hp->h_addr, &(server.sin_addr), hp->h_length);
    server.sin_family = AF_INET;
    server.sin_port = htons(10074);
    
    
    struct sockaddr_in client;
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = htonl(INADDR_ANY);
    client.sin_port = htons(10074);
    
    //binding the socket
    bind(sd, (struct sockaddr *) &client, sizeof(client));
   
    //create the filename to send to the server
    char test[18];
    
    //Telling the server
    strncpy(test, "TestFile_small.txt", 18);
   
    
    //send request to the server 
    sendto(sd, test, sizeof(test), 0, (struct sockaddr *) &server, sizeof(server));
    
    rc = 0;
    //loop to get the packets
    for(;;){
    
        cout << "Receiving\n";
        //receiving the packet
        rc = recv(sd, buf, sizeof(buf), 0);
        
	//Getting the conents of the packet
	for(int p = 7; p < 49; p++) {
        	cout << buf[p];
    	}
	cout << endl;

    //Check if it is the last packet
	if (buf[0] == '\0' && buf[1] == '\0' && buf[2] == '\0' && buf[3] == '\0'){
		break;	
	}
        buf[rc] = (char) NULL;
        
        //check the packet for errors
        errorCheckResult = errorDetection(buf, acknak);
        
        //If there is an error send a nak to the server
        if(errorCheckResult == 1) {
			acknak[0] = '1';
			cout << "ACK/NAK: " << acknak[0] << " Sequence Number: " << acknak[1] << endl;
		}
		
		else{
			acknak[0] = '0';
			cout << "ACK/NAK: " << acknak[0] << " Sequence Number: " << acknak[1] << endl;
			writeFile(buf);
		}
		sendto(sd, acknak, sizeof(acknak), 0, (struct sockaddr *) &server, sizeof(server));
    
        
        
    
        
        
        cout << "\n";
    }
    
    
    return 0;
}

//Funciton to check the packet for errors
int errorDetection(char buf[], char acknak[]) {
	//Variables
	int checkSum, numPackets, sequenceNum, index, nextIndex, pos;
	char headerChecksumArray[5];
	bool checkResult;
	
	index = 0;
	nextIndex = 0;
	pos = 0;
	checkResult = true;
	
	//get the first header info
	for(int i = 0; i < 7; i++){
		if(buf[i] != '0'){
			while(i < 5) {
				headerChecksumArray[index] = buf[i];
				index = 1 + index;
				i = 1 + i;
				pos = 1 + pos;
			}
			while(i < 7) {
				acknak[nextIndex] = buf[i];
				nextIndex = 1 + nextIndex;
				i = 1 + i;
			}
		}
	}
    
	//calculate the checksum of the packet
	for(int s = 7; s < 256; s++) {
		checkSum = checkSum + buf[s];
	}
	    int n = 0;
        while(n < 256){
            
            n++;
        }
	
	//cout << endl << checkSum << endl;
	
    //convert the checksum to a string
	stringstream convert;
	string result;
	convert << checkSum;
	result = convert.str();
	
	
	

	//check the checksum to determine if the packet is corrupt
	for (int i = 0; i < pos; i++){
		if(result[i] != headerChecksumArray[i]) {
			checkResult = false;
		}
	}
	
	
	//returning 0 if the packet received correctly
    //returning 1 if the packet is corrupted
	if(checkResult == true) {
		cout << "Receive Correctly"<< endl;
		return 0;
	}
	else {
		cout <<"Corrupted"<< endl;
		return 1;
	}
		
	
}

//write to file funciton
void writeFile(char buf[]){
    
    string ofile;
    
    //file name
    ofile = "outputfile.txt";
    //open file
    outStream.open((char*)ofile.c_str(), std::ofstream::app);
    
    //check to see if the output failed
    if(outStream.fail()) {
        cout << "Output file opening failed." << endl;
        
    }
    
    for(int t = 7; t < 256; t++) {
        outStream << buf[t];
    }
    outStream.close();
    
    
    
}
