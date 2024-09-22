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
	//memoria = malloc(cfg_memoria->TAM_MEMORIA);             //espacio del usuario
	if(strcmp(cfg_memoria->ESQUEMA,"FIJAS") == 0){
        
        lista_particiones = char_array_to_list(cfg_memoria->PARTICIONES);
        cantidad_particiones_memoria = list_size(lista_particiones);
        inicializar_memoria_particiones_fijas(cfg_memoria->TAM_MEMORIA,cantidad_particiones_memoria,cfg_memoria->ALGORITMO_BUSQUEDA);
    }else{

        if (strcmp(cfg_memoria->ESQUEMA,"DINAMICAS") == 0){
            inicializar_memoria_particiones_dinamicas(memoria_usuario);
        }
        
    }
    
                 
	/*
	    //PRUEBA
        
        printf("Entro a prueba crear_proceso:%d\n", cantidad_particiones_memoria);
        crear_proceso(100,lista_particiones,1);
    
        printf("Acualizo bitmap:\n");
        print_bitarray(bitmap_particiones);

        inicializar_proceso(1,64);
        printf("Acualizo lista miniPCB:\n");
        mostrar_lista_miniPCB(lista_miniPCBs); 

        inicializar_hilo(1,1, "archivo_pseudocodigo");
        printf("Acualizo lista miniPCB:\n");
        mostrar_lista_miniPCB(lista_miniPCBs);

        //PRUEBA ESCRIBIR Y LEER
        printf("INICIO PRUEBA ESCRIBIR Y LEER\n");
        t_escribir_leer* peticion_leer = malloc(sizeof(t_escribir_leer));
        peticion_leer->pid = 1;
        peticion_leer->direccion_fisica = 5;
        peticion_leer->tamanio = 4;
        peticion_leer->valor = "Hi!";

        if(write_mem(peticion_leer->direccion_fisica, peticion_leer->valor, peticion_leer->tamanio)){
				
			 printf("Escritura exitosa: \"%s\" en la dirección %u\n", peticion_leer->valor, peticion_leer->direccion_fisica);
         } 
         else {
             printf("Error al escribir en la dirección %u\n", peticion_leer->direccion_fisica);
			}

            //write_mem(6, "Hola", 5);

        //LEER
            char* respuesta_leer = malloc(4);
            memset(respuesta_leer, 0, sizeof(respuesta_leer));
			if(read_mem(peticion_leer->direccion_fisica,respuesta_leer)){
			 printf("Lectura exitosa: \"%s\" desde la dirección %u\n", respuesta_leer, peticion_leer->direccion_fisica );
            } 
            else {
              printf("Error al leer desde la dirección %u\n", peticion_leer->direccion_fisica );
			}
        //FIN ESCRIBIR Y LEER

        eliminar_hilo_de_lista(lista_miniPCBs,1,1);
        printf("Acualizo lista miniPCB:\n");
        mostrar_lista_miniPCB(lista_miniPCBs);

        finalizar_proceso_fijas(1);
        printf("Acualizo bitmap:\n");
        print_bitarray(bitmap_particiones);

        eliminar_proceso_de_lista(lista_miniPCBs,1);
        printf("Acualizo lista miniPCB:\n");
        mostrar_lista_miniPCB(lista_miniPCBs);
        //FIN PRUEBA
       */ 
    return true;   
}






//Funcion que inicia las variables de memoria en dinamica
void inicializar_memoria_particiones_dinamicas(void *tamanio_memoria) {

    lista_particiones_dinamicas = list_create();
    t_particion_dinamica* particion_dinamica_inicial = malloc(sizeof(t_particion_dinamica));

    particion_dinamica_inicial->pid = 0;                   // 0 indica que está libre
    particion_dinamica_inicial->tid = 0;
    particion_dinamica_inicial->inicio = 0;
    particion_dinamica_inicial->tamanio = cfg_memoria->TAM_MEMORIA;
    particion_dinamica_inicial->ocupado = false;
    //lista_particiones_dinamicas->siguiente = NULL;

    list_add(lista_particiones_dinamicas, particion_dinamica_inicial);
}


