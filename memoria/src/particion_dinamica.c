#include "../include/particion_dinamica.h"

/*
void crear_proceso(uint32_t proceso_pid, uint32_t tamanio_proceso){

    if (strcmp(cfg_memoria->ESQUEMA, "DINAMICAS") == 0){
        crear_proceso_dinamico(proceso_pid, tamanio_proceso);
    }else{
        if (strcmp(cfg_memoria->ESQUEMA, "FIJAS") == 0)
            crear_proceso_fijo(proceso_pid, tamanio_proceso);
    }
    
}
*/

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


    // Selecciona la partición según el algoritmo
    if (strcmp(cfg_memoria->ALGORITMO_BUSQUEDA, "FIRST_FIT") == 0) {
        particion_resultante = buscar_first_fit(tamanio_proceso);

    } else if (strcmp(cfg_memoria->ALGORITMO_BUSQUEDA, "BEST_FIT") == 0) {
        particion_resultante = buscar_best_fit(tamanio_proceso);

    } else if (strcmp(cfg_memoria->ALGORITMO_BUSQUEDA, "WORST_FIT") == 0) {
        particion_resultante = buscar_worst_fit(tamanio_proceso);
    }


    // Si es null no hay memoria
    if (particion_resultante == NULL) {
        return NULL; 
    }

    // Divido la partición si es más grande que el tamaño del proceso
    particion_resultante = dividir_particion(particion_resultante, tamanio_proceso);

    particion_resultante->pid = proceso_pid;
    particion_resultante->ocupado = true;

    return particion_resultante;
}



//Funcion que busca la primer particion
t_particion_dinamica *buscar_first_fit(uint32_t tamanio_proceso){

    t_particion_dinamica *particion = lista_particiones_dinamicas;

    //Recorremos la lista enlazada de particiones
    while (particion != NULL){

        //Verificamos que la particion obtenida este libre y sea >= al proceso
        if (!particion->ocupado && particion->tamanio >= tamanio_proceso) {
            return particion;
        }

        //Actualizo la particion para poder avanzar
        particion = particion->siguiente;
    }
    return NULL;
}


//Funcion que busca la mejor particion
t_particion_dinamica *buscar_best_fit(uint32_t tamanio_proceso){

    t_particion_dinamica *particion = lista_particiones_dinamicas;

    //iniciamos la variable mejor en Null
    t_particion_dinamica *mejor_particion = NULL;

    //Recorremos la lista de particiones comparando en cada iteracion
    while (particion != NULL){

        //Verificamos que la particion obtenida este libre y sea >= al proceso
        if (!particion->ocupado && particion->tamanio >= tamanio_proceso) {

            //Verificamos si todavia no hay una mejor o si el tamaño particion actual es menor a la mejor actual 
            if (mejor_particion == NULL || particion->tamanio < mejor_particion->tamanio) {
                mejor_particion = particion;
            }
        }

        //Actualizo la particion para poder avanzar
        particion = particion->siguiente;
    }

    //Retornamos la mejor o si no hay Null
    return mejor_particion;
}


//Funcion que busca la peor particion
t_particion_dinamica *buscar_worst_fit(uint32_t tamanio_proceso){

    t_particion_dinamica *particion = lista_particiones_dinamicas;

    //iniciamos la variable mejor en Null
    t_particion_dinamica *peor_particion = NULL;

    //Recorremos la lista de particiones comparando en cada iteracion
    while (particion != NULL){
        
        //Verificamos que la particion obtenida este libre y sea >= al proceso
        if (!particion->ocupado && particion->tamanio >= tamanio_proceso){

            //Verificamos si todavia no hay una peor o si el tamaño particion actual es mayor a la peor actual 
            if (peor_particion == NULL || particion->tamanio > peor_particion->tamanio) {
                peor_particion = particion;
            }
        }

        //Actualizo la particion para poder avanzar
        particion = particion->siguiente;
    }

    //Retornamos la peor o si no hay Null
    return peor_particion;
}




