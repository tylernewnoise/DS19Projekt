#include <stdio.h>      // fprintf, fclose, fopen, getc
#include <stdbool.h>    // bool
#include <stdint.h>     // uint64_t, uint8_t
#include <stdlib.h>     // calloc, malloc, realloc, free
#include <string.h>     // memcpy, strcmp, strlen

/**
 * Basic Idea:
 * 1. Read every word-translation-pair from a wb.file into a linked list.
 * 2. Traverse the linked list from head on and add every node to the hash table. While doing so, check for duplicates.
 * 3. The nodes now represent and item in the hashtable, so we don't have to "free" the linked list and allocate new
 * memory for the hash table. We simply use the nodes of the list for the hashtable.
 * 4. Read from stdin and check every word.
 */

/* Data structure for the linked list and the dictionary. The node contains strings and a pointer to the next node. */
struct Node {
    unsigned char *word;
    unsigned char *translation;
    struct Node *next_entry;
};

/* Data structure for the hash table with the size and an array of pointers of type Node. */
struct HT_dictionary {
    uint64_t dict_size;
    struct Node **dict_items;
};

/* The head of the linked list and the dictionary. */
struct Node *first_entry = NULL;
struct HT_dictionary *dictionary = NULL;

/* Function prototypes for the linked list. */
void delete_linked_list(void);
void delete_node(void);
bool prepend_to_linked_list(const unsigned char *, const unsigned char *);
uint64_t read_wb_line(const unsigned char *);

/* Function prototypes for the dictionary hash table. */
struct HT_dictionary *create_new_ht_dictionary(uint64_t);
void delete_ht_dictionary(void);
void delete_ht_dictionary_item(struct Node *);
uint64_t djb2_hash(const unsigned char *, uint64_t);
uint64_t find_next_prime(uint64_t);
bool is_prime(uint64_t);
void insert_linked_list_node_to_ht_dictionary(void);
void insert_to_ht_dictionary(struct Node *);
unsigned char *search_in_ht_dictionary(const unsigned char *);

/* Function prototypes for read from stdin. */
bool is_uppercase(int);
bool is_letter(int);
int read_from_stdin(void);

/* Program main entry point. */
int main(int argc, char *argv[]) {
    // Check program arguments.
    if (argc != 2) {
        fprintf(stderr, "Usage: cat stdin | %s filename\n", argv[0]);
        return 2;
    }
    int ret = 0;
    // Read wb.file to linked list.
    uint64_t dict_size = read_wb_line((const unsigned char *) argv[1]);

    // Create hashtable of desired size. The size is 1.5 times bigger up to the next prime.
    // For example: if the file contains 1000 lines, then the size is find_next_prime(1000 + 500) = 1511.
    dictionary = create_new_ht_dictionary(find_next_prime(dict_size + dict_size));

    // Write all entries of linked list to hashtable.
    insert_linked_list_node_to_ht_dictionary();

    // Read from standard input.
    ret = read_from_stdin();

    // Delete the dictionary and free all allocated memory.
    delete_ht_dictionary();
    return ret;
}

/* Functions for linked list. */
/* Delete the entire linked list. This function is necessary to prevent memory leaks in case of failures. */
void delete_linked_list(void) {
    while (first_entry != NULL)
        delete_node();
}

/* Delete a single node from the head of the linked list. */
void delete_node(void) {
    // Create a temporary pointer of type Node which points at the head of the list.
    struct Node *tmp = first_entry;
    // The head become now the second item in the list.
    first_entry = first_entry->next_entry;

    // Set pointer for the next entry of the old head to NULL and free() everything.
    tmp->next_entry = NULL;

    if (tmp == first_entry)
        first_entry = NULL;

    free(tmp->translation);
    free(tmp->word);
    free(tmp);
}

