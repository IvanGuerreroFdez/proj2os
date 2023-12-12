#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers.h"

#define COMLEN 100 // Command lengthss

void printByteMaps(EXT_BYTE_MAPS *ext_bytemaps) {} // end of printByteMaps

int checkCommand(char *strcommand, char *order, char *argument1, char *argument2) {} // end of checkCommand

void readSuperBlock(EXT_SIMPLE_SUPERBLOCK *psup) {} // end of readSuperBlock

int searchFile(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes, char *name) {} // end of searchFile

void directory(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes) {} // end of directory

int renameFile(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes, char *nombreantiguo, char *nombrenuevo) {} // end of renameFile

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

void recordData(EXT_DATOS *memdata, FILE *f) {} // end of recordData

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
    FILE *entranceFile;

    entranceFile = fopen("partition.bin", "r+b");
    fread(&fileData, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, entranceFile);    

    return 0;
} // end of main