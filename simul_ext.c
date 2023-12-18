#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "headers.h"

#define COMLEN 100 // Command len
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
    printf("Inodes: ");
    for (int i = 0; i < MAX_INODOS; i++) {
        printf("%d ", ext_bytemaps->bmap_inodos[i]);
    }

    printf("\nBlocks [0 - 25]: ");
    for (int i = 0; i < 25; i++) {
        printf("%d ", ext_bytemaps->bmap_bloques[i]);
    }
    printf("\n");
}

int checkCommand(char *strcommand, char *order, char *argument1, char *argument2) {
    int count = sscanf(strcommand, "%s %s %s", order, argument1, argument2);

    if (count == 1) {
        if (strcmp(order, "info") == 0 || strcmp(order, "bytemaps") == 0 || strcmp(order, "dir") == 0 || strcmp(order, "exit") == 0) {
            return 0;
        }
    } else if (count == 2) {
        if (strcmp(order, "rename") == 0 || strcmp(order, "print") == 0 || strcmp(order, "remove") == 0) {
            return 0;
        }
    } else if (count == 3) {
        if (strcmp(order, "copy") == 0) {
            return 0;
        }
    }
    printf("Invalid Command. Try again please\n");
    return -1;
}

void readSuperBlock(EXT_SIMPLE_SUPERBLOCK *psup) {
    printf("Inodes count = %u\n", psup->s_inodes_count);
    printf("Blocks count = %u\n", psup->s_blocks_count);
    printf("Free Inodes count = %u\n", psup->s_free_blocks_count);
    printf("Free Blocks count = %u\n", psup->s_free_inodes_count);
    printf("First data block = %u\n", psup->s_first_data_block);
    printf("Block Size: %u bytes\n", psup->s_block_size);
}

int searchFile(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes, char *name) {
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directory[i].dir_nfich, name) == 0) {
            return i;
        }
    }
    return -1;
}

void dir(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes) {
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directory[i].dir_inodo != NULL_INODO) {
            printf("%s\t", directory[i].dir_nfich);
            printf("Size: %d\t", inodes->blq_inodos[directory[i].dir_inodo].size_fichero);
            printf("Inode: %d\t", directory[i].dir_inodo);
            printf("Blocks: ");
            for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
                printf("%d ", inodes->blq_inodos[directory[i].dir_inodo].i_nbloque[j]);
            }
            printf("\n");
        }
    }
}

int renameFile(EXT_ENTRADA_DIR *directory, char *nombreantiguo, char *nombrenuevo) {
    int fileIndex = searchFile(directory, NULL, nombreantiguo);
    if (fileIndex != -1) {
        if (searchFile(directory, NULL, nombrenuevo) == -1) {
            strcpy(directory[fileIndex].dir_nfich, nombrenuevo);
            printf("File %s succesfully renamed!.\n", nombreantiguo);
        } else {
            printf("ERROR: the file already exist (remember is case sensitive and try again).\n");
        }
    } else {
        printf("ERROR: file %s is not found. Use 'dir' to check the files ^^\n", nombreantiguo);
    }
    return 0;
}

int print(char *nombre) {
    FILE *f = fopen(nombre, "r");
    if (f == NULL) {
        printf("ERROR: file not found. Use 'dir' to check the files ^^\n");
        return -1;
    }
    char c;
    while ((c = getc(f)) != EOF) {
        printf("%c", c);
    }
    fclose(f);
    return 0;
}

int delete(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes, EXT_BYTE_MAPS *ext_bytemaps, char *name) {
    int fileIndex = searchFile(directory, inodes, name);

    if (fileIndex != -1) {
        int inodeIndex = directory[fileIndex].dir_inodo;
        for (int i = 0; i < MAX_NUMS_BLOQUE_INODO && inodes->blq_inodos[inodeIndex].i_nbloque[i] != NULL_BLOQUE; i++) {
            ext_bytemaps->bmap_bloques[inodes->blq_inodos[inodeIndex].i_nbloque[i]] = 0;
        }
        ext_bytemaps->bmap_inodos[inodeIndex] = 0;
        memset(&directory[fileIndex], 0, sizeof(EXT_ENTRADA_DIR));
        printf("The file %s has been deleted succesfully :)\n", name);
    } else {
        printf("ERROR: file %s is not found. Use 'dir' to check the files ^^\n", name);
    }
    return 0;
}