/* Insert a new node to the linked list at the beginning of the list. */
bool prepend_to_linked_list(const unsigned char *word, const unsigned char *translation) {
    // Allocate memory for the node and the strings to insert the node to the linked list.
    struct Node *new_node = malloc(sizeof(struct Node));
    unsigned char *tmp_word = malloc(strlen((char *) word) + 1);
    unsigned char *tmp_translation = malloc(strlen((char *) translation) + 1);

    // If malloc() fucks up, return. We don't exit here, because we want to free() our allocated line and close() the
    // open file pointer.
    if (new_node == NULL || tmp_word == NULL || tmp_translation == NULL)
        return false;

    // Add the data to the new node.
    memcpy(tmp_word, word, strlen((char *) word) + 1);
    memcpy(tmp_translation, translation, strlen((char *) translation) + 1);
    new_node->word = tmp_word;
    new_node->translation = tmp_translation;
    new_node->next_entry = first_entry;
    first_entry = new_node;
    return true;
}

/* Read the wb.file and insert its content to the linked list. */
uint64_t read_wb_line(const unsigned char *argv) {
    FILE *file_pointer;

    // Open file in read-only-mode.
    if ((file_pointer = fopen((const char *) argv, "r")) == NULL) {
        fprintf(stderr, "Error opening file %s!\n", argv);
        exit(2);
    }

    // Allocate some memory for an array to put the chars in.
    size_t size_word = 1024;
    size_t size_translation = 1024;
    unsigned char *str_word = malloc(size_word);
    unsigned char *str_translation = malloc(size_translation);

    // Break if memory allocation fails.
    if (str_word == NULL || str_translation == NULL) {
        fprintf(stderr, "Error: could not start to read wb-file - out of memory!\n");
        fclose(file_pointer);
        exit(2);
    }
    str_word[0] = '\0';
    str_translation[0] = '\0';

    int c = getc(file_pointer);     // Char to read.
    uint64_t index_word = 0;        // Index for the word we read.
    uint64_t index_translation = 0; // Index for the translation we read.
    uint64_t wb_lines = 0;          // Count of lines we read -> how many entries will our dictionary have.
    bool is_colon = false;          // To remember if there already was a colon in the line.

    while (c != EOF) {
        // Check for valid chars.
        if ((c < 'a' || c > 'z') && c != ':' && c != '\n') {
            fprintf(stderr, "Error: wrong dictionary format in line %lu!\n", wb_lines + 1);
            free(str_translation);
            free(str_word);
            delete_linked_list();
            fclose(file_pointer);
            exit(2);
        }

        // Reallocate if the char array for the word gets too big.
        if (!is_colon && index_word + 1 == size_word) {
            unsigned char *tmp = realloc(str_word, size_word *= 2);
            if (!tmp) {
                fprintf(stderr, "Error: could not read wb-file - out of memory!\n");
                free(str_word);
                free(str_translation);
                delete_linked_list();
                fclose(file_pointer);
                exit(2);
            } else
                str_word = tmp;
        }
        // Reallocate if the char array for the translation gets too big.
        if (is_colon && index_translation + 1 == size_translation) {
            unsigned char *tmp = realloc(str_translation, size_translation *= 2);
            if (!tmp) {
                fprintf(stderr, "Error: could not read wb-file - out of memory!\n");
                free(str_word);
                free(str_translation);
                delete_linked_list();
                fclose(file_pointer);
                exit(2);
            } else
                str_translation = tmp;
        }

        // Check if the line index is at zero and if there is a colon.
        if (str_word[0] == ':' || (c == ':' && is_colon == true)) {
            fprintf(stderr, "Error: wrong dictionary format in line %lu!\n", wb_lines + 1);
            free(str_word);
            free(str_translation);
            delete_linked_list();
            fclose(file_pointer);
            exit(2);
        }

        // If a newline appears and word-translation-pair was read, push it in the list.
        if (c == '\n' && index_translation > 0) {
            // Append terminating NULL-character.
            str_word[index_word] = '\0';
            str_translation[index_translation] = '\0';

            if (!prepend_to_linked_list(str_word, str_translation)) {
                fprintf(stderr, "Error: could not create dictionary - out of memory!\n");
                free(str_word);
                free(str_translation);
                delete_linked_list();
                fclose(file_pointer);
                exit(2);
            }

            // Set everything "back" to zero.
            str_word[0] = '\0';
            str_translation[0] = '\0';
            is_colon = false;
            index_word = index_translation = 0;
            wb_lines++;
        } else if (c == ':') {
            // Notice first colon.
            is_colon = true;
        } else if (!is_colon)
            // Write to word string or...
            str_word[index_word++] = (char) c;
        else
            // ...to translation string.
            str_translation[index_translation++] = (char) c;

        c = getc(file_pointer);
    }

    // If the file has no line break at the end we have to check if we read a line.
    if (index_translation > 0) {
        str_word[index_word] = '\0';
        str_translation[index_translation] = '\0';
        if (!prepend_to_linked_list(str_word, str_translation)) {
            fprintf(stderr, "Error: could not create dictionary - out of memory!\n");
            free(str_word);
            free(str_translation);
            delete_linked_list();
            fclose(file_pointer);
            exit(2);
        }
        wb_lines++;
    }

    // To read nothing is a valid input, but check first if it really was EOF.
    if (feof(file_pointer) == 0) {
        fprintf(stderr, "Error - could not create dictionary - wrong input!\n");
        free(str_word);
        free(str_translation);
        delete_linked_list();
        fclose(file_pointer);
        exit(2);
    }

    free(str_word);
    free(str_translation);
    fclose(file_pointer);
    return wb_lines;
}

