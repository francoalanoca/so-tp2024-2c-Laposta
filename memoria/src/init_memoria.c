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
//uint32_t cantidad_particiones_memoria;  //seria la cantidad de particiones pasadas por config 
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
    crear_lista_procesos();
	//memoria = malloc(cfg_memoria->TAM_MEMORIA);             //espacio del usuario
	if(strcmp(cfg_memoria->ESQUEMA,"FIJAS") == 0){
        
        lista_particiones = char_array_to_list(cfg_memoria->PARTICIONES);
        cantidad_particiones_memoria = list_size(lista_particiones);
        inicializar_memoria_particiones_fijas(cfg_memoria->TAM_MEMORIA,cantidad_particiones_memoria,cfg_memoria->ALGORITMO_BUSQUEDA);
    }
    else{
        lista_particiones = list_create(); 
        inicializar_memoria_particiones_dinamicas(cfg_memoria->TAM_MEMORIA,cfg_memoria->ALGORITMO_BUSQUEDA);
    }
                         //lista en en donde se almacenara las particiones (contiene los proceso) 
	
	    //prueba
        void print_bitarray(t_bitarray *bitarray) {
    // Obtén la cantidad máxima de bits que puede tener el bitarray
    size_t max_bits = bitarray_get_max_bit(bitarray);

    printf("Contenido del bitarray:\n");

    // Recorre cada bit en el bitarray
    for (size_t i = 0; i < max_bits; i++) {
        // Verifica el valor del bit en la posición 'i'
        int bit_value = bitarray_test_bit(bitarray, i) ? 1 : 0;
        printf("%d", bit_value);

        // Opcional: añadir un espacio cada 8 bits para mayor legibilidad
        if ((i + 1) % 8 == 0) {
            printf(" ");
        }
    }
    printf("\n");  // Nueva línea al final de la impresión
}

        printf("Entro a prueba crear_proceso:%d\n", cantidad_particiones_memoria);
        crear_proceso(100,lista_particiones,1);
        printf("Acualizo bitmap:\n");
        print_bitarray(bitmap_particiones);

        inicializar_proceso(1, 64, "iniciar_proceso->archivo_pseudocodigo");
        printf("Acualizo lista miniPCB:\n");
        mostrar_lista_miniPCB(lista_miniPCBs);

 printf("Elemento agreagado a diccionario, nuevo estado del diccionario:\n");
    dictionary_iterator(pids_por_bloque, print_element);
        finalizar_proceso_fijas(1);
        printf("Acualizo bitmap:\n");
        print_bitarray(bitmap_particiones);

        inicializar_hilo(1,1, "archivo_pseudocodigo");
        printf("Acualizo lista miniPCB:\n");
        mostrar_lista_miniPCB(lista_miniPCBs);

        eliminar_hilo_de_lista(lista_miniPCBs,1,1);
        printf("Acualizo lista miniPCB:\n");
        mostrar_lista_miniPCB(lista_miniPCBs);

        eliminar_proceso_de_lista(lista_miniPCBs,1);
        printf("Acualizo lista miniPCB:\n");
        mostrar_lista_miniPCB(lista_miniPCBs);

	
    return true;   
}





//Funcion que redondea el valor al multiplo cercano de base y retorna
int redondear_a_multiplo_mas_cercano_de(int base, int valor){
    int v = valor == 0 ? 1 : valor;
    return (int) ceil((float) v / (float) base) * base;
}






void cerrar_programa(){


    //cortar_conexiones();
    //cerrar_servers();  
    config_destroy(file_cfg_memoria);
    log_info(logger_memoria,"TERMINADA_LA_CONFIG");
    log_info(logger_memoria, "TERMINANDO_EL_LOG");
    log_destroy(logger_memoria);
}

void crear_lista_procesos(){
    lista_miniPCBs = list_create();
}

