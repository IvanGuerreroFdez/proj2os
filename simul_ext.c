#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdbool.h>
#include "headers.h"

// Constants for better readability
#define COMLEN 100           // Command length
#define SIZE_BLOQUE 512       // Block size
#define MAX_INODOS 24         // Maximum number of inodes
#define MAX_FICHEROS 20       // Maximum number of files in directory
#define MAX_BLOQUES_DATOS 96  // Maximum number of data blocks
#define PRIM_BLOQUE_DATOS 4   // First data block number
#define MAX_BLOQUES_PARTICION MAX_BLOQUES_DATOS + PRIM_BLOQUE_DATOS  // Total number of blocks in the partition
#define MAX_NUMS_BLOQUE_INODO 7  // Maximum number of block indices in an inode
#define LEN_NFICH 17            // Maximum length of a file name
#define NULL_INODO 0xFFFF       // Null inode value
#define NULL_BLOQUE 0xFFFF      // Null block value

bool permissions = false;

// Function to grant permisions
void GrantPermissions() {
    if (permissions == false) {
        permissions = true;
        printf("Permissions granted!\n");
    } else {
        printf("Permissions already granted!\n");
    }
}

// Function to print byte maps
void printByteMaps(EXT_BYTE_MAPS *ext_bytemaps) {
    printf("Inodes: ");
    for (int i = 0; i < MAX_INODOS; i++) {
        printf("%d ", ext_bytemaps->bmap_inodos[i]);
    } // end for loop

    printf("\nBlocks [0 - 25]: ");
    for (int i = 0; i < 25; i++) {
        printf("%d ", ext_bytemaps->bmap_bloques[i]);
    } // end for loop

    printf("\n");
} // end of printByteMaps

// Function to check the validity of the command
int checkCommand(char *strcommand, char *order, char *argument1, char *argument2) {
    int count = sscanf(strcommand, "%s %s %s", order, argument1, argument2);

    if (count == 1) {
        if (strcmp(order, "info") == 0 || strcmp(order, "bytemaps") == 0 || strcmp(order, "dir") == 0 || strcmp(order, "exit") == 0 || strcmp(order, "chmod") == 0) {
            return 0;
        } // end if condition
    } else if (count == 2) {
        if (strcmp(order, "print") == 0 || strcmp(order, "remove") == 0) {
            return 0;
        } // end if condition
    } else if (count == 3) {
        if (strcmp(order, "copy") == 0 || strcmp(order, "rename") == 0) {
            return 0;
        } // end if condition
    } // end if, else if x2 conditions

    printf("Invalid Command. Try again please.\n");
    return -1;
} // end of checkCommand

// Function to print information from the superblock
void readSuperBlock(EXT_SIMPLE_SUPERBLOCK *psup) {
    printf("Inodes count = %u\n", psup->s_inodes_count);
    printf("Blocks count = %u\n", psup->s_blocks_count);
    printf("Free Inodes count = %u\n", psup->s_free_inodes_count);
    printf("Free Blocks count = %u\n", psup->s_free_blocks_count);
    printf("First data block = %u\n", psup->s_first_data_block);
    printf("Block Size: %u bytes\n", psup->s_block_size);
} // end of readSuperBlock

// Function to search for a file in the directory
int searchFile(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes, char *name) {
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directory[i].dir_nfich, name) == 0) {
            return i;
        } // end if condition
    } // end for loop
    return -1;
} // end of searchFile

// Function to print the directory contents
void dir(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes) {
    for (int i = 1; i < MAX_FICHEROS; i++) {
        if (directory[i].dir_inodo != NULL_INODO) {
            printf("%s\t", directory[i].dir_nfich);
            if (strstr(directory[i].dir_nfich, ".txt") != NULL) {
                printf("Size: %d\t", inodes->blq_inodos[directory[i].dir_inodo].size_fichero);
                printf("Inode: %d\t", directory[i].dir_inodo);
                printf("Blocks: ");
                for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
                    if (inodes->blq_inodos[directory[i].dir_inodo].i_nbloque[j] != NULL_BLOQUE) {
                        printf("%d ", inodes->blq_inodos[directory[i].dir_inodo].i_nbloque[j]);
                    }
                } // end for loop
                if (permissions == true) {
                    printf("\tEnough permisions");
                } else {
                    printf("\tNot enough permisions");
                }
            }
            printf("\n");
        } // end if condition
    } // end for loop
} // end of dir

