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

/////////////////////////////////////////////////////// FCB ///////////////////////////////////////////////////
 t_FCB* inicializar_fcb(char* nombre_archivo, uint32_t tamanio_archivo, uint32_t primer_bloque) {
    t_FCB* fcb = malloc(sizeof(t_FCB));
    if (!fcb) {
        perror("Error al inicializar el FCB");
        exit(EXIT_FAILURE);
    }
    memset(fcb,0, sizeof (t_FCB));

    fcb->nombre_archivo = strdup(nombre_archivo);
    fcb->tamanio_archivo = tamanio_archivo;
    fcb->primer_bloque = primer_bloque;
    return fcb;
}


void persistir_fcb(t_FCB *fcb) {

    char path_directory_fcb [100] ;
    strcpy(path_directory_fcb, cfg_file_system->MOUNT_DIR);
    char file_path[100]; // Tamaño suficiente para almacenar la ruta completa del archivo

    snprintf(file_path, sizeof(file_path), "%s/%s",path_directory_fcb,fcb->nombre_archivo);

    t_config* file_fcb = config_create(file_path);
    if (file_fcb == NULL) {
        log_info(logger_file_system, "ERROR AL CREAR  CONFIG PARA PERSISTIR ARCHIVO FCB %s", file_path);
    }

    char* tamanio_archivo = uint32_to_string(fcb->tamanio_archivo);
    char* primer_bloque = uint32_to_string(fcb->primer_bloque);

    config_set_value(file_fcb,"nombre_archivo", fcb->nombre_archivo );
    log_info(logger_file_system, "PERSISTIDO PARAMETRO nombre_archivo: %s", fcb->nombre_archivo);

    config_set_value(file_fcb,"tamanio_archivo", tamanio_archivo );
    log_info(logger_file_system, "PERSISTIDO PARAMETRO tamanio_archivo: %d", fcb->tamanio_archivo);

    config_set_value(file_fcb,"primer_bloque", primer_bloque );
    log_info(logger_file_system, "PERSISTIDO PARAMETRO primer_bloque: %d", fcb->primer_bloque);

    if (!config_save(file_fcb)){
        perror("Error al guardar fcb");
    };

    config_destroy(file_fcb);

}


/////////////////////////////////////////// FUNCIONALIDADES /////////////////////////////////////////////////////////////////

 void dumpear(t_dumped* dumped, int socket_cliente){
    log_info(logger_file_system, "Nombre de archivo recibido: %s",dumped->nombre_archivo);
    log_info(logger_file_system, "Contenido de archivo: %s",dumped->contenido);
    log_info(logger_file_system, "tamanio solicitodo: %d",dumped->tamanio_archivo);
    if (hay_espacio_total_disponible(dumped->tamanio_archivo))
    {
        t_list* lista_bloques = malloc(sizeof(t_list));
        
        grabar_bloques( asignar_bloques(dumped->tamanio_archivo) , dumped->contenido);
        //enviar_resultado_memoria(PEDIDO_MEMORY_DUMP_RTA_OK,socket_cliente);
        list_destroy(lista_bloques);
    }else {
        enviar_resultado_memoria(PEDIDO_MEMORY_DUMP_RTA_ERROR,socket_cliente);
    }


 }


