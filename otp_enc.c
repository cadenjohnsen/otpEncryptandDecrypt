/*
* this file sends the text to be encrypted, encryption key, and port to otp_enc_d
* then it outputs the encrypted text to stdout
* similar to client.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

void delay(int seconds) {
	int milliseconds = seconds * 1000;	// convert seconds to milliseconds
	clock_t start_time = clock();

	while(clock() < start_time + milliseconds);	// pause the program for a specified time in milliseconds
}

int main(int argc, char const *argv[]) {
	int i, j;
	int size = 0;
  int socketFD, portNumber, charsWritten, charsRead;
  char ciphertext[100000];
  char plaintext[100000];
  char key[100000];
	char buffer[50000];
  char* hostName = "localhost";
  struct sockaddr_in serverAddress;
  struct hostent* serverHostInfo;
  FILE *fpplaintext;
	FILE *fpkey;

  if (argc < 4) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");// Set up the socket

	memset(buffer, '!', sizeof(buffer)); // Clear out the buffer array with specified bad character

	fpplaintext = fopen(argv[1], "r");	// open plaintext file
	fpkey = fopen(argv[2], "r");	// open key file

	memset(plaintext, '\0', sizeof(plaintext)); // Clear out the plaintext array
	fgets(plaintext, sizeof(plaintext) - 1, fpplaintext); // Get input from the user, trunc to plaintext - 1 chars, leaving \0
	plaintext[strcspn(plaintext, "\n")] = '\0'; // Remove the trailing \n that fgets adds

	memset(key, '\0', sizeof(key)); // Clear out the key array
	fgets(key, sizeof(key) - 1, fpkey); // Get input from the user, trunc to key - 1 chars, leaving \0
	key[strcspn(key, "\n")] = '\0'; // Remove the trailing \n that fgets adds

	for(i = 0; i < strlen(plaintext); i++) {	// loop through all characters in plaintext
		if (plaintext[i] < 65 && plaintext[i] != 32 || plaintext[i] > 90) {	// if character is not a letter or space
			fprintf(stderr, "ERROR plaintext file has invalid character\n");	// print error
			fclose(fpplaintext);	// close plaintext file
			fclose(fpkey);	// close key file
			exit(1);	// exit with value 1
		}
	}
	for(i = 0; i < strlen(key); i++) {	// loop through all characters in plaintext
		if (key[i] < 65 && key[i] != 32 || key[i] > 90) {	// if character is not a letter or space
			fprintf(stderr, "ERROR key file has invalid character\n");	// print error
			fclose(fpplaintext);	// close plaintext file
			fclose(fpkey);	// close key file
			exit(1);	// exit with value 1
		}
	}
	if(strlen(key) < strlen(plaintext)) {	// checks if plaintext is longer than key
		fprintf(stderr, "ERROR key is smaller than plaintext\n");	// print error
		fclose(fpplaintext);	// close plaintext file
		fclose(fpkey);	// close key file
		exit(1);	// exit with value 1
	}

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
		error("CLIENT: ERROR connecting"); // Connect socket to address
	}

	// while (buffer != NULL) {
		memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
		if(strlen(plaintext) < 50000) {	// check if the text file to send is less than available size to send
			for(i = 0; i < strlen(plaintext); i++) {	// loop through size of plaintext
				buffer[i] = plaintext[i];	// make first half of buffer become plaintext file
				buffer[i + strlen(plaintext)] = key[i];	// make second half of buffer key file
			}

			// Send message to server
			charsWritten = send(socketFD, buffer, strlen(buffer), 0); // Write to the server
			if (charsWritten < 0) error("CLIENT: ERROR writing to socket");	// error if writing does not work
			if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");	// error if not all data has been written but it has stopped ending

			// Get return message from server
			memset(ciphertext, '\0', sizeof(ciphertext)); // Clear out the ciphertext again for reuse
			charsRead = recv(socketFD, ciphertext, sizeof(ciphertext) - 1, 0); // Read data from the socket, leaving \0 at end
			if (charsRead < 0) error("CLIENT: ERROR reading from socket");	// error if writing does not work
			if(ciphertext != NULL) {	// if the message received is not NULL
				printf("%s\n", ciphertext);	// print to stdout the received text
			}
		}

	fclose(fpplaintext);	// close the plaintext file
  fclose(fpkey);	// close the key file
	close(socketFD); // Close the socket

  return 0;
}