void inicializar_proceso(uint32_t pid, uint32_t tamanio_proceso, char* archivo_pseudocodigo){
    t_miniPCB* nuevo_proceso = malloc(sizeof(t_miniPCB));
    t_hilo* nuevo_hilo = malloc(sizeof(t_hilo));

    nuevo_proceso->pid = pid;
    nuevo_proceso->hilos = list_create();

    nuevo_hilo->tid = 0;
    nuevo_hilo->registros.PC = 0;
    nuevo_hilo->registros.AX = 0;
    nuevo_hilo->registros.BX = 0;
    nuevo_hilo->registros.CX = 0;
    nuevo_hilo->registros.DX = 0;
    nuevo_hilo->registros.EX = 0;
    nuevo_hilo->registros.FX = 0;
    nuevo_hilo->registros.GX = 0;
    nuevo_hilo->registros.HX = 0;
    nuevo_hilo->lista_de_instrucciones = list_create();
    //PENDIENTE: llenar la lista de instruciones usando funcion que lee el archivo de pseudicodigo

    list_add(nuevo_proceso->hilos,nuevo_hilo);
    //PENDIENTE: ver de donde se consigue la base

    nuevo_proceso->tamanio_proceso = tamanio_proceso;

    list_add(lista_miniPCBs,nuevo_proceso);
}

void inicializar_hilo(uint32_t pid, uint32_t tid, char* nombre_archivo){
    t_hilo* nuevo_hilo = malloc(sizeof(t_hilo));

    //PENDIENTE: ver si ya existe el tid para ese pid
    nuevo_hilo->tid = tid;
    nuevo_hilo->registros.PC = 0;
    nuevo_hilo->registros.AX = 0;
    nuevo_hilo->registros.BX = 0;
    nuevo_hilo->registros.CX = 0;
    nuevo_hilo->registros.DX = 0;
    nuevo_hilo->registros.EX = 0;
    nuevo_hilo->registros.FX = 0;
    nuevo_hilo->registros.GX = 0;
    nuevo_hilo->registros.HX = 0;
    nuevo_hilo->lista_de_instrucciones = list_create();
    //PENDIENTE: llenar la lista de instruciones usando funcion que lee el archivo de pseudicodigo
    asignar_hilo_a_proceso(nuevo_hilo,pid);
    
}

void asignar_hilo_a_proceso(t_hilo* hilo, uint32_t pid){
    bool encontrado = false;
    for (int i = 0; i < list_size(lista_miniPCBs); i++){
        t_miniPCB* miniPCB = list_get(lista_miniPCBs, i);

        if (miniPCB->pid == pid){
			list_add(miniPCB->hilos,hilo);
            printf("Se agrega tid %d a proceso %d\n",hilo->tid, pid);
            encontrado = true;
        }
    }
    if(!encontrado){
        printf("No se asigno hilo ya que no se encuentra el proceso %d\n", pid);
    }

    
}

// Función para convertir un char** en un t_list
t_list* char_array_to_list(char** array) {
    t_list* list = list_create();
    
    // Iterar sobre cada char* en el char**
    for (int i = 0; array[i] != NULL; i++) {
        list_add(list, array[i]);
    }
    
    return list;
}

void eliminar_proceso_de_lista(t_list* lista_procesos, uint32_t pid){
    uint32_t indice_a_eliminar = buscar_indice_pcb_por_pid(lista_procesos,pid);
	t_miniPCB* proceso_a_eliminar = malloc(sizeof(t_miniPCB));
    proceso_a_eliminar = list_get(lista_procesos,indice_a_eliminar);
	list_remove_and_destroy_element(lista_procesos,indice_a_eliminar,liberar_miniPCB);
    printf("Se elimina pid %d\n",pid);
}