void cerrar_programa(){


    //cortar_conexiones();
    //cerrar_servers();  
    config_destroy(file_cfg_memoria);
    log_info(logger_memoria,"TERMINADA_LA_CONFIG");
    log_info(logger_memoria, "TERMINANDO_EL_LOG");
    log_destroy(logger_memoria);
}

void inicializar_proceso(uint32_t pid, uint32_t tamanio_proceso){
    t_miniPCB* nuevo_proceso = malloc(sizeof(t_miniPCB));
    //t_hilo* nuevo_hilo = malloc(sizeof(t_hilo));

    nuevo_proceso->pid = pid;
    nuevo_proceso->hilos = list_create();

    // nuevo_hilo->tid = 0;
    // nuevo_hilo->registros.PC = 0;
    // nuevo_hilo->registros.AX = 0;
    // nuevo_hilo->registros.BX = 0;
    // nuevo_hilo->registros.CX = 0;
    // nuevo_hilo->registros.DX = 0;
    // nuevo_hilo->registros.EX = 0;
    // nuevo_hilo->registros.FX = 0;
    // nuevo_hilo->registros.GX = 0;
    // nuevo_hilo->registros.HX = 0;
    // nuevo_hilo->lista_de_instrucciones = list_create();
    // leer_instrucciones_particiones_fijas(archivo_pseudocodigo,nuevo_hilo);
    

 
    //uint32_t indice_bloque_a_liberar = buscar_indice_bloque_por_pid(pids_por_bloque,pid);

    //printf("El indice en la lista del bloque a liberar es: %d\n",indice_bloque_a_liberar);

    //t_pid_por_bloque* bloque_x_pid = list_get(pids_por_bloque,indice_bloque_a_liberar);

    //nuevo_hilo->registros.base = calcular_base_proceso_fijas(bloque_x_pid->bloque, lista_particiones);

    //nuevo_hilo->registros.limite = tamanio_proceso;

    //list_add(nuevo_proceso->hilos,nuevo_hilo);

    list_add(lista_miniPCBs,nuevo_proceso);
}

void inicializar_hilo(uint32_t pid, uint32_t tid, char* nombre_archivo, uint32_t tamanio_proceso){
    t_hilo* nuevo_hilo = malloc(sizeof(t_hilo));

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

    uint32_t indice_bloque_a_liberar = buscar_indice_bloque_por_pid(pids_por_bloque,pid);

    printf("El indice en la lista del bloque a liberar es: %d\n",indice_bloque_a_liberar);

    t_pid_por_bloque* bloque_x_pid = list_get(pids_por_bloque,indice_bloque_a_liberar);

    nuevo_hilo->registros.base = calcular_base_proceso_fijas(bloque_x_pid->bloque, lista_particiones);

    nuevo_hilo->registros.limite = tamanio_proceso;
    
    nuevo_hilo->lista_de_instrucciones = list_create();
    leer_instrucciones_particiones_fijas(nombre_archivo,nuevo_hilo);
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
	//t_miniPCB* proceso_a_eliminar = malloc(sizeof(t_miniPCB));
    //proceso_a_eliminar = list_get(lista_procesos,indice_a_eliminar);
	list_remove_and_destroy_element(lista_procesos,indice_a_eliminar,(void*)liberar_miniPCB);
    printf("Se elimina pid %d\n",pid);
}