/* Functions for dictionary hash table */
/* Create new dictionary. */
struct HT_dictionary *create_new_ht_dictionary(uint64_t size) {
    // Allocate memory for the hash table.
    struct HT_dictionary *ht = malloc(sizeof(*dictionary));

    if (ht == NULL) {
        fprintf(stderr, "Error: could not create dictionary - out of memory!\n");
        delete_linked_list();
        exit(2);
    }

    // Set the desired size...
    ht->dict_size = size;
    // ... and allocate an array of size elements for all the nodes we want to put in.
    ht->dict_items = calloc(ht->dict_size, sizeof(struct Node *));

    if (ht->dict_items == NULL) {
        fprintf(stderr, "Error: could not create dictionary - out of memory!\n");
        delete_linked_list();
        delete_ht_dictionary();
        exit(2);
    }

    return ht;
}

/* Delete the entire hash table by running through it. */
void delete_ht_dictionary(void) {

    for (uint64_t i = 0; i < dictionary->dict_size; i++) {
        struct Node *item = dictionary->dict_items[i];

        if (item != NULL)
            delete_ht_dictionary_item(item);
    }

    free(dictionary->dict_items);
    free(dictionary);
}

/* Delete hash table item. */
void delete_ht_dictionary_item(struct Node *item) {
    free(item->word);
    free(item->translation);
    free(item);
}

/* DJB2 hash function for strings. */
uint64_t djb2_hash(const unsigned char *word, uint64_t collisions) {
    uint64_t hash = 5381;
    uint8_t c;

    while ((c = *word++))
        hash = ((hash << 5u) * hash) + c; // hash * 33 + c

    return (hash + collisions) % dictionary->dict_size;
}

/* Helpers to make the dictionary size prime. */
uint64_t find_next_prime(uint64_t n) {
    uint64_t next_prime = ++n;

    // If the next number is even skip it so we will only check uneven numbers.
    if (next_prime % 2 == 0)
        next_prime++;

    while (1) {
        if (is_prime(next_prime))
            return next_prime;
        next_prime = next_prime + 2;
    }
}

bool is_prime(uint64_t n) {
    uint64_t x = 2;
    while (x <= n / 2) {
        if (n % x == 0)
            return false;
        x++;
    }
    return true;
}