void eliminar_hilo_de_lista(t_list* lista_procesos, uint32_t pid, uint32_t tid){
    uint32_t indice_proceso_hilo = buscar_indice_pcb_por_pid(lista_procesos,pid);
	t_miniPCB* proceso_hilo_a_eliminar = malloc(sizeof(t_miniPCB));
    proceso_hilo_a_eliminar = list_get(lista_procesos,indice_proceso_hilo);
    uint32_t indice_hilo_a_eliminar = buscar_indice_hilos_por_tid(proceso_hilo_a_eliminar->hilos,tid);
	list_remove_and_destroy_element(proceso_hilo_a_eliminar->hilos,indice_hilo_a_eliminar,liberar_hilo);
    printf("Se elimina tid %d correspondiente a pid %d\n",tid,pid);
}

void liberar_hilo(t_hilo *hilo) {
    if (hilo == NULL) return; // Verifica que no sea NULL

    // Recorre y libera cada instrucción en la lista 'lista_de_instrucciones'
    for (int i = 0; i < list_size(hilo->lista_de_instrucciones); i++) {
        char *instruccion = list_get(hilo->lista_de_instrucciones, i); // Obtiene la instrucción
        free(instruccion); // Libera la instrucción
    }

    // Libera la lista de instrucciones del hilo
    list_destroy(hilo->lista_de_instrucciones);
    
    // Libera el hilo en sí
    free(hilo);
}

void liberar_miniPCB(t_miniPCB *miniPCB) {
    if (miniPCB == NULL) return; // Verifica que no sea NULL

    // Recorre y libera cada hilo en la lista 'hilos'
    for (int i = 0; i < list_size(miniPCB->hilos); i++) {
        t_hilo *hilo = list_get(miniPCB->hilos, i); // Obtiene el hilo actual
        liberar_hilo(hilo); // Llama a la función de liberar hilo
    }

    // Libera la lista de hilos
    list_destroy(miniPCB->hilos);

    // Finalmente, libera el miniPCB
    free(miniPCB);
}

uint32_t buscar_indice_pcb_por_pid(t_list* lista, uint32_t pid) {
    for (int i = 0; i < list_size(lista); i++) {
        t_miniPCB* proceso = list_get(lista, i);
        if (proceso->pid == pid) {
            return i;
        }
    }
    return -1; 
}

uint32_t buscar_indice_hilos_por_tid(t_list* lista, uint32_t tid) {
    for (int i = 0; i < list_size(lista); i++) {
        t_hilo* hilo = list_get(lista, i);
        if (hilo->tid == tid) {
            return i;
        }
    }
    return -1; 
}

void mostrar_instrucciones(t_list* lista_de_instrucciones) {
    if (lista_de_instrucciones == NULL) return; // Verifica que no sea NULL

    // Recorre y muestra cada instrucción
    for (int i = 0; i < list_size(lista_de_instrucciones); i++) {
        char *instruccion = list_get(lista_de_instrucciones, i);
        printf("    Instrucción: %s\n", instruccion);
    }
}

void mostrar_hilos(t_list* lista_de_hilos) {
    if (lista_de_hilos == NULL) return; // Verifica que no sea NULL

    // Recorre y muestra cada hilo
    for (int i = 0; i < list_size(lista_de_hilos); i++) {
        t_hilo *hilo = list_get(lista_de_hilos, i);
        printf("  Hilo TID: %u\n", hilo->tid);

        // Llama a la función para mostrar las instrucciones de este hilo
        mostrar_instrucciones(hilo->lista_de_instrucciones);
    }
}

void mostrar_lista_miniPCB(t_list* lista_miniPCB) {
    if (lista_miniPCB == NULL) return; // Verifica que no sea NULL

    // Recorre y muestra cada t_miniPCB en la lista
    for (int i = 0; i < list_size(lista_miniPCB); i++) {
        t_miniPCB *miniPCB = list_get(lista_miniPCB, i);
        printf("Proceso PID: %u\n", miniPCB->pid);
        printf("  Tamaño del Proceso: %u\n", miniPCB->tamanio_proceso);
        printf("  Base: %u\n", miniPCB->base);
        
        // Llama a la función para mostrar los hilos de este miniPCB
        mostrar_hilos(miniPCB->hilos);
    }
}