t_list* asignar_bloques(uint32_t tamanio) {

    log_info(logger_file_system, "entramos en asignar bloques");
    log_info(logger_file_system, "tamanio solicitodo: %d",tamanio);
    uint32_t cant_bloques_nuevos = (tamanio / cfg_file_system->BLOCK_SIZE)+1;
    t_list* lista_punteros;
    lista_punteros = malloc(sizeof(t_list));
    int i;
    log_info(logger_file_system, "CANTIDAD DE BLOQUES NECESARIOS : %d",cant_bloques_nuevos);    

    for (i = 0; i < cant_bloques_nuevos; i++) {
        uint32_t posicion_bit_libre =  encontrar_bit_libre(bitarray);
        //asignar bloques en bitmap
        bitarray_set_bit(bitarray, posicion_bit_libre);
        //colecto las posicones para luego escribir sobre ellas
        list_add(lista_punteros,posicion_bit_libre);
        
    }
    sincronizar_bitmap ();
    return lista_punteros;
 }


 void grabar_bloques(t_list* lista_bloques, char *datos_escribir) {
   int  block_size = cfg_file_system->BLOCK_SIZE;
   int datos_length = strlen(datos_escribir);
    //grabar bloque de punteros la posición de los punteros
        // el primer bloque de la lista es el bloque de punteros
    escribir_punteros (lista_bloques);    
       
    //grabar bloques de datos con los datos
        // desde el segundo bloque
    for (int i = 1; i < list_size(lista_bloques); i++) {
        int offset = (i - 1) * block_size; // Calcular el offset para el bloque actual
        int tamanio_fragmento;

        // calculo tamanio de fragmentos para no tirar segfault o escribir basura
        if (datos_length - offset < block_size) {
            tamanio_fragmento = datos_length - offset;
        } else {
            tamanio_fragmento = block_size;
        }

        // Escribir el bloque con el fragmento correspondiente
        escribir_bloque(list_get(lista_bloques,i), tamanio_fragmento, datos_escribir + offset);
    }
    fflush(archivo_bloques); //agrego esto para "garantizar" que se escriba el archivo de bloques si no tengo que esperar un fclose
 }

void escribir_punteros (uint32_t* lista_bloques ){

    int retardo = cfg_file_system->RETARDO_ACCESO_BLOQUE / 1000;
    sleep(retardo);
    uint32_t posicion_bloque_punteros = list_get(lista_bloques,0);

    // en el archivo se ubica en el byte 0 del bloque de punteros
    if (fseek(archivo_bloques,(posicion_bloque_punteros * cfg_file_system->BLOCK_SIZE ) , SEEK_SET)!= 0){
        log_info(logger_file_system,"Error al mover el puntero de archivo al bloque: %d ",posicion_bloque_punteros);
        return -1;
    }  else{
        log_info(logger_file_system, "PUNTERO POSICIONADO EN BLOQUE: %d ",posicion_bloque_punteros );
    };
     log_info(logger_file_system, "vamos a escribir el bloque de punteros tamaño de lista %d ",list_size(lista_bloques) );
      log_info(logger_file_system, "contenido de lista %d ",list_get(lista_bloques,0) );
      log_info(logger_file_system, "contenido de lista %d ",list_get(lista_bloques,1) );
      log_info(logger_file_system, "contenido de lista %d ",list_get(lista_bloques,2) );

    
    if (archivo_bloques == NULL) {
        perror("Error al abrir el archivo");
        return;
    }      
    // escribe cada  puntero de tamaño 4 bytes dentro del bloque de punteros
    for (int i = 1; i < list_size(lista_bloques); i++) {
         log_info(logger_file_system, "puntero a escribir %d ",list_get(lista_bloques,i) );
            uint32_t puntero = list_get(lista_bloques,i);
        if (fwrite(&puntero, 4, 1, archivo_bloques)<= 0){
            log_info(logger_file_system,"Error al escribir el bloque: %d ",posicion_bloque_punteros);
            return -1;
        }  else{
            log_info(logger_file_system, "BLOQUE: %d ESCRITO con valor %d",posicion_bloque_punteros, list_get(lista_bloques,i));
        };
    };
    fflush(archivo_bloques); //agrego esto para "garantizar" que se escriba el archivo de bloques si no tengo que esperar un fclose
}