void eliminar_hilo_de_lista(t_list* lista_procesos, uint32_t pid, uint32_t tid){
    uint32_t indice_proceso_hilo = buscar_indice_pcb_por_pid(lista_procesos,pid);
	t_miniPCB* proceso_hilo_a_eliminar = malloc(sizeof(t_miniPCB));
    proceso_hilo_a_eliminar = list_get(lista_procesos,indice_proceso_hilo);
    uint32_t indice_hilo_a_eliminar = buscar_indice_hilos_por_tid(proceso_hilo_a_eliminar->hilos,tid);
	list_remove_and_destroy_element(proceso_hilo_a_eliminar->hilos,indice_hilo_a_eliminar,(void*)liberar_hilo);
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
        printf("  Registro PC: %u\n", hilo->registros.PC);
        printf("  Registro AX: %u\n", hilo->registros.AX);
        printf("  Registro BX: %u\n", hilo->registros.BX);
        printf("  Registro CX: %u\n", hilo->registros.CX);
        printf("  Registro DX: %u\n", hilo->registros.DX);
        printf("  Registro EX: %u\n", hilo->registros.EX);
        printf("  Registro FX: %u\n", hilo->registros.FX);
        printf("  Registro GX: %u\n", hilo->registros.GX);
        printf("  Registro HX: %u\n", hilo->registros.HX);


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
        printf("  Tamaño del Proceso: %u\n", miniPCB->registros.limite);
        printf("  Base: %u\n", miniPCB->registros.base);
        
        // Llama a la función para mostrar los hilos de este miniPCB
        mostrar_hilos(miniPCB->hilos);
    }
}

void print_bitarray(t_bitarray *bitarray) {
    // Obtengo la cantidad máxima de bits que puede tener el bitarray
    size_t max_bits = bitarray_get_max_bit(bitarray);
    printf("Contenido del bitarray:\n");

    // Recorro cada bit en el bitarray
    for (size_t i = 0; i < max_bits; i++) {
        // Verifico el valor del bit en la posición 'i'
        int bit_value = bitarray_test_bit(bitarray, i) ? 1 : 0;
        printf("%d", bit_value);

        //añado un espacio cada 8 bits para mayor legibilidad
        if ((i + 1) % 8 == 0) {
            printf(" ");
        }
    }
    printf("\n");  // Nueva línea al final de la impresión
}

bool existe_proceso_en_memoria(uint32_t pid){

    for (int i = 0; i < list_size(lista_miniPCBs); i++) {
        t_miniPCB* proceso_actual = list_get(lista_miniPCBs, i);

        if (proceso_actual->pid == pid) {
            return true;
        }
    }
    return false; 

}

bool existe_hilo_en_memoria(uint32_t pid, uint32_t tid){

    for (int i = 0; i < list_size(lista_miniPCBs); i++) {
        t_miniPCB* proceso_actual = list_get(lista_miniPCBs, i);

        if (proceso_actual->pid == pid) {
            for (int j = 0; j < list_size(proceso_actual->hilos); j++) {
                t_hilo* hilo_actual = list_get(proceso_actual->hilos, j);

                if (hilo_actual->tid == tid) {
                    return true;
                }
            }
        }
    }
    return false; 

}

uint32_t buscar_tamanio_proceso_por_pid(uint32_t pid){

    for (int i = 0; i < list_size(lista_miniPCBs); i++) {
        t_miniPCB* proceso_actual = list_get(lista_miniPCBs, i);

        if (proceso_actual->pid == pid) {
            return proceso_actual->registros.limite;
        }
    }
    return -1; 

}

// Función que busca un miniPCB con el pid y un hilo con el tid y devuelve el contexto asociado
t_m_contexto* buscar_contexto_en_lista(uint32_t pid, uint32_t tid) {
    size_t cantidad_miniPCBs = list_size(lista_miniPCBs);

    for (size_t i = 0; i < cantidad_miniPCBs; i++) {
        t_miniPCB* miniPCB = list_get(lista_miniPCBs, i);

        if (miniPCB->pid == pid) {
            size_t cantidad_hilos = list_size(miniPCB->hilos);

            for (size_t j = 0; j < cantidad_hilos; j++) {
                t_hilo* hilo = list_get(miniPCB->hilos, j);

                if (hilo->tid == tid) {
                    t_m_contexto* contexto = malloc(sizeof(t_m_contexto));
                    contexto->registros = hilo->registros;  
                    //contexto->base = miniPCB->base;        
                    //contexto->limite = miniPCB->tamanio_proceso; 

                    return contexto;  
                }
            }
        }
    }

    // Si no encontramos el miniPCB o el hilo, retornamos NULL
    return NULL;
}

