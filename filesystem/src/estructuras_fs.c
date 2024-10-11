#include "../include/estructuras_fs.h"


int block_count;
int bitmap_size_in_bytes ;
int fd_bitmap;
FILE* archivo_bitmap;
void* bitmap;
t_bitarray* bitarray;
char * path_archivo_bitmap ;
char * path_archivo_bloques;

void iniciar_fs () {
    
    
    path_archivo_bitmap  = string_new();
    path_archivo_bloques = string_new();    

    string_append(&path_archivo_bitmap,cfg_file_system->MOUNT_DIR); 
    string_append(&path_archivo_bitmap,"/bitmap.dat");
    log_info(logger_file_system, "variables cargadas  %s",path_archivo_bitmap);
    string_append(&path_archivo_bloques,cfg_file_system->MOUNT_DIR); 
    string_append(&path_archivo_bloques,"/bloques.dat");


    //BITMAP//
    if(crear_bitmap (path_archivo_bitmap)>=0 ) {
        log_info(logger_file_system, "Bitmap creado correctamente");
    }
    else {
        log_info(logger_file_system, "Error en creacion de bitmap");
        return EXIT_FAILURE;
    }

    // ARCHIVO DE BLOQUES//
    if(crear_archivo_bloques (path_archivo_bloques,cfg_file_system->BLOCK_SIZE, cfg_file_system->BLOCK_COUNT)>=0 ) {
        log_info(logger_file_system, "Archivo de bloques iniciado correctamente");
    }
    else {
        log_info(logger_file_system, "Error en inicio de Archivo de bloques");
        return EXIT_FAILURE;
    }

    cargar_directorio_fcbs(cfg_file_system->MOUNT_DIR);

    log_info(logger_file_system, "File system iniciado");  
    
}


/////////////////////////////////////////////////INICIAR FS////////////////////////////////////


int crear_bitmap (char * path_archivo_bitmap) {
    block_count = cfg_file_system->BLOCK_COUNT;
    bitmap_size_in_bytes = (block_count + 7) / 8; // 1 bit por bloque

    fd_bitmap = open(path_archivo_bitmap, O_RDWR | O_CREAT, 0666); // Abre archivo para escritura/lectura o lo crea


    archivo_bitmap = fopen(path_archivo_bitmap, "r+");  // Abre archivo para escritura/lectura
    if (archivo_bitmap == NULL) {
        archivo_bitmap = fopen(path_archivo_bitmap, "w+");  // Abre archivo para escritura/lectura
        log_info(logger_file_system, "ARCHIVO DE BITMAP NO ENCONTRADO SE CREA UNO NUEVO");
    }
    fd_bitmap = fileno(archivo_bitmap);

    if (fd_bitmap == -1) {
        perror("Error al abrir o crear el archivo");
        return -1;
    }

    // levanto la info del file descriptor
    struct stat st;
    int stat_result = fstat(fd_bitmap, &st);

    if (stat_result == -1) {
        perror("Error al obtener información del archivo");
        close(fd_bitmap);
        return -1;
    }

    if (st.st_size == 0) {
        // El archivo está vacío, establece el tamaño del archivo
        log_info(logger_file_system, "ESTABLECER EL TAMAÑO DEL BITMAP  : %d Bytes" ,bitmap_size_in_bytes);
        if (ftruncate(fd_bitmap, bitmap_size_in_bytes) == -1) {
            perror("Error al establecer el tamaño del archivo");
            close(fd_bitmap);
            return -1;
        }

        // Mapea el archivo a la memoria
         bitmap = mmap(NULL, block_count , PROT_READ | PROT_WRITE, MAP_SHARED, fd_bitmap, 0);

        if (bitmap == MAP_FAILED) {
            perror("Error al mapear el archivo");
            close(fd_bitmap);
            return -1;
        }

        log_info(logger_file_system, "BITMAP MAPEADO A MEMORIA ");
        // Inicializa el bitmap
        bitarray = bitarray_create_with_mode(bitmap, bitmap_size_in_bytes, LSB_FIRST);
        log_info(logger_file_system, "BITMAP CARGADO EN BITARRAY con : %d bits", bitarray_get_max_bit(bitarray));
        // Marca el primer bloque como utilizado
        //bitarray_set_bit(bitarray, 0);

    } else {
        log_info(logger_file_system, "EL ARCHIVO DEL BITMAP YA EXISTE ");
        // El archivo ya existe, mapea el archivo a la memoria y cargor el bitarray para manejarlo
        bitmap = mmap(NULL, block_count, PROT_READ | PROT_WRITE, MAP_SHARED, fd_bitmap, 0);


        if (bitmap == MAP_FAILED) {
            perror("Error al mapear el archivo");
            close(fd_bitmap);
            return -1;
        }

        // Inicializa el bitmap
        bitarray = bitarray_create(bitmap, bitmap_size_in_bytes);
        log_info(logger_file_system, "BITMAP CARGADO EN BITARRAY ");
        //bitarray = bitarray_create_with_mode(bitmap, block_count, LSB_FIRST);

    }

    return 0;
}// fin crearbitmap

void cerrar_bitmap() {
    munmap(bitmap, bitmap_size_in_bytes);
    close(fd_bitmap);
}


FILE *archivo_bloques;
int fd_archivo_bloques;

int crear_archivo_bloques (char * path_archivo_bloques, int block_size, int block_count) {

    archivo_bloques = fopen(path_archivo_bloques, "r+");  // Abre archivo para escritura/lectura
    if (archivo_bloques == NULL) {
        archivo_bloques = fopen(path_archivo_bloques, "w+");  // Abre archivo para escritura/lectura
        log_info(logger_file_system, "ARCHIVO DE BLOQUES NO ENCONTRADO SE CREA UNO NUEVO");
    }
    uint32_t file_size_in_bytes = block_size * block_count;

    fd_archivo_bloques = fileno(archivo_bloques);

    // calculo el tamaño actual del archivo
    fseek(archivo_bloques, 0, SEEK_END);
    long tamanio_actual = ftell(archivo_bloques);
    rewind(archivo_bloques); // posiciono el puntero al inicio del archivo

    if (tamanio_actual == 0) {
        // El archivo está vacío, establece el tamaño del archivo
        if (ftruncate(fd_archivo_bloques, file_size_in_bytes) == -1) {
            perror("Error al establecer el tamaño del archivo");
            close(fd_archivo_bloques);
            return -1;
        }
    }
    return 0;
}// fin cargar archivo de bloques