/* Insert node in the dictionary. After all word-translation-pairs are in the dictionary the linked list
 * is empty because all the nodes are now dictionary items and hence stored in the hash table.
 * Also perform a duplicate check for the dictionary. */
void insert_linked_list_node_to_ht_dictionary(void) {

    while (first_entry != NULL) {
        // Temporary pointer to the head of the linked list.
        struct Node *tmp = first_entry;

        // Check for duplicates.
        if (search_in_ht_dictionary(tmp->word) != NULL) {
            fprintf(stderr, "Wrong dictionary format, found duplicate: <%s>!\n", tmp->word);
            delete_linked_list();
            delete_ht_dictionary();
            exit(2);
        }

        // The node next to the head becomes the new head...
        first_entry = first_entry->next_entry;
        tmp->next_entry = NULL;
        // ...and the old head is inserted to the hash table.
        insert_to_ht_dictionary(tmp);
    }
}

/* Insert a new word-translation-pair in the dictionary. */
void insert_to_ht_dictionary(struct Node *item) {
    // Calculate the hash a.k.a. where to store the node (the bucket).
    uint64_t index = djb2_hash(item->word, 0);
    // Set a pointer to that bucket.
    struct Node *cur_item = dictionary->dict_items[index];

    uint64_t collisions = 0;
    // Try to put the item in. If some other item is already there, recalculate the hash to deal
    // with the collision.
    while (cur_item != NULL) {
        index = djb2_hash(item->word, collisions);
        cur_item = dictionary->dict_items[index];
        collisions++;
    }

    // Finally insert it at the empty index.
    dictionary->dict_items[index] = item;
}

/* Search for a word in the dictionary. This is almost the same as inserting:
 * Calculate the hash and check if the word matches. If not, try again until a match or an emtpy bucket. */
unsigned char *search_in_ht_dictionary(const unsigned char *word) {
    uint64_t collisions = 1;
    uint64_t index = djb2_hash(word, 0);
    struct Node *item = dictionary->dict_items[index];

    while (item != NULL) {
        if (strcmp((const char *) item->word, (const char *) word) == 0)
            return item->translation;

        index = djb2_hash(word, collisions);
        item = dictionary->dict_items[index];
        collisions++;
    }

    return NULL;
}