// Función que actualiza los registros del hilo con el tid correspondiente dentro del miniPCB con el pid correspondiente
bool actualizar_contexto(t_m_contexto* contexto) {
    // Verificamos si el contexto es válido
    if (contexto == NULL) {
        return false;
    }

    // Iteramos sobre la lista de miniPCBs
    size_t cantidad_miniPCBs = list_size(lista_miniPCBs);
    for (size_t i = 0; i < cantidad_miniPCBs; i++) {
        t_miniPCB* miniPCB = list_get(lista_miniPCBs, i);

        // Verificamos si el pid coincide
        if (miniPCB->pid == contexto->pid) {
            // Ahora iteramos sobre la lista de hilos dentro de este miniPCB
            size_t cantidad_hilos = list_size(miniPCB->hilos);
            for (size_t j = 0; j < cantidad_hilos; j++) {
                t_hilo* hilo = list_get(miniPCB->hilos, j);

                // Verificamos si el tid coincide
                if (hilo->tid == contexto->tid) {
                    // Actualizamos los registros del hilo con los del contexto
                    hilo->registros = contexto->registros;
                    return true;  // Retornamos true para indicar que se actualizó correctamente
                }
            }
        }
    }

    // Si no encontramos el pid o el tid, retornamos false
    return false;
}


// Función que busca el miniPCB al que pertenece la dirección física
t_miniPCB* obtener_particion_proceso(uint32_t direccion_fisica) {
    for (int i = 0; i < list_size(lista_particiones); i++) {
        t_miniPCB* proceso = list_get(lista_particiones, i);
        //printf("roceso->base:%d,proceso->tamanio:%d\n", proceso->base,proceso->tamanio_proceso);
        if (direccion_fisica >= proceso->registros.base && 
            direccion_fisica < proceso->registros.base + proceso->registros.limite) {
            return proceso;
        }
    }
    return NULL; // Si no se encuentra un proceso que contenga la dirección
}

bool write_mem(uint32_t direccion_fisica, char* valor, uint32_t longitud) {
   // Obtenemos el proceso correspondiente a la dirección
    t_miniPCB* proceso = obtener_particion_proceso(direccion_fisica);
    //printf("Escribire el bloque correspondiente a particion %d:\n", proceso->pid);
    if (proceso == NULL) {
        // No se encontró ningún proceso que contenga la dirección
        return false;
    }

    // Verificamos que los bytes que queremos escribir no se pasen del espacio del proceso
    if (direccion_fisica + longitud > proceso->registros.base + proceso->registros.limite) {
        // No hay suficiente espacio para escribir la cadena completa
        return false;
    }

    // Calculamos la posición en la memoria a partir de la dirección física
    uint8_t* posicion_memoria = (uint8_t*)memoria_usuario + direccion_fisica;

    // Escribimos la cadena en la posición calculada
    memcpy(posicion_memoria, valor, longitud);

    return true;  // La escritura fue exitosa
}

// Función para leer 4 bytes si la dirección está dentro del proceso
bool read_mem(uint32_t direccion_fisica, char* resultado) {
    uint32_t longitud = 4;  // Siempre leemos 4 bytes

    // Obtenemos el proceso correspondiente a la dirección
    t_miniPCB* proceso = obtener_particion_proceso(direccion_fisica);
    //printf("Leere el bloque correspondiente a particion %d:\n", proceso->pid);
    if (proceso == NULL) {
        // No se encontró ningún proceso que contenga la dirección
        return false;
    }

    // Verificamos que los 4 bytes no se pasen del espacio del proceso
    if (direccion_fisica + longitud > proceso->registros.base + proceso->registros.limite) {
        // No hay suficiente espacio para leer los 4 bytes
        return false;
    }

    // Calculamos la posición en la memoria a partir de la dirección física
    uint8_t* posicion_memoria = (uint8_t*)memoria_usuario + direccion_fisica;

    // Leemos los 4 bytes de la posición calculada
    memcpy(resultado, posicion_memoria, longitud);

    return true;  // La lectura fue exitosa
}



