#define SIZE_BLOQUE 512
#define MAX_INODOS 24
#define MAX_FICHEROS 20
#define MAX_BLOQUES_DATOS 96
#define PRIM_BLOQUE_DATOS 4
#define MAX_BLOQUES_PARTICION MAX_BLOQUES_DATOS + PRIM_BLOQUE_DATOS // 96 + 4 = 100
  //Superblock + Bytemap Inode & Bytemap Block + Inodes + Directory
#define MAX_NUMS_BLOQUE_INODO 7
#define LEN_NFICH 17
#define NULL_INODO 0xFFFF
#define NULL_BLOQUE 0xFFFF

/* Superblock structure */
typedef struct {
  unsigned int s_inodes_count;          /* Partitition inodes */
  unsigned int s_blocks_count;          /* Partitition blocks */
  unsigned int s_free_blocks_count;     /* Free blocks */
  unsigned int s_free_inodes_count;     /* Free inodes */
  unsigned int s_first_data_block;      /* First data block */
  unsigned int s_block_size;        /* Block size in bytes */
  unsigned char s_relleno[SIZE_BLOQUE-6*sizeof(unsigned int)]; /* filling to 0's*/
} EXT_SIMPLE_SUPERBLOCK;

/* Bytemaps in each block */
typedef struct {
  unsigned char bmap_bloques[MAX_BLOQUES_PARTICION];
  unsigned char bmap_inodos[MAX_INODOS];  /* inode 0 & 1 reserved, inode 2 directory */
  unsigned char bmap_relleno[SIZE_BLOQUE-(MAX_BLOQUES_PARTICION+MAX_INODOS)*sizeof(char)];
} EXT_BYTE_MAPS;

/* inode */
typedef struct {
  unsigned int size_fichero;
  unsigned short int i_nbloque[MAX_NUMS_BLOQUE_INODO];
} EXT_SIMPLE_INODE;

/* List of inodes that fit on a block */
typedef struct {
  EXT_SIMPLE_INODE blq_inodos[MAX_INODOS];
  unsigned char blq_relleno[SIZE_BLOQUE-MAX_INODOS*sizeof(EXT_SIMPLE_INODE)];
} EXT_BLQ_INODOS;

/* Individual entry of the directory */
typedef struct {
  char dir_nfich[LEN_NFICH];
  unsigned short int dir_inodo;
  //Agregar nuevo parametro para permisos
    // Habría que hacer un comando para garantizar permisos
    // Los permisos serian un string o un array de strings???
    // Quizás un array de enteros (1 = permiso garantizado, 0 = sin permiso)
} EXT_ENTRADA_DIR;

/* Data blocks */
typedef struct{
  unsigned char dato[SIZE_BLOQUE]; 	
} EXT_DATOS;