/* Functions for reading from standard input. A lot of this comes from Benni. */
int read_from_stdin(void) {

    // Allocate some memory for an array to put the chars in.
    size_t size_search_pattern = 1024;
    unsigned char *search_pattern = malloc(size_search_pattern);

    // Break if memory allocation fails.
    if (search_pattern == NULL) {
        fprintf(stderr, "Error: could not start to read form stdin - out of memory!\n");
        delete_ht_dictionary();
        exit(2);
    }

    int c;
    int ret = 0;
    size_t len = 0;
    bool word_started = false;
    bool last_word = false;
    bool word_starts_uppercase = false;

    // Read char by char until you get to the end of the file.
    c = getc(stdin);

    // To read nothing is a valid input, so we can exit here.
    if (c == EOF) {
        free(search_pattern);

        // But check if the file was really empty or was it some strange character?
        if (feof(stdin) == 0) {
            delete_ht_dictionary();
            fprintf(stderr, "Error: wrong input format due to non valid character!\n");
            exit(2);
        }
        return ret;
    }

    while (true) {
        // Did we read a legal character? If not, we can break and check if feof() == 0.
        if ((c != '\n') && ((c < ' ') || (c > '~'))) {
            // But print the last word of the file first.
            if (strlen((const char *) search_pattern) > 0)
                last_word = true;
            else
                break;
        }

        // Resize the array if the input becomes to big by factor two.
        if (len == size_search_pattern) {
            unsigned char *tmp = realloc(search_pattern, size_search_pattern *= 2);
            // Break if reallocation fails.
            if (!tmp) {
                fprintf(stderr, "Error: could not read form stdin - out of memory!\n");
                free(search_pattern);
                delete_ht_dictionary();
                exit(2);
            } else
                search_pattern = tmp;
        }

        if (is_letter(c)) {
            word_started = true;
            // Add character to search pattern. c is in ASCII format, so casting is fine here.
            search_pattern[len++] = (char) c;
        } else {
            // Character is not a letter, hence the reading of the search pattern has finished or
            // we read a valid char, but it is not a letter (delimiter).
            if (word_started) {
                // A word of only letters has been read.
                // Make it a zero terminated string then.
                search_pattern[len] = '\0';

                // Check if first char is capitalized.
                if (is_uppercase(search_pattern[0]))
                    word_starts_uppercase = true;

                // Make a copy of the search pattern string to turn it to small letters.
                unsigned char *lookup_copy = malloc(strlen((const char *) search_pattern) + 1);
                if (lookup_copy == NULL) {
                    fprintf(stderr, "Error: could not start to read form stdin - out of memory!\n");
                    free(search_pattern);
                    delete_ht_dictionary();
                    exit(2);
                }
                memcpy(lookup_copy, search_pattern, strlen((const char *) search_pattern) + 1);

                // Make chars lowercase.
                for (size_t i = 0; lookup_copy[i] != '\0'; i++)
                        lookup_copy[i] = lookup_copy[i] | 32u;

                // Create a pointer to our translation.
                unsigned char *tmp = search_in_ht_dictionary(lookup_copy);

                // Print the original search pattern if it is not found in the dictionary and set
                // the return value to 1.
                if (tmp == NULL) {
                    fprintf(stdout, "<%s>", search_pattern);
                    ret = 1;
                } else {
                    // If the search pattern is found and a translation returned we maybe need to
                    // capitalize the first letter for output.
                    if (word_starts_uppercase) {
                        // Make a copy of the translation returned from the dictionary.
                        // This is necessary because we have to change the first letter and don't want
                        // to change it in the dictionary, too.
                        unsigned char *translation = malloc(strlen((const char *) tmp) + 1);
                        if (translation == NULL) {
                            fprintf(stderr, "Error: could not start to read form stdin - out of memory!\n");
                            free(lookup_copy);
                            free(search_pattern);
                            delete_ht_dictionary();
                            exit(2);
                        }
                        memcpy(translation, tmp, strlen((const char *) tmp) + 1);

                        // Make the first letter uppercase.
                        translation[0] = translation[0] & ~32u;

                        // Print the capitalized translation.
                        fprintf(stdout, "%s", translation);
                        free(translation);
                    } else
                        // Print the lowercase translation.
                        fprintf(stdout, "%s", tmp);
                }

                // Clear memory for the search pattern copy.
                free(lookup_copy);
                // Reset the read-in array to zeros.
                search_pattern[0] = '\0';

                // Reset word variables.
                len = 0;
                word_started = false;
                word_starts_uppercase = false;

                // Print characters after current word without changing.
                if (!last_word)
                    fprintf(stdout, "%c", c);
            } else {
                if (c == EOF)
                    break;
                // Print characters between words without changing.
                fprintf(stdout, "%c", c);
            }
        }

        if (!last_word)
            c = getc(stdin);
        else
            break;
    }

    // End of file or error?
    if (feof(stdin) == 0) {
        fprintf(stderr, "Error: wrong input format due to non valid character!\n");
        free(search_pattern);
        delete_ht_dictionary();
        exit(2);
    }

    free(search_pattern);
    return ret;
}

/* Helper functions to check if a character is capitalized or a letter. */
bool is_uppercase(int input) {
    return (((input >= 'A') && (input <= 'Z')));
}

bool is_letter(int input) {
    return (((input >= 'A') && (input <= 'Z')) || ((input >= 'a' && input <= 'z')));
}
