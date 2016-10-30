#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <strings.h>
#include <sstream>
using namespace std;

//Constants and functions
const int PACK_SIZE = 256;
const int HEAD_SIZE = 7;
int getFileSize(const char* filename);
int segmentation(char file[], char packet[], int StartPosition, int fileSize);
void errorDetection(char packet[], char sequence);
bool Gremlin(char packet[], char copyPacket[]);

int main ()
{
   // Set up the server object (local machine -- INADDR_ANY)
   struct sockaddr_in server;
   server.sin_family = AF_INET;
   server.sin_addr.s_addr = htonl(INADDR_ANY);
   server.sin_port = htons(10074);
   
   // Set up the client object
   struct sockaddr_in client;
   struct hostent *hp;
   hp = gethostbyname("tux191.eng.auburn.edu");
   bcopy(hp->h_addr, &(client.sin_addr), hp->h_length);
   client.sin_family = AF_INET;
   client.sin_port = htons(10074);
   
   // Create the socket
   // AF_INET = protocol family (Internet Protocol)
   // SOCK_DGRAM = communication type (UDP)
   // 0 = a particular protocol in the family
   int sd = socket(AF_INET, SOCK_DGRAM, 0);
   
   // Binding the socket to the local machine
   bind(sd, (struct sockaddr *) &server, sizeof(server));
   
   // Extra Variables
   bool endOfFile = false;
   char packet[PACK_SIZE];
   char msg[100];
   int numBytes = 0;
   int position = 0;
   int checkSum = 0;
   char ack_nak[2] = {'0', '0'};
   char null[4] = {'\0', '\0', '\0', '\0'};
   char copyPacket[PACK_SIZE];
   char sequenceNum = '0';
   bool lostPacket = false;
   struct timeval tv;
   
   // Receive GET request
   numBytes = recv(sd, msg, sizeof(msg), 0);
   cout << "Message Received\n";
   msg[numBytes] = '\0';
   
   // Set the filename based on what the GET message contained
   const char* filename = msg;
   
   // Get the size of the file
   cout << "Getting the size of: " << filename << "\n";
   int fileSize = getFileSize(filename);
   cout << "File size: " << fileSize << "\n";   
   
   // Char array that the file data will be stored in
   char wholeFile[fileSize];  
   // Open the file to read it
   FILE *file = fopen(filename, "rb");
   if (file == NULL)
   {
      cout << "File opening failed";
   }
   
   // Read the bytes and store in char array
   fread(wholeFile, 1, fileSize, file);
   
   fclose(file);
   
   while(!endOfFile)
   {
		cout << "Current ack/nak: " << ack_nak[0] << " Current sequence number: " << sequenceNum << "\n";
		if (ack_nak[0] == '0' && ack_nak[1] == sequenceNum && !lostPacket)
		{
			if(sequenceNum == '0')
			{
				sequenceNum = '1';
			}
			else
			{
				sequenceNum = '0';
			}

			// Break the file into packet
			position = segmentation(wholeFile, packet, position, fileSize);         

			// Calculate the check sum and add header to the packet
			errorDetection(packet, sequenceNum);

			// Pass packet through gremlin
			lostPacket = Gremlin(packet, copyPacket);

			if(!lostPacket)
			{
				// Send the packet
				cout << "Sending Packet\n";
				for(int i = 0; i < 47; i++)
				{
					cout << packet[i];
				}
				cout << "... ";
				sendto(sd, packet, sizeof(packet), 0, (struct sockaddr *) &client, sizeof(client));
			}
			else
			{
				cout << "Packet Lost\n";
			}

		}
		else
		{
			// Resend the packet
			cout << "Resending Packet\n";
			sendto(sd, copyPacket, sizeof(copyPacket), 0, (struct sockaddr *) &client, sizeof(client));
			lostPacket = false;
		}

		// Wait for ACK -- if ACK start with next packet -- if NAK resend the same packet
		cout << "Waiting for ACK\n";
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
		numBytes = recv(sd, ack_nak, sizeof(ack_nak), 0);

		// Check if this is the end of file -- if it is set endOfFile to true
		if(position > fileSize && (ack_nak[0] == '0'))
		{
			endOfFile = true;
		}
   }
   
   // Send NULL char to client to signal end of transmission
   cout << "End of file has been reached\n";
   cout << "Sending null char to end transmission\n";
   sendto(sd, null, sizeof(null), 0, (struct sockaddr *) &client, sizeof(client));
   
   // Close the socket
   close(sd);
}

// Seeks to the end of the file and returns the size in bytes
int getFileSize(const char* filename)
{
   FILE *file = fopen(filename, "rb");
   if (file == NULL)
   {
      cout << "File opening failed";
   }
   fseek(file, 0, SEEK_END);
   int size = ftell(file);
   fclose(file);
   return size;
}

// Breaks the file into smaller packets and returns the finishing position in the file
int segmentation(char file[], char packet[], int StartPosition, int fileSize)
{
   for (int i = 0; i < PACK_SIZE - HEAD_SIZE; i++)
   {
      if(StartPosition + i > fileSize - 1)
      {
         packet[i] = '\0';
      }
      else
      {
         packet[i] = file[StartPosition + i];
      }
   }
   
   return StartPosition + (PACK_SIZE - HEAD_SIZE);
}

bool Gremlin(char packet[], char copyPacket[]){
    //this might be user input?-------------------------
    int lostProb = 20;
    int damageProb = 50;
    bool lost = false;  
    
    //copy packet
    for (int i = 0; i < PACK_SIZE; i++) {
        copyPacket[i] = packet[i];
    }
    
    //generate random number for damage probability
    int percent = rand() % 100;
    
    if(damageProb > percent){
        int randomIndex = rand() % 256;
        int secRandomIndex = rand() % 256;
        int thirdRandomIndex = rand() % 256;
        int damageNum = rand() % 100;
        
        //one byte is damaged
        if (damageNum >= 0 && damageNum < 70) {
            packet[randomIndex] = '\0';
        }
        
        //two bytes are damaged
        else if(damageNum >= 70 && damageNum < 90){
            
            packet[randomIndex] = '\0';
            packet[secRandomIndex] ='\0';
        }
        
        //threee bytes are damaged
        
        else{
            packet[randomIndex] = '\0';
            packet[secRandomIndex] ='\0';
            packet[thirdRandomIndex] = '\0';
            
        }
    }

    //generate random number for lost probability
    int percentLost = rand() % 100;
    if(lostProb > percentLost)
    {
	lost = true;
    }

    return lost;
}

// Calculates the checksum of current packet
void errorDetection(char packet[], char sequence)
{
   int checkSum = 0;
   for (int i = 0; i < PACK_SIZE - HEAD_SIZE; i++)
   {
      checkSum = checkSum + packet[i];
   }
   
   // Shift packet bytes down to make room for header
   for (int i = PACK_SIZE - (HEAD_SIZE + 1); i >= 0; i--)
   {
      packet[i + HEAD_SIZE] = packet[i];
      packet[i] = '0';
   }   
   
   // Convert checkSum to a string
   stringstream convert;
   string result;
   convert << checkSum;
   result = convert.str();
   
   int packIndex = 4;
   // Add header bytes to front of packet
   for (int i = result.size() - 1; i >= 0; i--)
   {
      packet[packIndex] = result[i];
      packIndex--;
   }
   
   // ACK/NAK and sequence number
   packet[5] = '1';
   packet[6] = sequence;
}
