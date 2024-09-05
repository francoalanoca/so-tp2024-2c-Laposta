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

    t_particion* particion_proceso = malloc(sizeof(t_particion));
    particion_proceso = asignar_memoria(tamanio_proceso);

    
    log_info(logger_memoria, "Creada particion \n");
    list_add(lista_particiones, particion_proceso);
}




t_particion *asignar_memoria(uint32_t tamanio_proceso){

    t_particion *particion_resultante = NULL;

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

    // Marcar la partición como ocupada
    particion_resultante->ocupado = true;

    return particion_resultante;
}



//Funcion que busca la primer particion
t_particion *buscar_first_fit(uint32_t tamanio_proceso){

    //Recorremos la lista de particiones comparando en cada iteracion
    for (int i = 0; i < list_size(lista_particiones); i++) {
        t_particion *particion = list_get(lista_particiones, i);

        //Verificamos que la particion obtenida este libre y sea >= al proceso
        if (!particion->ocupado && particion->tamanio >= tamanio_proceso) {
            return particion;
        }
    }
    return NULL;
}


//Funcion que busca la mejor particion
t_particion *buscar_best_fit(uint32_t tamanio_proceso){

    //iniciamos la variable mejor en Null
    t_particion *mejor_particion = NULL;

    //Recorremos la lista de particiones comparando en cada iteracion
    for (int i = 0; i < list_size(lista_particiones); i++) {
        t_particion *particion = list_get(lista_particiones, i);

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
t_particion *buscar_worst_fit(uint32_t tamanio_proceso){

    //iniciamos la variable mejor en Null
    t_particion *peor_particion = NULL;

    //Recorremos la lista de particiones comparando en cada iteracion
    for (int i = 0; i < list_size(lista_particiones); i++){
        t_particion *particion = list_get(lista_particiones, i);

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







//Funcion que en base al id de un proceso finalizamos sus estructuras
void finalizar_proceso(uint32_t proceso_pid){

    log_trace(logger_memoria, "Liberacion del proceso PID %i", proceso_pid);
    t_tabla_de_paginas *tabla_de_paginas = busco_tabla_de_paginas_por_PID(proceso_pid);
    t_miniPCB *proceso = busco_un_proceso_PID(proceso_pid);

    //Recorremos la lista de paginas
    for (int j = 0; j < list_size(tabla_de_paginas->lista_de_paginas); j++){

        //Sacamos la pagina de la lista
        t_pagina *pagina = list_get(tabla_de_paginas->lista_de_paginas, j);

        //Marco el bit como disponible
        bitarray_clean_bit(bitmap_frames, pagina->marco);
        
    }
    log_trace(logger_memoria, "Log Obligatorio: \n");
    log_info(logger_memoria, "Destruccion de tabla de paginas: \n");
    log_info(logger_memoria, "PID: %d - Tamaño: %d", proceso_pid, list_size(tabla_de_paginas->lista_de_paginas));

    list_destroy_and_destroy_elements(tabla_de_paginas->lista_de_paginas, free);
    list_destroy_and_destroy_elements(proceso->lista_de_instrucciones, free);
    free(tabla_de_paginas);
    list_remove_element(lista_miniPCBs, proceso);
}

*/