int copy(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes, EXT_BYTE_MAPS *ext_bytemaps,
         EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *originName, char *destName) {
    int srcIndex = searchFile(directory, inodes, originName);
    int destIndex = searchFile(directory, NULL, destName);
    if (srcIndex != -1) {
        if (destIndex == -1) {
            int srcInodeIndex = directory[srcIndex].dir_inodo;
            int destInodeIndex = -1;
            for (int i = 0; i < MAX_INODOS; i++) {
                if (ext_bytemaps->bmap_inodos[i] == 0) {
                    destInodeIndex = i;
                    break;
                }
            }

            if (destInodeIndex != -1) {
                int destBlockIndices[MAX_NUMS_BLOQUE_INODO];
                int numBlocks = 0;
                for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
                    if (ext_bytemaps->bmap_bloques[i] == 0) {
                        destBlockIndices[numBlocks++] = i;
                    }

                    if (numBlocks == inodes->blq_inodos[srcInodeIndex].size_fichero) {
                        break;
                    }
                }

                if (numBlocks == inodes->blq_inodos[srcInodeIndex].size_fichero) {
                    strcpy(directory[srcIndex].dir_nfich, destName);
                    directory[srcIndex].dir_inodo = destInodeIndex;
                    memcpy(&inodes->blq_inodos[destInodeIndex], &inodes->blq_inodos[srcInodeIndex], SIZE_BLOQUE);
                    ext_bytemaps->bmap_inodos[destInodeIndex] = 1;
                    for (int i = 0; i < numBlocks; i++) {
                        memmove(&ext_superblock->s_first_data_block + destBlockIndices[i],
                                &ext_superblock->s_first_data_block + inodes->blq_inodos[srcInodeIndex].i_nbloque[i], SIZE_BLOQUE);
                        ext_bytemaps->bmap_bloques[destBlockIndices[i]] = 1;
                    }
                    printf("The file %s has been succesfully copied into %s.\n", originName, destName);
                } else {
                    printf("ERROR: Not enough free blocks\n");
                }
            } else {
                printf("ERROR: Not enough inodes\n");
            }
        } else {
            printf("ERROR: destination file %s already exist (remember is case sensitive and try again).\n", destName);
        }
    } else {
        printf("ERROR: file %s is not found. Use 'dir' to check the files ^^\n", originName);
    }

    return 0;
}

void recordInodeAndDirectory(EXT_ENTRADA_DIR *directory, EXT_BLQ_INODOS *inodes, FILE *f) {
    fseek(f, SIZE_BLOQUE * 3, SEEK_SET);
    fwrite(directory, SIZE_BLOQUE, 1, f);
    fwrite(inodes, SIZE_BLOQUE, 1, f);
}

void recordByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *f) {
    fseek(f, SIZE_BLOQUE, SEEK_SET);
    fwrite(ext_bytemaps, SIZE_BLOQUE, 1, f);
}

void recordSuperBlock(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *f) {
    fseek(f, 0, SEEK_SET);
    fwrite(ext_superblock, SIZE_BLOQUE, 1, f);
}

void recordDataBlocks(EXT_DATOS *memdata, FILE *f) {
    fseek(f, SIZE_BLOQUE * 4, SEEK_SET);
    fwrite(memdata, SIZE_BLOQUE, MAX_BLOQUES_DATOS, f);
}

int main() {
    char *command = (char *)malloc(sizeof(char) * COMLEN);
    char *order = (char *)malloc(sizeof(char) * COMLEN);
    char *argument1 = (char *)malloc(sizeof(char) * COMLEN);
    char *argument2 = (char *)malloc(sizeof(char) * COMLEN);

    EXT_SIMPLE_SUPERBLOCK ext_superblock;
    EXT_BYTE_MAPS ext_bytemaps;
    EXT_BLQ_INODOS ext_blq_inodes;
    EXT_ENTRADA_DIR directory[MAX_FICHEROS];
    EXT_DATOS memData[MAX_BLOQUES_DATOS];
    EXT_DATOS fileData[MAX_BLOQUES_PARTICION];
    int recordData = 0;

    FILE *entranceFile = fopen("partition.bin", "rb");
    fread(&fileData, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, entranceFile);

    memcpy(&ext_superblock, (EXT_SIMPLE_SUPERBLOCK *)&fileData[0], SIZE_BLOQUE);
    memcpy(&directory, (EXT_ENTRADA_DIR *)&fileData[3], SIZE_BLOQUE);
    memcpy(&ext_bytemaps, (EXT_BYTE_MAPS *)&fileData[1], SIZE_BLOQUE);
    memcpy(&ext_blq_inodes, (EXT_BLQ_INODOS *)&fileData[2], SIZE_BLOQUE);
    memcpy(&memData, (EXT_DATOS *)&fileData[4], MAX_BLOQUES_DATOS * SIZE_BLOQUE);

    for (;;) {
        do {
            printf(">> ");
            fflush(stdin);
            fgets(command, COMLEN, stdin);
            command[strcspn(command, "\n")] = '\0';
        } while (checkCommand(command, order, argument1, argument2) != 0);

        // Commands: info, bytemaps, dir, rename, print, remove, copy, exit
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
            if (checkCommand("rename", order, argument1, argument2) == 0) {
                renameFile(directory, argument1, argument2);
            }
            continue;
        } else if (strcmp(order, "print") == 0) {
            print(argument1);
            continue;
        } else if (strcmp(order, "remove") == 0) {
            if (checkCommand("remove", order, argument1, NULL) == 0) {
                delete(directory, &ext_blq_inodes, &ext_bytemaps, argument1);
            }
            continue;
                } else if (strcmp(order, "copy") == 0) {
            if (checkCommand("copy", order, argument1, argument2) == 0) {
                copy(directory, &ext_blq_inodes, &ext_bytemaps, &ext_superblock, argument1, argument2);
            }
            continue;
        } else if (strcmp(order, "exit") == 0) {
            printf("You are exiting. Thanks for the visit, see you soon <3\n");
            break;
        } else {
            printf("Command not found. Try again please.\n");
            continue;
        }
    }

    recordSuperBlock(&ext_superblock, entranceFile);
    recordByteMaps(&ext_bytemaps, entranceFile);
    recordInodeAndDirectory(directory, &ext_blq_inodes, entranceFile);
    recordDataBlocks(memData, entranceFile);

    fclose(entranceFile);
    free(command);
    free(order);
    free(argument1);
    free(argument2);

    return 0;
}