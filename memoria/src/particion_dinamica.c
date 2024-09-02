#include "../include/particion_dinamica.h"

/*
void crear_proceso(uint32_t proceso_pid, uint32_t tamanio_proceso){

    if (cfg_memoria->ESQUEMA == "DINAMICAS"){
        crear_proceso_dinamico(uint32_t proceso_pid, uint32_t tamanio_proceso);
    }else{
        if (cfg_memoria == "FIJAS")
            crear_proceso_fijo(uint32_t proceso_pid, uint32_t tamanio_proceso);
        
    }
    
}
*/

/*
//Funcion que crea la tabla de paginas a partir de un pid
void crear_proceso_dinamico(uint32_t proceso_pid, uint32_t tamanio_proceso){

    log_info(logger_memoria, "Creacion del proceso PID %i \n", proceso_pid);
    log_info(logger_memoria, "Iniciando estructuras \n");

    //Guardo en una varia de tipo struct la tabla creada
    t_tabla_de_paginas *tabla_de_paginas = crear_tabla_pagina(proceso_pid);
    log_info(logger_memoria, "Creada tabla de paginas \n");
    list_add(lista_tablas_de_paginas, tabla_de_paginas);
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