// Function to rename a file in the directory
int renameFile(EXT_ENTRADA_DIR *directory, char *nombreantiguo, char *nombrenuevo) {
    int fileIndex = searchFile(directory, NULL, nombreantiguo);

    if (fileIndex != -1) {
        if (searchFile(directory, NULL, nombrenuevo) == -1) {
            strcpy(directory[fileIndex].dir_nfich, nombrenuevo);
            printf("File %s succesfully renamed!.\n", nombreantiguo);
        } else {
            printf("ERROR: the file already exists (remember it is case-sensitive), try again.\n");
        } // end if else conditions
    } else {
        printf("ERROR: file %s is not found. Use 'dir' to check the files ^^\n", nombreantiguo);
    } // end if else conditions

    return 0;
} // end of renameFile

// Function to print the contents of a file
int print(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes, EXT_DATOS *memData, char *name) {
    int fileIndex = searchFile(directory, inodes, name);
    int counter = 0;

    if (permissions == true) {
        if (fileIndex != -1) {
            int inodeIndex = directory[fileIndex].dir_inodo;
            for(int i = 0; i < MAX_NUMS_BLOQUE_INODO && inodes->blq_inodos[inodeIndex].i_nbloque[i] != NULL_BLOQUE; i++) {
                int blockIndex = ((inodes->blq_inodos[inodeIndex].i_nbloque[i]) -4);

                for (int j = 0; j < SIZE_BLOQUE && counter < inodes->blq_inodos[inodeIndex].size_fichero != '\0'; j++) {
                    counter++;
                    printf("%c", memData[blockIndex].dato[j]); 
                } // end for loop
            } // end for loop

            printf("\n");
        } else {
            printf("ERROR: file %s is not found. Use 'dir' to check the files ^^\n", name);
            return -1;
        } // end if else condition
    } else {
        printf("Not enough permissions. Please type de comand 'chmod' to get root permissions!\n");
    }
    return 0;
} // end of print

// Function to delete a file from the directory and release associated blocks
int delete(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes, 
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, 
           char *name, FILE *fich) {
    int fileIndex = searchFile(directory, inodes, name);

    if (fileIndex != -1) {
        int inodeIndex = directory[fileIndex].dir_inodo;
        for (int i = 0; i < MAX_NUMS_BLOQUE_INODO && inodes->blq_inodos[inodeIndex].i_nbloque[i] != NULL_BLOQUE; i++) {
            ext_bytemaps->bmap_bloques[inodes->blq_inodos[inodeIndex].i_nbloque[i]] = 0;
        }

        ext_bytemaps->bmap_inodos[inodeIndex] = 0;
        memset(&directory[fileIndex], 0, sizeof(EXT_ENTRADA_DIR));
        printf("The file %s has been deleted successfully :)\n", name);
    } else {
        printf("ERROR: file %s is not found. Use 'dir' to check the files ^^\n", name);
    }

    return 0;
} // end of delete

