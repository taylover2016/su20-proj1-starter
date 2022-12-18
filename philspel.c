/*
 * Include the provided hash table library.
 */
#include "hashtable.h"

/*
 * Include the header file.
 */
#include "philspel.h"

/*
 * Standard IO and file routines.
 */
#include <stdio.h>

/*
 * General utility routines (including malloc()).
 */
#include <stdlib.h>

/*
 * Character utility routines.
 */
#include <ctype.h>

/*
 * String utility routines.
 */
#include <string.h>

/*
 * This hash table stores the dictionary.
 */
HashTable *dictionary;

/*
 * The MAIN routine.  You can safely print debugging information
 * to standard error (stderr) as shown and it will be ignored in 
 * the grading process.
 */
int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Specify a dictionary\n");
    return 0;
  }
  /*
   * Allocate a hash table to store the dictionary.
   */
  fprintf(stderr, "Creating hashtable\n");
  dictionary = createHashTable(2255, &stringHash, &stringEquals);

  fprintf(stderr, "Loading dictionary %s\n", argv[1]);
  readDictionary(argv[1]);
  fprintf(stderr, "Dictionary loaded\n");

  fprintf(stderr, "Processing stdin\n");
  processInput();

  /*
   * The MAIN function in C should always return 0 as a way of telling
   * whatever program invoked this that everything went OK.
   */
  return 0;
}

/*
 * This should hash a string to a bucket index.  Void *s can be safely cast
 * to a char * (null terminated string) and is already done for you here 
 * for convenience.
 */
unsigned int stringHash(void *s) {
  char *string = (char *)s;
  unsigned int val = 0;
  
  // Add up all digits of a string using base-26
  // All in lowercase
  for (int i = 0; string[i]; i++)
  {
    char c = tolower(string[i]);
    val = val*26 + c - 'a';
  }

  return val;
}

/*
 * This should return a nonzero value if the two strings are identical 
 * (case sensitive comparison) and 0 otherwise.
 */
int stringEquals(void *s1, void *s2) {
  char *string1 = (char *)s1;
  char *string2 = (char *)s2;
  return !strcmp(string1, string2);
}

/*
 * This function should read in every buffer from the dictionary and
 * store it in the hash table.  You should first open the file specified,
 * then read the words one at a time and insert them into the dictionary.
 * Once the file is read in completely, return.  You will need to allocate
 * (using malloc()) space for each buffer.  As described in the spec, you
 * can initially assume that no buffer is longer than 60 characters.  However,
 * for the final 20% of your grade, you cannot assumed that words have a bounded
 * length.  You CANNOT assume that the specified file exists.  If the file does
 * NOT exist, you should print some message to standard error and call exit(1)
 * to cleanly exit the program.
 *
 * Since the format is one buffer at a time, with new lines in between,
 * you can safely use fscanf() to read in the strings until you want to handle
 * arbitrarily long dictionary characters.
 */
void readDictionary(char *dictName) {
  FILE * dict;
  dict = fopen(dictName, "r");

  if (dict == NULL)
  {
    fprintf(stderr, "Reading Failure! File does not exist!\n");
    exit(1);
  }

  // Try to read in words
  const size_t INITIAL_CAP = 64;
  size_t CURRENT_CAP = INITIAL_CAP;

  char *buffer = malloc(INITIAL_CAP);
  char *word = malloc(INITIAL_CAP);
  if (buffer == NULL)
  {
    fprintf(stderr, "Malloc Failure! Not enough memory!\n");
    exit(1);
  }

  // Using fgets to safely read in the words
  size_t current_len = 0;
  while (fgets(buffer, CURRENT_CAP, dict) != NULL)
  {
    size_t buffer_len = strlen(buffer);
    
    // Expand the size if not big enough
    if (current_len + buffer_len + 1 > CURRENT_CAP)
    {
      // Use realloc if not enough room
      size_t NEW_CAP = CURRENT_CAP*2 + 1;
      char *new_word = realloc(word, NEW_CAP);

      // Note for memory allocation failure
      if (new_word == NULL)
      {
        fprintf(stderr, "Realloc Failure! Not enough memory!\n");
        exit(1);
      }

      word = new_word;
      CURRENT_CAP = NEW_CAP;
    }

    strcpy(word+current_len, buffer);
    current_len += buffer_len;
  }

  // Now word stores all words
  // Split them using strtok

  char * pch;
  pch = strtok (word,"\n");
  if (pch == NULL)
  {
    fprintf(stderr, "No words!\n");
    exit(1);
  }
  while (pch != NULL)
  {
    //printf ("%s\n",pch);
    // Insert the word into hash table
    const size_t string_length = strlen(pch);
    char * word_repo = malloc(string_length);
    strcpy(word_repo, pch);
    insertData(dictionary, (void *)word_repo, (void *)word_repo);
    
    pch = strtok (NULL, "\n");
  }


  free(buffer);

  fclose(dict);
}

/*
 * This should process standard input (stdin) and copy it to standard
 * output (stdout) as specified in the spec (e.g., if a standard 
 * dictionary was used and the string "this is a taest of  this-proGram" 
 * was given to stdin, the output to stdout should be 
 * "this is a teast [sic] of  this-proGram").  All words should be checked
 * against the dictionary as they are input, then with all but the first
 * letter converted to lowercase, and finally with all letters converted
 * to lowercase.  Only if all 3 cases are not in the dictionary should it
 * be reported as not found by appending " [sic]" after the error.
 *
 * Since we care about preserving whitespace and pass through all non alphabet
 * characters untouched, scanf() is probably insufficent (since it only considers
 * whitespace as breaking strings), meaning you will probably have
 * to get characters from stdin one at a time.
 *
 * Do note that even under the initial assumption that no buffer is longer than 60
 * characters, you may still encounter strings of non-alphabetic characters (e.g.,
 * numbers and punctuation) which are longer than 60 characters. Again, for the 
 * final 20% of your grade, you cannot assume words have a bounded length.
 */
void processInput() {
  char current_word[64] = "";
  int idx = 0;
  char c;
  char* sic = " [sic]";

  while ((c = getchar()) != EOF)
  {
    if (isalpha(c))
    {
      putchar(c);
      current_word[idx++] = c;
    }
    else
    {
      // Output " [sic]" if not in the dictionary
      if (!isInDictionary(dictionary, current_word))
      {
        fprintf(stdout, "%s", sic);
      }

      putchar(c);

      // Proceed to the next word
      while ((c = getchar()) != EOF && !isalpha(c)) {
        putchar(c);
      }

      if (c != EOF)
      {
        // Reset the word buffer
        memset(current_word, '\0', sizeof(current_word));
        current_word[0] = c;
        idx = 1;
        putchar(c);
      }
    }
  }

  // Handle the last word with care
  if (!isInDictionary(dictionary, current_word))
    {
      fprintf(stdout, "%s", sic);
    }

}


int isInDictionary(HashTable *dictionary, char *word)
{
  int case1 = 0;
  int case2 = 0;
  int case3 = 0;
  if (findData(dictionary, word) != NULL)
  {
    case1 = 1;
  }

  char *tmp = malloc(64);
  strcpy(tmp, word);
  size_t LEN = strlen(word);
  for (int i = 1; i < LEN; i++)
  {
    tmp[i] = tolower(word[i]);
  }
  if (findData(dictionary, tmp) != NULL)
  {
    case2 = 1;
  }
  tmp[0] = tolower(tmp[0]);
  if (findData(dictionary, tmp) != NULL)
  {
    case3 = 1;
  }

  free(tmp);
  return case1+case2+case3;
}