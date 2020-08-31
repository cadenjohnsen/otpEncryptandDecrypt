/*
* this file sends the text to be decrypted, decryption key, and port to otp_dec_d
* then it outputs the decrypted text to stdout
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
  FILE *fpciphertext;
	FILE *fpkey;

  if (argc < 4) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args

  // Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname(hostName); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	memset(buffer, '!', sizeof(buffer)); // Clear out the plaintext array

	fpciphertext = fopen(argv[1], "r");
	fpkey = fopen(argv[2], "r");

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

		memset(ciphertext, '\0', sizeof(ciphertext)); // Clear out the plaintext array
		fgets(ciphertext, sizeof(ciphertext) - 1, fpciphertext); // Get input from the user, trunc to ciphertext - 1 chars, leaving \0
		ciphertext[strcspn(ciphertext, "\n")] = '\0'; // Remove the trailing \n that fgets adds

		memset(key, '\0', sizeof(key)); // Clear out the key array
		fgets(key, sizeof(key) - 1, fpkey); // Get input from the user, trunc to key - 1 chars, leaving \0
		key[strcspn(key, "\n")] = '\0'; // Remove the trailing \n that fgets adds

		for(i = 0; i < strlen(ciphertext); i++) {	// check if cipher has invalid character
			if (ciphertext[i] < 65 && ciphertext[i] != 32 || ciphertext[i] > 90) {	// if character is not a space or capital letter
				fprintf(stderr, "ERROR ciphertext file has invalid character\n");	// print error
				fclose(fpciphertext);	// close ciphertext file
			  fclose(fpkey);	// close key file
				exit(1);	// exit with value 1
			}
		}
		for(i = 0; i < strlen(key); i++) {	// check if the key has invalid character
			if (key[i] < 65 && key[i] != 32 || key[i] > 90) {	// if character is not a space or capital letter
				fprintf(stderr, "ERROR key file has invalid character\n");	// print error
				fclose(fpciphertext);	// close the ciphertext file
			  fclose(fpkey);	// close the key file
				exit(1);	// exit with value 1
			}
		}
		if(strlen(key) < strlen(ciphertext)) {	// check if ciphertext is longer than key
			fprintf(stderr, "ERROR key is smaller than ciphertext\n");	// print error
			fclose(fpciphertext);	// close ciphertext file
		  fclose(fpkey);	// close key file
			exit(1);	// exit with value 1
		}

		// while (buffer != NULL) {
		memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array

		if(strlen(ciphertext) < 25000) {	// if cipher is under size of one send
			for(i = 0; i < strlen(ciphertext); i++) {	// loop through buffer
				buffer[i] = ciphertext[i];	// set first half of buffer to ciphertext
				buffer[i + strlen(ciphertext)] = key[i];	// set second half of buffer to key
			}

			// Send message to server
			charsWritten = send(socketFD, buffer, strlen(buffer), 0); // Write to the server
			if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
			if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");

			// Get return message from server
			memset(plaintext, '\0', sizeof(plaintext)); // Clear out the plaintext again for reuse
			charsRead = recv(socketFD, plaintext, sizeof(plaintext) - 1, 0); // Read data from the socket, leaving \0 at end
			if (charsRead < 0) error("CLIENT: ERROR reading from socket");
			printf("%s\n", plaintext);	// send decrypted message to stdout
		}

	fclose(fpciphertext);	// close ciphertext file
  fclose(fpkey);	// close key file
	close(socketFD); // Close the socket

  return 0;
}