// Function to copy a file in the directory to another location
int copy(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes, EXT_BYTE_MAPS *ext_bytemaps, 
         EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memData, 
         char *originName, char *destName, FILE *entranceFile) {
    int sourceIndex = searchFile(directory, inodes, originName);
    int destIndex = searchFile(directory, NULL, destName);
    int counter = 0;

    if (sourceIndex == -1) {
        printf("ERROR: Source file %s not found. Use 'dir' to check the files ^^\n", originName);
        return -1;
    }
    if (destIndex != -1) {
        printf("ERROR: Destination file %s already exists. Try a different name.\n", destName);
        return -1;
    }

    int freeInodeIndex;
    for (freeInodeIndex = 0; freeInodeIndex < MAX_INODOS; freeInodeIndex++) {
        if (ext_bytemaps->bmap_inodos[freeInodeIndex] == 0) {
            ext_bytemaps->bmap_inodos[freeInodeIndex] = 1; 
            break;
        }
    }
    if (freeInodeIndex == MAX_INODOS) {
        printf("ERROR: No free inode available. Cannot copy the file.\n");
        return -1;
    }

    printf("traza 1\n");
    int destInodeIndex = freeInodeIndex;
    /* strcpy(directory[freeInodeIndex].dir_nfich, destName);
    directory[freeInodeIndex].dir_inodo = destInodeIndex;
    inodes->blq_inodos[destInodeIndex].size_fichero = inodes->blq_inodos[directory[sourceIndex].dir_inodo].size_fichero; */

    printf("traza 2\n");
    int inodeIndex = directory[sourceIndex].dir_inodo;
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        printf("traza 3\n");
        int destBlockIndex = ((inodes->blq_inodos[inodeIndex].i_nbloque[i]) -4);
        printf("traza 4\n");
        for (int j = 0; j < SIZE_BLOQUE && counter < inodes->blq_inodos[inodeIndex].size_fichero != '\0'; j++) {
            printf("traza 5\n");
            counter++;
            printf("traza 5.5\n");
            memData[destBlockIndex].dato[j] = memData[inodes->blq_inodos[inodeIndex].i_nbloque[i] - 4].dato[j]; //Esto da un segmentation fault y termina la ejecucion
            printf("traza 6\n");
        } // end for loop
        if (destBlockIndex == NULL_BLOQUE) {
            break; 
        }
        printf("traza 7\n");
        int freeBlockIndex;
        printf("traza 8\n");
        for (freeBlockIndex = 0; freeBlockIndex < MAX_BLOQUES_DATOS; freeBlockIndex++) {
            printf("traza 9\n");
            if (ext_bytemaps->bmap_bloques[freeBlockIndex] == 0) {
                printf("traza 10\n");
                ext_bytemaps->bmap_bloques[freeBlockIndex] = 1;  
                break;
            }
        }
        printf("traza 11\n");
        if (freeBlockIndex == MAX_BLOQUES_DATOS) {
            printf("ERROR: No free block available. Cannot copy the file.\n");
            ext_bytemaps->bmap_inodos[destInodeIndex] = 0;
            memset(&directory[freeInodeIndex], 0, sizeof(EXT_ENTRADA_DIR));

            return -1;
        }
        inodes->blq_inodos[destInodeIndex].i_nbloque[i] = freeBlockIndex;
        printf("traza 12\n");
        memcpy(memData[freeBlockIndex].dato, memData[destBlockIndex].dato, SIZE_BLOQUE);
        printf("traza 13\n"); 
    }
    printf("File %s successfully copied to %s!\n", originName, destName);

    return 0;
} // end of copy

// Function to record directory and inode data to the file
void recordInodeAndDirectory(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes, FILE *f) {
    fseek(f, SIZE_BLOQUE * 3, SEEK_SET);
    fwrite(directory, SIZE_BLOQUE, 1, f);
    fwrite(inodes, SIZE_BLOQUE, 1, f);
} // end of recordInodeAndDirectory

// Function to record bytemaps data to the file
void recordByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *f) {
    fseek(f, SIZE_BLOQUE, SEEK_SET);
    fwrite(ext_bytemaps, SIZE_BLOQUE, 1, f);
} // end of recordByteMaps

// Function to record superblock data to the file
void recordSuperBlock(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *f) {
    fseek(f, 0, SEEK_SET);
    fwrite(ext_superblock, SIZE_BLOQUE, 1, f);
} // end of recordSuperBlock

