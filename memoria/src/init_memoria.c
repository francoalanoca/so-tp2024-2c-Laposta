#include "../include/init_memoria.h"


//-------------------------Definicion de variables globales----------------------


int socket_memoria;
int socket_cpu;
int socket_kernel;
int socket_filesystem;

t_log *logger_memoria; 
t_config *file_cfg_memoria;
t_config_memoria *cfg_memoria;

//void* memoria;                          //espacio de usuario
//t_list* lista_particiones;              //lista de las particiones
//t_list* lista_miniPCBs;                 //lista de los procesos
//pthread_mutex_t mutex_memoria;
//uint32_t cantidad_particiones_memoria;  //seria tam_memoria / tam_pagina
     
//t_bitarray *bitmap_particiones;







//-------------------------Definicion de funciones----------------------------


//-------------------Configuraciones---------------------------
/*
int inicializar_configuraion(char* path_config){

    if (!init(path_config) || !cargar_configuracion(path_config)) {

        cerrar_programa();
        printf("No se pudo inicializar Memoria");
        return EXIT_FAILURE;
    }
}
*/



t_config_memoria *cfg_memoria_start(){

    t_config_memoria *cfg = malloc(sizeof(t_config_memoria));
    return cfg;
}


//Funcion que hace la inicializacion de las config y logger
int init(char *path_config){
    //inicializo estructura de configuracion
    cfg_memoria = cfg_memoria_start();


    //inicializo el archivo de configuracion
    file_cfg_memoria = iniciar_config(path_config,logger_memoria);

    //Retorna lo que devuelve la funcion check luego de validar la propierties
    return checkProperties(path_config);
}




//Funcion que checkea las propiedades
int checkProperties(char *path_config){
    
    //Creamos una config y verificamos si puede ser abierto
    t_config *config = config_create(path_config);

    //Si no pudo ser abierto el config sera informado por consola
    if (config == NULL) {
        printf("Ocurrió un error al intentar abrir el archivo config\n");
        return false;
    }

    char *properties[] = {
            "PUERTO_ESCUCHA",
            "IP_FILESYSTEM",
            "PUERTO_FILESYSTEM",
            "TAM_MEMORIA",
            "PATH_INSTRUCCIONES",
            "RETARDO_RESPUESTA",
            "ESQUEMA",
            "ALGORITMO_BUSQUEDA",
            "PARTICIONES",
            "LOG_LEVEL",
            NULL
            };

    //Verifico si falta alguna de las propiedades en confid
    if (!config_has_all_properties(config, properties)) {
        printf("Propiedades faltantes en el archivo de configuracion\n");
        return false;
    }

    config_destroy(config);

    return true;
}





//Cargo todas las configuracionde en las variables tipo config
int cargar_configuracion(char *path_config){

    file_cfg_memoria = config_create(path_config);

    //Cargo en la variable tipo config las configuraciones iniciales
    cfg_memoria->PUERTO_ESCUCHA = strdup(config_get_string_value(file_cfg_memoria, "PUERTO_ESCUCHA"));
    printf("PUERTO_ESCUCHA cargado correctamente: %s\n", cfg_memoria->PUERTO_ESCUCHA);

    cfg_memoria->IP_FILESYSTEM = strdup(config_get_string_value(file_cfg_memoria, "IP_FILESYSTEM"));
    printf("IP_FILESYSTEM cargado correctamente: %s\n", cfg_memoria->IP_FILESYSTEM);

    cfg_memoria->PUERTO_FILESYSTEM = strdup(config_get_string_value(file_cfg_memoria, "PUERTO_FILESYSTEM"));
    printf("PUERTO_FILESYSTEM cargado correctamente: %s\n", cfg_memoria->PUERTO_FILESYSTEM);

    cfg_memoria->TAM_MEMORIA = config_get_int_value(file_cfg_memoria, "TAM_MEMORIA");
    printf("TAM_MEMORIA cargado correctamente: %d\n", cfg_memoria->TAM_MEMORIA);

    cfg_memoria->PATH_INSTRUCCIONES = strdup(config_get_string_value(file_cfg_memoria, "PATH_INSTRUCCIONES"));
    printf("PATH_INSTRUCCIONES cargado correctamente: %s\n", cfg_memoria->PATH_INSTRUCCIONES);

    cfg_memoria->RETARDO_RESPUESTA = config_get_int_value(file_cfg_memoria, "RETARDO_RESPUESTA");
    printf("RETARDO_RESPUESTA cargado correctamente: %d\n", cfg_memoria->RETARDO_RESPUESTA);

    cfg_memoria->ESQUEMA = strdup(config_get_string_value(file_cfg_memoria, "ESQUEMA"));
    printf("ESQUEMA cargado correctamente: %s\n", cfg_memoria->ESQUEMA);

    cfg_memoria->ALGORITMO_BUSQUEDA = strdup(config_get_string_value(file_cfg_memoria, "ALGORITMO_BUSQUEDA"));
    printf("ALGORITMO_BUSQUEDA cargado correctamente: %s\n", cfg_memoria->ALGORITMO_BUSQUEDA);

    cfg_memoria->PARTICIONES = config_get_array_value(file_cfg_memoria, "PARTICIONES");

    cfg_memoria->LOG_LEVEL = strdup(config_get_string_value(file_cfg_memoria, "LOG_LEVEL"));
    printf("LOG_LEVEL cargado correctamente: %s\n", cfg_memoria->LOG_LEVEL);



    printf("Archivo de configuracion cargado correctamente\n");
    config_destroy(file_cfg_memoria);
    return true;
}



