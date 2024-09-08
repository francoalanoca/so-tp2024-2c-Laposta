#include "../include/particion_dinamica.h"


void crear_proceso(uint32_t proceso_pid, uint32_t tamanio_proceso){

    if (strcmp(cfg_memoria->ESQUEMA, "DINAMICAS") == 0){
        crear_proceso_dinamico(proceso_pid, tamanio_proceso);
    }else{
        if (strcmp(cfg_memoria->ESQUEMA, "FIJAS") == 0)
            crear_proceso_fijo(proceso_pid, tamanio_proceso);
    }
    
}


/*
//Funcion que crea las estructuras del proceso
void crear_proceso_dinamico(uint32_t proceso_pid, uint32_t tamanio_proceso){

    log_info(logger_memoria, "Creacion del proceso PID %i \n", proceso_pid);
    log_info(logger_memoria, "Iniciando estructuras \n");

    t_particion_dinamica* particion_proceso = malloc(sizeof(t_particion_dinamica));
    particion_proceso = asignar_memoria(proceso_pid, tamanio_proceso);

    
    log_info(logger_memoria, "Creada particion \n");
    list_add(lista_particiones, particion_proceso);
}




t_particion_dinamica *asignar_memoria(uint32_t proceso_pid, uint32_t tamanio_proceso){

    t_particion_dinamica *particion_resultante = NULL;

    particion_resultante->pid = proceso_pid;
    particion_resultante->tamanio = tamanio_proceso;

    // Selecciona la partición según el algoritmo
    if (strcmp(cfg_memoria->ALGORITMO_BUSQUEDA, "FIRST_FIT") == 0) {
        particion_resultante = buscar_first_fit(tamanio_proceso);

    } else if (strcmp(cfg_memoria->ALGORITMO_BUSQUEDA, "BEST_FIT") == 0) {
        particion_resultante = buscar_best_fit(tamanio_proceso);

    } else if (strcmp(cfg_memoria->ALGORITMO_BUSQUEDA, "WORST_FIT") == 0) {
        particion_resultante = buscar_worst_fit(tamanio_proceso);
    }

    //particion_resultante->inicio = memoria;

    // Si es null no hay memoria
    if (particion_resultante == NULL) {
        return NULL; 
    }

    // Marcar la partición como ocupada
    particion_resultante->ocupado = true;

    return particion_resultante;
}



//Funcion que busca la primer particion
t_particion_dinamica *buscar_first_fit(uint32_t tamanio_proceso){

    //Recorremos la lista de particiones comparando en cada iteracion
    for (int i = 0; i < list_size(lista_particiones); i++) {
        t_particion_dinamica *particion = list_get(lista_particiones, i);

        //Verificamos que la particion obtenida este libre y sea >= al proceso
        if (!particion->ocupado && particion->tamanio >= tamanio_proceso) {
            return particion;
        }
    }
    return NULL;
}


//Funcion que busca la mejor particion
t_particion_dinamica *buscar_best_fit(uint32_t tamanio_proceso){

    //iniciamos la variable mejor en Null
    t_particion_dinamica *mejor_particion = NULL;

    //Recorremos la lista de particiones comparando en cada iteracion
    for (int i = 0; i < list_size(lista_particiones); i++) {
        t_particion_dinamica *particion = list_get(lista_particiones, i);

        //Verificamos que la particion obtenida este libre y sea >= al proceso
        if (!particion->ocupado && particion->tamanio >= tamanio_proceso) {

            //Verificamos si todavia no hay una mejor o si el tamaño particion actual es menor a la mejor actual 
            if (mejor_particion == NULL || particion->tamanio < mejor_particion->tamanio) {
                mejor_particion = particion;
            }
        }
    }

    //Retornamos la mejor o si no hay Null
    return mejor_particion;
}


//Funcion que busca la peor particion
t_particion_dinamica *buscar_worst_fit(uint32_t tamanio_proceso){

    //iniciamos la variable mejor en Null
    t_particion_dinamica *peor_particion = NULL;

    //Recorremos la lista de particiones comparando en cada iteracion
    for (int i = 0; i < list_size(lista_particiones); i++){
        t_particion_dinamica *particion = list_get(lista_particiones, i);

        //Verificamos que la particion obtenida este libre y sea >= al proceso
        if (!particion->ocupado && particion->tamanio >= tamanio_proceso){

            //Verificamos si todavia no hay una peor o si el tamaño particion actual es mayor a la peor actual 
            if (peor_particion == NULL || particion->tamanio > peor_particion->tamanio) {
                peor_particion = particion;
            }
        }
    }

    //Retornamos la peor o si no hay Null
    return peor_particion;
}








//Funcion que unifica bloques continuos que estan vacios
void unificar_bloques(){
    t_particion primer_bloque;
    t_particion segundo_bloque;
    t_particion bloque_resultante;

    if(primer_bloque->is_free == "true" && segundo_bloque->is_free == "true"){

        bloque_resultante->start = primer_bloque->start;
        bloque_resultante->size = primer_bloque->size + segundo_bloque->size;
        bloque_resultante->is_free = true;
        bloque_resultante->next = segundo_bloque->next;

        
    }
}







//Funciones para la finalizacion de un proceso

t_particion_dinamica *busco_particion_por_PID(uint32_t proceso_pid){

    log_trace(logger_memoria, "Buscando la particion por PID");

    t_particion_dinamica *particion;

    //Recorremos la lista que contiene particiones
    for (int i = 0; i < list_size(lista_particiones); i++){

        //Sacamos una particion de la lista
        particion = list_get(lista_particiones, i);

        //Si el id de la tabla es el mismo que la tabla, la retorna
        if (proceso_pid == particion->id)
            return particion;
    }

    log_error(logger_memoria, "PID - %d No se encontro la Particion", proceso_pid);
    abort();
}



t_miniPCB *busco_un_proceso_PID(uint32_t proceso_pid){

    log_trace(logger_memoria, "Buscando el proceso por PID");

    t_miniPCB *proceso;

    //Recorremos la lista que contiene la lista de procesos
    for (int i = 0; i < list_size(lista_miniPCBs); i++){

        //Sacamos un proceso de la lista
        proceso = list_get(lista_miniPCBs, i);

        //Si el id del proceso es el mismo que el proceso buscado, la retorna
        if (proceso_pid == proceso->pid)
            return proceso;
    }

    log_error(logger_memoria, "PID - %d No se encontro el proceso", proceso_pid);
    abort();
}




t_miniPCB *busco_un_hilo_TID(uint32_t hilo_pid){

    log_trace(logger_memoria, "Buscando el hilo por TID");

    t_miniPCB *proceso;

    //Recorremos la lista que contiene la lista de hilos
    for (int i = 0; i < list_size(lista_miniPCBs); i++){

        //Sacamos un hilo de la lista
        proceso = list_get(lista_miniPCBs, i);

        //Si el id del proceso es el mismo que el proceso buscado, la retorna
        if (proceso_pid == proceso->pid)
            return proceso;
    }

    log_error(logger_memoria, "PID - %d No se encontro el proceso", proceso_pid);
    abort();
}



//Funcion que en base al id de un proceso finalizamos sus estructuras
void finalizar_proceso(uint32_t proceso_pid){

    log_trace(logger_memoria, "Liberacion del proceso PID %i", proceso_pid);
    t_particion_dinamica *particion = busco_particion_por_PID(proceso_pid);
    t_miniPCB *proceso = busco_un_proceso_PID(proceso_pid);

    //Recorremos la lista de hilos
    for (int j = 0; j < list_size(proceso->hilos); j++){

        //Sacamos la pagina de la lista
        t_hilo *hilo = list_get(proceso->hilos, j);

        list_destroy_and_destroy_elements(hilo->lista_de_instrucciones, free);
        
    }


    log_trace(logger_memoria, "Log Obligatorio: \n");
    log_info(logger_memoria, "Destruccion de particion: \n");
    log_info(logger_memoria, "PID: %d - Tamaño: %d", proceso_pid, list_size(tabla_de_paginas->lista_de_paginas));


    //
    
    list_remove_element(lista_miniPCBs, proceso);
}

*/