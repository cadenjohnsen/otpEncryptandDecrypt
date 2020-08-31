/*
* file creates a key with random characters of a specified length
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[]) {
  int i, j;
  int length;
  char* key = NULL;
  char tempChar;

  srand(time(NULL));

  if(argc == 2) { // check if there are two command line arguments
    if(atoi(argv[1]) == 0) {  // check if the second command line argument is not an integer
      fprintf(stderr, "ERROR Please enter a valid key length\n");
      exit(1);
    }
    else {
      length = atoi(argv[1]); // get the specified length
      key = (char*) malloc((length + 1) * sizeof(char));  // allocate memory for string

      for(i = 0; i < length; i++) { // loop through length of string
        if(rand() % 27 == 20) { // 1/27 chance the character is a space
          tempChar = 32;  // make that character a space
        }
        else {
          tempChar = (rand() % 26) + 65;  // pick a random ASCII value for a capital letter
        }

        key[i] = tempChar;  // append it to the next character in string
      }
      key[i] = '\0';  // terminate the string
      printf("%s\n", key); // display the given key
      free(key);  // free memory allocated
    }
  }
  else {  // if there is no second command line argument
    fprintf(stderr, "ERROR Please enter a valid key length\n");
    exit(1);
  }

  return 0;
}