//-------------------Variables---------------------------
//Funcion que inicia las variables necesarias para el funcionamiento de memoria (listas, particiones, usuario, etc)
int inicializar_memoria(){
    logger_memoria = log_create("memoria.log", "Memoria", true, log_level_from_string(cfg_memoria->LOG_LEVEL));

    //Si hubo un error al crear el logger se informara por consola
    if (logger_memoria == NULL) {
        printf("No pude crear el logger\n");
        return false;
    }

	//memoria = malloc(cfg_memoria->TAM_MEMORIA);             //espacio del usuario
	if(strcmp(cfg_memoria->ESQUEMA,"FIJAS") == 0){
        //hacer list_create?
        lista_particiones = cfg_memoria->PARTICIONES;
        inicializar_memoria_particiones_fijas(cfg_memoria->TAM_MEMORIA,lista_particiones,list_size(lista_particiones),cfg_memoria->ALGORITMO_BUSQUEDA);
        
    }
    else{
        lista_particiones = list_create(); 
        inicializar_memoria_particiones_dinamicas(cfg_memoria->TAM_MEMORIA,cfg_memoria->ALGORITMO_BUSQUEDA);
    }
                         //lista en en donde se almacenara las particiones (contiene los proceso) 
	lista_miniPCBs = list_create();
	//pthread_mutex_init(&mutex_memoria, NULL);
	//cantidad_particiones_memoria = list_size(cfg_memoria->PARTICIONES);
	//bitmap_particiones = crear_bitmap(cantidad_particiones_memoria);
    return true;   
}


//Funcion que redondea el valor al multiplo cercano de base y retorna
int redondear_a_multiplo_mas_cercano_de(int base, int valor){
    int v = valor == 0 ? 1 : valor;
    return (int) ceil((float) v / (float) base) * base;
}

//Funcion que en base a la cantidad de frames crea bitmap
t_bitarray *crear_bitmap(int entradas){

    int ent = entradas;
    // si la cantidad de entradas es menor que lo que puede ocupar un Byte * N, entonces redondeamos
    // al multiplo mas cercano mayor que el valor. Entonces si son 4 entradas -> 8, 15 -> 16, etc.
    if (ent % 8 != 0){
        ent = redondear_a_multiplo_mas_cercano_de(8, ent); 
        log_trace(logger_memoria, "tamanio inusual de memoria/pagina causo conflicto, redondeando al multiplo de 8 mas cercano: %i", ent);
    }

    void *puntero = malloc(ent / 8);
    t_bitarray *bitmap = bitarray_create_with_mode(puntero, ent / 8, LSB_FIRST);

    for (int i = 0; i < bitarray_get_max_bit(bitmap); i++){
        bitarray_clean_bit(bitmap, i);
    }

    
    return bitmap;
}






void cerrar_programa(){


    //cortar_conexiones();
    //cerrar_servers();  
    config_destroy(file_cfg_memoria);
    log_info(logger_memoria,"TERMINADA_LA_CONFIG");
    log_info(logger_memoria, "TERMINANDO_EL_LOG");
    log_destroy(logger_memoria);
}


