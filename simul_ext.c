#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h> // Used to see files in a directory
#include <ctype.h>
#include <stdbool.h>
#include "headers.h"

#define COMLEN 100 // Command length
#define SIZE_BLOQUE 512
#define MAX_INODOS 24
#define MAX_FICHEROS 20
#define MAX_BLOQUES_DATOS 96
#define PRIM_BLOQUE_DATOS 4
#define MAX_BLOQUES_PARTICION MAX_BLOQUES_DATOS+PRIM_BLOQUE_DATOS // 96 + 4 = 100
#define MAX_NUMS_BLOQUE_INODO 7
#define LEN_NFICH 17
#define NULL_INODO 0xFFFF
#define NULL_BLOQUE 0xFFFF

void printByteMaps(EXT_BYTE_MAPS *ext_bytemaps) {
    // Prints inodes
    printf("Inodes: ");
    for(int i = 0; i < MAX_INODOS; i++) {
        printf("%d ", ext_bytemaps->bmap_inodos[i]);
    } // end for loop

    // Prints first 25 blocks
    printf("\nBlocks [0 - 25]: ");
    for(int i = 0; i < 25; i++) {
        printf("%d ", ext_bytemaps->bmap_bloques[i]);
    } // end for loop
    printf("\n");
} // end of printByteMaps

int checkCommand(char *strcommand, char *order, char *argument1, char *argument2) {
    int count = sscanf(strcommand, "%s %s %s", order, argument1, argument2);

    if(count==1){
        if(strcmp(order, "info")==0 || strcmp(order, "bytemaps")==0 || strcmp(order, "dir")==0 || strcmp(order, "exit")==0) {
            return 0;
        } // end if condition
    } else if (count==2){
        if(strcmp(order, "rename")==0 || strcmp(order, "imprimir")==0 || strcmp(order, "remove")==0) {
            return 0;
        }
    } else if (count==3){
        if(strcmp(order, "copy")==0) {
            return 0;
        } // end if condition
    } // end if, else if, else if conditions

    printf("Error in the command. Try again!\n");
    return -1;
} // end of checkCommand

void readSuperBlock(EXT_SIMPLE_SUPERBLOCK *psup) {} // end of readSuperBlock

int searchFile(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes, char *name) {} // end of searchFile

void directory(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes) {
    DIR *dr = opendir("."); // Opens current directory
    struct dirent *en;

    if(dr) { // If the directory exists
        while((en = readdir(dr)) != NULL) { // goes over files in the directory
            printf("%c\t", directory->dir_nfich);
            printf("Size: %d\t", inodes->blq_inodos->size_fichero);
            printf("Inode: %d\t", directory->dir_inodo);
            printf("Blocks: ");
            for(int i = 0; i < sizeof(inodes->blq_relleno); i++) { // Not sure about this
                printf("%d ", inodes->blq_relleno[i]); // Not sure about this
            } // end for loop
        } // end while loop
    } // end if condition
    
    closedir(dr);
} // end of directory

int renameFile(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes, char *nombreantiguo, char *nombrenuevo) {
    DIR *dr = opendir(".");
    struct dirent *en;
    bool srcFileEx = false;
    bool newNameFileEx = false;

    if(dr) {
        while((en = readdir(dr)) != NULL) {
            if(directory->dir_nfich == nombreantiguo) { // Check if source file exists
                srcFileEx = true;
            } else if(directory->dir_nfich == nombrenuevo) { // Checks if file with the new name exists
                newNameFileEx = true;
            } // end if, else if conditions
        } // end while loop
    } // end if condition
    
    if(srcFileEx == false) { // If source file does not exist
        printf("ERROR: File %c is not found\n", nombreantiguo);
    } else if(newNameFileEx) { // If file with the new name exists
        printf("ERROR: File %c already exists\n", nombrenuevo);
    } else { // If everything is correct --> no error
        int ren = rename(nombreantiguo, nombrenuevo); // Library already included in C

        if(ren = -1) {
            printf("ERROR: Unable to rename the file\n");
        } // end if condition
    } // end if, else if, else conditions
} // end of renameFile

int print(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes, EXT_DATOS *memdata, char *nombre) {} // end of print

int delete(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           char *name,  FILE *f) {} // end of delete

int copy(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdata, char *originName, char *destName,  FILE *f) {} // end of copy

void recordInodeDirectory(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes, FILE *f) {} // end of recordInodeDirectory

void recordByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *f) {} // end of recordByteMaps

void recordSuperBlock(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *f) {} // end of recordSuperBlock

void recordData(EXT_DATOS *memdata, FILE *f) {
    
} // end of recordData

int main() {
    char *command[COMLEN];
    char *order[COMLEN];
	char *argument1[COMLEN];
	char *argument2[COMLEN];

    unsigned long int m;
    EXT_SIMPLE_SUPERBLOCK ext_superblock;
    EXT_BYTE_MAPS ext_bytemaps;
    EXT_BLQ_INODOS ext_blq_inodes;
    EXT_ENTRADA_DIR directory[MAX_FICHEROS];
    EXT_DATOS memData[MAX_BLOQUES_DATOS];
    EXT_DATOS fileData[MAX_BLOQUES_PARTICION];
    int dirEntrance;
    int recordData;
    FILE *entranceFile = fopen("partition.bin", "r+b");

    fread(&fileData, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, entranceFile); 

    memcpy(&ext_superblock,(EXT_SIMPLE_SUPERBLOCK *)&fileData[0], SIZE_BLOQUE);
    memcpy(&directory,(EXT_ENTRADA_DIR *)&fileData[3], SIZE_BLOQUE);
    memcpy(&ext_bytemaps,(EXT_BLQ_INODOS *)&fileData[1], SIZE_BLOQUE);
    memcpy(&ext_blq_inodes,(EXT_BLQ_INODOS *)&fileData[2], SIZE_BLOQUE);
    memcpy(&memData,(EXT_DATOS *)&fileData[4],MAX_BLOQUES_DATOS*SIZE_BLOQUE);

    for(;;) {
        do {
            printf(">> ");
            fflush(stdin); // Cleans buffer for stdin
            fgets(command, COMLEN, stdin);
        } while(checkCommand(command, order, argument1, argument2) != 0);

        if (strcmp(order, "dir") == 0) {
            directory(&directory, &ext_blq_inodes);
            continue;
        } // end if condition

        recordInodeDirectory(&directory, &ext_blq_inodes, entranceFile);
        recordByteMaps(&ext_bytemaps, entranceFile);
        recordSuperBlock(&ext_superblock, entranceFile);

        if (recordData) {
            recordData(&memData, entranceFile);
        } // end if condition

        recordData = 0;

        if(strcmp(order, "exit") == 0) {
            recordData(&memData, entranceFile);
            fclose(entranceFile);

            return 0;
        } // end if condition
    } // end for loop 

    return 0;
} // end of main