void escribir_bloque (int numero_bloque, int tamanio_escritura, char *datos_escribir){

    int retardo = cfg_file_system->RETARDO_ACCESO_BLOQUE / 1000;
    sleep(retardo);

    if (fseek(archivo_bloques,(numero_bloque * cfg_file_system->BLOCK_SIZE ) , SEEK_SET)!= 0){
        log_info(logger_file_system,"Error al mover el puntero de archivo al bloque: %d ",numero_bloque);
        return -1;
    }  else{
        log_info(logger_file_system, "PUNTERO POSICIONADO EN BLOQUE: %d ",numero_bloque );
    };

    if (fwrite(datos_escribir, tamanio_escritura, 1, archivo_bloques)<= 0){
        log_info(logger_file_system,"Error al escribir el bloque: %d ",numero_bloque);
        return -1;
    }  else{
        log_info(logger_file_system, "BLOQUE: %d ESCRITO con valor %s",numero_bloque, datos_escribir);
    };
    fflush(archivo_bloques); //agrego esto para "garantizar" que se escriba el archivo de bloques si no tengo que esperar un fclose
}


////////////////////////////////////////////// UTILIDAD///////////////////////////////////////////////////////////////////////


uint32_t encontrar_bit_libre(t_bitarray* bitarray_in) {

    log_info(logger_file_system, "tamaño del bitarray %d %d",bitarray_get_max_bit(bitarray_in), bitarray_test_bit(&bitarray_in, 0));
    uint32_t i;
    for (i = 0; i < bitarray_get_max_bit(bitarray_in); i++) {
        if (!bitarray_test_bit(bitarray_in, i)) {
            log_info(logger_file_system, "Acceso a Bitmap - Bloque: %d - Estado: libre", i); 
            return i;
        }else {
            log_info(logger_file_system, "Acceso a Bitmap - Bloque: %d - Estado: ocupado", i); 
        }
    }
    return -1; // Retorna -1 si no se encuentra ningún bit en 0
}

void sincronizar_bitmap (){
    memcpy(bitmap, bitarray->bitarray, bitmap_size_in_bytes);
    int resultado_sync = msync(bitmap, bitmap_size_in_bytes, MS_SYNC);
    int resultado_fync= fsync(fd_bitmap);
    if (resultado_sync == -1 || resultado_fync == -1) {
        perror("Error al sincronizar el bitmap");        
    } else {
        log_info(logger_file_system, "SINCRONIZACION DE BITMAP EXITOSA");
    }
}


bool hay_espacio_total_disponible(int espacio_necesario){
    int espacio_disponible = 0;
    for (int i = 0; i < bitarray_get_max_bit(bitarray); i++) {
        if (!bitarray_test_bit(bitarray, i)) {
            espacio_disponible++;
        }
    }
    log_info(logger_file_system,"Cantidad de bits %d:",  bitarray_get_max_bit(bitarray));
    log_info(logger_file_system,"Bloques/bits libres %d:",  espacio_disponible);
    log_info(logger_file_system,"Espacio total disponible %d:",  espacio_disponible*cfg_file_system->BLOCK_SIZE);
return espacio_disponible*cfg_file_system->BLOCK_SIZE >= espacio_necesario+cfg_file_system->BLOCK_SIZE; // agrego el tamaño del bloque de punteros
}   



void imprimir_estado_bitarray() {

    log_info(logger_file_system, "ESTADO BITARRAY:");
    uint32_t i;
    for (i = 0; i < bitarray_get_max_bit(bitarray); i++) {
        if (bitarray_test_bit(bitarray, i)) {
            log_info(logger_file_system,"%d",1);         
        }else {
            log_info(logger_file_system,"%d",0); 
        }
    }
}

void free_t_FCB(t_FCB* fcb) {

        free(fcb);
 
}

void enviar_resultado_memoria(op_code codigo_operacion, int socket_memoria){

    if (send(socket_memoria, &codigo_operacion, sizeof(uint32_t), MSG_WAITALL) != sizeof(uint32_t)) {
        log_debug(logger_file_system, "Error al enviar respuesta de handshake a kernel"); // guarda con este log
    }       
}

char* uint32_to_string (uint32_t number) {
    char* str ;
    if (asprintf(&str, "%u", number)== -1) {
        //error al asignar memoria
        return NULL;
    }
    return str;
}