// Function to record data blocks to the file
void recordDataBlocks(EXT_DATOS *memdata, FILE *f) {
    fseek(f, SIZE_BLOQUE * 4, SEEK_SET);
    fwrite(memdata, SIZE_BLOQUE, MAX_BLOQUES_DATOS, f);
} // end of recordDataBlocks

// Main function
int main() {
    // Memory allocation for command and arguments
    char command[COMLEN];
	char order[COMLEN];
	char argument1[COMLEN];
	char argument2[COMLEN];

    // Declaration of data structures
    EXT_SIMPLE_SUPERBLOCK ext_superblock;
    EXT_BYTE_MAPS ext_bytemaps;
    EXT_BLQ_INODOS ext_blq_inodes;
    EXT_ENTRADA_DIR directory[MAX_FICHEROS];
    EXT_DATOS memData[MAX_BLOQUES_DATOS];
    EXT_DATOS fileData[MAX_BLOQUES_PARTICION];
    int recordData = 0;

    // File handling
    FILE *entranceFile = fopen("particion.bin", "r+b");

    // Checking File existance
    if (entranceFile == NULL) {
        printf("Error opening the file\n");
        return 1;
    } // end if condition
    fread(&fileData, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, entranceFile);

    // Data initialization from the file
    memcpy(&ext_superblock, (EXT_SIMPLE_SUPERBLOCK *)&fileData[0], SIZE_BLOQUE);
    memcpy(&directory, (EXT_ENTRADA_DIR *)&fileData[3], sizeof(EXT_ENTRADA_DIR) * MAX_FICHEROS);
    memcpy(&ext_bytemaps, (EXT_BYTE_MAPS *)&fileData[1], SIZE_BLOQUE);
    memcpy(&ext_blq_inodes, (EXT_BLQ_INODOS *)&fileData[2], SIZE_BLOQUE);
    memcpy(&memData, (EXT_DATOS *)&fileData[4], MAX_BLOQUES_DATOS * SIZE_BLOQUE);

    // Main command processing loop
    for (;;) {
        do {
            printf(">> ");
            fflush(stdin);
            fgets(command, COMLEN, stdin);
            command[strcspn(command, "\n")] = '\0';
        } while (checkCommand(command, order, argument1, argument2) != 0);

        // Process each command
        if (strcmp(order, "dir") == 0) {
            dir(directory, &ext_blq_inodes);
            continue;
        } else if (strcmp(order, "info") == 0) {
            readSuperBlock(&ext_superblock);
            continue;
        } else if (strcmp(order, "bytemaps") == 0) {
            printByteMaps(&ext_bytemaps);
            continue;
        } else if (strcmp(order, "rename") == 0) {
            renameFile(directory, argument1, argument2);
            continue;
        } else if (strcmp(order, "print") == 0) {
            print(directory, &ext_blq_inodes, memData, argument1);
            continue;
        } else if (strcmp(order, "remove") == 0) {
            delete(directory, &ext_blq_inodes, &ext_bytemaps, &ext_superblock, argument1, NULL);
            continue;
        } else if (strcmp(order, "copy") == 0) {
            copy(directory, &ext_blq_inodes, &ext_bytemaps, &ext_superblock, memData, argument1, argument2, entranceFile);
            continue;
        } else if (strcmp(order, "chmod") == 0) {
            GrantPermissions();
            continue;
        } else if (strcmp(order, "exit") == 0) {
            printf("You are exiting. Thanks for the visit, see you soon <3\n");
            break;
        } else {
            printf("Command not found. Try again, please.\n");
            continue;
        } // end if, else if x7, else conditions
    } // end for loop    

    // Write modified data structures back to the file
    recordSuperBlock(&ext_superblock, entranceFile);
    recordByteMaps(&ext_bytemaps, entranceFile);
    recordInodeAndDirectory(directory, &ext_blq_inodes, entranceFile);
    recordDataBlocks(memData, entranceFile);

    // Close the file and free allocated memory
    fclose(entranceFile);

    return 0;
} // end of main