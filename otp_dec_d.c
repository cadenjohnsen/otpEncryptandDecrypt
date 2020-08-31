/*
* waits in background for opt_dec to be called and then
* receives plaintext and cipher over specified port
* decrypts the ciphertext and returns it to opt_dec
* similar to server.c
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

char* decryptMessage(char ciphertext[25000], char key[25000]) {
	int i, j, temp;
	int modPlainText[25000];
	int modKey[25000];
	int modCipherText[25000];
	char* tempString;
	char tempChar;
	char plaintext[25000];

	tempString = malloc(25000 * sizeof(char));	// allocate memory for tempString
	memset(tempString, '\0', 25000);	// clear the memory

	for(i = 0; i < strlen(ciphertext); i++) {	// loop through plaintext string
		modCipherText[i] = ciphertext[i];	// turn the character into a numerical value
		if(modCipherText[i] == 32) {	// if the character is a space move it into the last place on the index
			modCipherText[i] = 26;
		}
		else if(modCipherText[i] == 0) {	// if the character is NULL make it an impossible value
			modCipherText[i] = 49;
		}
		else if(modCipherText[i] > 64 && modCipherText[i] < 91) {	// if the character is a valid letter then bring it down to 27 modulo table
			modCipherText[i] = modCipherText[i] - 65;
		}
		else {
			fprintf(stderr, "ERROR ciphertext file has invalid character: %c\n", ciphertext[i]);	// print error for invalid character
		}
		modKey[i] = key[i];	// turn the character into a numerical value
		if(modKey[i] == 32) {	// if the character is a space move it into the last place on the index
			modKey[i] = 26;
		}
		else if(modKey[i] == 0) {	// if the character is NULL make it an impossible value
			modKey[i] = 49;
		}
		else if (modKey[i] > 64 && modKey[i] < 91) {	// if the character is a valid letter then bring it down to 27 modulo table
			modKey[i] = modKey[i] - 65;
		}
		modPlainText[i] = modCipherText[i] - modKey[i];	// subtract the plaintext and key values together
		if(modPlainText[i] < 0) {	// if it is a negative number turn it back into a character
			modPlainText[i] += 27;
		}
		if(modPlainText[i] > 26) {	// if it is larger than the modulo subtract 27 to turn it into a character
			modPlainText[i] -= 27;
		}
		tempChar = modPlainText[i];	// assign the numerical value into a character
		if(modCipherText[i] < 27) {	// if the character is a space then set it to space
			if(tempChar == 26) {
				tempChar = 32;
			}
			else {	// if the character is a letter add 65 to make it a capital letter
				tempChar += 65;
			}
			tempString[i] = tempChar;	// add the character into the tempString

		}
	}
	tempString[++i] = '\0';	// make the last character NULL

	return tempString;	// return the encrypted string
}

int main(int argc, char const *argv[]) {
	int i, j, temp;
	int size = 0;
  int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char ciphertext[25000];
  char key[25000];
	char buffer[50000];
	char* plaintext;
	struct sockaddr_in serverAddress, clientAddress;

	plaintext = malloc(25000 * sizeof(char));	// allocate memory for plaintext

  if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

  // Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 6); // Flip the socket on - it can now receive up to 6 connections

while(1) {
		// Accept a connection, blocking if one is not available until one connects
		sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
		establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
		if (establishedConnectionFD < 0) error("ERROR on accept");

			memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
			memset(ciphertext, '\0', sizeof(ciphertext)); // Clear out the ciphertext again for reuse
			memset(plaintext, '\0', sizeof(plaintext)); // Clear out the plaintext again for reuse
			memset(key, '\0', sizeof(key)); // Clear out the ciphertext again for reuse

			// Get the message from the client
			charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer) - 1, 0); // Read the client's message from the socket
			if (charsRead < 0) error("ERROR reading from socket");

			size = strlen(buffer)/2;	// cut the buffer in half
			for(i = 0; i < size; i++) {	// loop through half of the buffer
				ciphertext[i] = buffer[i];	// first half of buffer goes into ciphertext
				key[i] = buffer[i + size];	// last half of buffer goes into key
			}

			if(strlen(key) < strlen(ciphertext)) {	// check if key is longer than ciphertext
				fprintf(stderr, "ERROR key is smaller than ciphertext\n");	// print error
			}

			strcpy(plaintext, decryptMessage(ciphertext, key));	// decrypt the message and return into plaintext

			// Send cipher back to the client
			charsRead = send(establishedConnectionFD, plaintext, strlen(plaintext), 0); // Send success back
			if (charsRead < 0) error("ERROR writing to socket");

		close(establishedConnectionFD); // Close the existing socket which is connected to the client
	}

	close(listenSocketFD); // Close the listening socket
	free(plaintext);	// free allocated memory

  return 0;
}