//Funcion que va diviendo el espacio deacuerdo al tamaño del proceso
t_particion_dinamica *dividir_particion(t_particion_dinamica* particion, uint32_t tamanio_proceso) {
    
    //Si la partición tiene exactamente el tamaño necesario, no la dividimos
    if (particion->tamanio == tamanio_proceso) {
        return particion;
    }

    //Creamos una nueva partición para la parte libre
    t_particion_dinamica* nueva_particion = malloc(sizeof(t_particion_dinamica));
    nueva_particion->inicio = particion->inicio + tamanio_proceso;
    nueva_particion->tamanio = particion->tamanio - tamanio_proceso;
    nueva_particion->ocupado = false;
    nueva_particion->pid = 0;                                           //NO tiene proceso asignado
    nueva_particion->siguiente = particion->siguiente;

    // La partición original ahora se reduce al tamaño del proceso
    particion->tamanio = tamanio_proceso;
    particion->siguiente = nueva_particion;

    return particion;
}








//Funciones para la finalizacion de un proceso

t_particion_dinamica *busco_particion_dinamica_por_PID(uint32_t proceso_pid){

    log_trace(logger_memoria, "Buscando la particion por PID");

    t_particion_dinamica *particion = lista_particiones_dinamicas;

    //Recorremos la lista enlazada que contiene particiones
    while (particion != NULL){
    
        //Si el id de la particion es el mismo, la retorna
        if (proceso_pid == particion->pid){
            return particion;
        }

        //Actualizo la particion para poder avanzar
        particion = particion->siguiente;
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
        if (hilo_pid == proceso->pid)
            return proceso;
    }

    log_error(logger_memoria, "PID - %d No se encontro el proceso", hilo_pid);
    abort();
}




// Función para unificar particiones libres adyacentes
void unificar_particiones_dinamicas(t_particion_dinamica *particion){

    t_particion_dinamica *particion_actual = particion;
    t_particion_dinamica *particion_anterior = NULL;
    t_particion_dinamica *actual = lista_particiones_dinamicas;


    // 1. Unificación hacia adelante (con la siguiente partición)
    if (particion_actual->siguiente != NULL && !particion_actual->siguiente->ocupado) {
        log_trace(logger_memoria, "Unificando con la particion siguiente libre.");

        t_particion_dinamica *particion_siguiente = particion_actual->siguiente;

        // Aumentamos el tamaño de la partición actual
        particion_actual->tamanio += particion_siguiente->tamanio;

        // Ajustamos el puntero al siguiente de la partición unificada
        particion_actual->siguiente = particion_siguiente->siguiente;

        // Liberamos la partición siguiente que ha sido unificada
        free(particion_siguiente);
    }


    // 2. Unificación hacia atrás (con la partición anterior)
    // Buscamos la partición anterior en la lista
    while (actual != NULL && actual != particion_actual) {
        particion_anterior = actual;
        actual = actual->siguiente;
    }


    // Si existe una partición anterior y está libre
    if (particion_anterior != NULL && !particion_anterior->ocupado) {
        log_trace(logger_memoria, "Unificando con la particion anterior libre.");

        // Aumentamos el tamaño de la partición anterior
        particion_anterior->tamanio += particion_actual->tamanio;

        // Ajustamos el puntero al siguiente de la partición anterior
        particion_anterior->siguiente = particion_actual->siguiente;

        // Liberamos la partición actual que ha sido unificada
        free(particion_actual);
    }
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
        free(hilo);
        
    }


    log_trace(logger_memoria, "Log Obligatorio: \n");
    log_info(logger_memoria, "Destruccion de particion: \n");
    log_info(logger_memoria, "PID: %d - Tamaño: %d", proceso_pid, list_size(tabla_de_paginas->lista_de_paginas));

    particion->ocupado = false;

    unificar_particiones_dinamicas(particion);
    
    list_remove_element(lista_miniPCBs, proceso);
}

*/