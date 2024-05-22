#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct hashTable *hashTableP;

struct hashTable
{
    char Word[50];
    int Frequency;
    int status; // Empty -> 0, Occupied -> 1, Deleted -> -1
};

int currentTableSize = 0; //current size of the hash table

void hashInitialize(hashTableP, int);
int hashFunction(char*, int);
void hashInsert(hashTableP, char*, int);
void hashDisplay(hashTableP, int);
int hashSearch(hashTableP, char*, int);
void hashDelete(hashTableP, char*, int);

int main()
{

    //hash table creation & initialization
    int TableSize = 311;
    hashTableP myTable = (struct hashTable*)malloc(sizeof(struct hashTable) * TableSize);
    hashInitialize(myTable , TableSize);

    hashInsert(myTable, "Hi", TableSize);
    hashInsert(myTable, "World", TableSize);
    hashInsert(myTable, "My", TableSize);
    hashInsert(myTable, "Name", TableSize);
    hashInsert(myTable, "is", TableSize);
    hashInsert(myTable, "is", TableSize);
    hashInsert(myTable, "Mohammad", TableSize);

    int index = hashSearch(myTable, "is", TableSize);
    if( index == -1){
        printf("\nThe hash table is empty\n\n");
    } else if (index == -2){
        printf("\nThe word you entered not found!\n\n");
    }else{
        printf("\nIndex = %d | Word = %s | Frequency = %d \n\n", index, myTable[index].Word, myTable[index].Frequency);
    }

    //hashDisplay(myTable, TableSize);

    hashDelete(myTable, "Mohammad", TableSize);

    hashDisplay(myTable, TableSize);

    printf("\n Done!\n");

    return 0;
}

//function to initialize the double hash table
void hashInitialize(hashTableP hashTable, int TableSize){

    for(int i = 0; i < TableSize; i++){
        hashTable[i].status = 0; //set status Empty
    }
}

//function to hash the Word word
int hashFunction(char* Word, int TableSize){
    long long int hashValue = 0;

    while(*Word != '\0'){
        hashValue = (hashValue << 5) + *Word++;
    }

    return (hashValue % TableSize);
}

//function to insert a Word to the double hash table
void hashInsert(hashTableP hashTable, char* Word, int TableSize){

    int tempIndex = hashFunction(Word, TableSize);
    int index = tempIndex;
    int i = 1;

    if(currentTableSize == TableSize){
        printf("\nHash Table is full!\n\n");
        return;
    }

    //go through the table until we reach an empty node
    while(hashTable[index].status == 1){
        if(strcasecmp(hashTable[index].Word, Word) == 0){
            hashTable[index].Frequency++;
            return;
        }

        index = ((tempIndex + (i * (307 - (tempIndex % 307)))) % TableSize); //prop the hash index

        if(index == tempIndex){
            printf("\nHash Table is full!!\n\n");
            return;
        }

        i++;
    }

    //insert record into the table
    hashTable[index].status = 1;
    strcpy(hashTable[index].Word, Word);
    hashTable[index].Frequency = 1;
    currentTableSize++;

    printf("\nThe word you entered has been inserted\n");
    //printf("Current Size = %d\n", currentTableSize);
}

//function to display the double hash table
void hashDisplay(hashTableP hashTable, int TableSize) {
    for (int i = 0; i < TableSize; i++) {
        if (hashTable[i].status == 1) {
            printf("%d. Word = %s | Frequency = %d\n", i, hashTable[i].Word, hashTable[i].Frequency);
        } else if (hashTable[i].status == -1) {
            printf("%d. DELETED\n", i);
        } else {
            printf("%d. EMPTY\n", i);
        }
    }
}

//function to search a Word in the double hash table
int hashSearch(hashTableP hashTable, char* Word, int TableSize){

    int tempIndex = hashFunction(Word, TableSize);
    int index = tempIndex;
    int i = 1;

    if(currentTableSize == 0){
        return -1;
    }

    //go through the table until we reach an empty node
    while(hashTable[index].status != 0){
        if(strcasecmp(hashTable[index].Word, Word) == 0 && hashTable[index].status == 1){
            return index;
        }

        index = ((tempIndex + (i * (307 - (tempIndex % 307)))) % TableSize);

        if(index == TableSize){
            break;
        }

        i++;
    }

    return -2;
}

//function to delete a Word from the double hash table
void hashDelete(hashTableP hashTable, char* Word, int TableSize){


    int index = hashSearch(hashTable, Word, TableSize);
    if(index == -1){
        printf("\nThe hash table is empty\n\n");

    } else if (index == -2){
        printf("\nThe word you entered not found!\n\n");

    }else{

        hashTable[index].status = -1;
        currentTableSize--;
        printf("\nThe word you entered has been deleted!\n\n");
    }
}
