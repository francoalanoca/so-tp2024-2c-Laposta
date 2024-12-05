#include "../include/memoria_usuario.h"

//-------------------------Definicion de variables globales----------------------
/*
void* memoria_usuario;                          //espacio de usuario
t_list* lista_particiones;              //lista de las particiones
t_list* lista_particiones_dinamicas; //variable que guarda la lista de particiones
t_list* lista_miniPCBs;                 //lista de los procesos
uint32_t cantidad_particiones_memoria;  //seria tam_memoria / tam_pagina
t_bitarray *bitmap_particiones;         //bitmap para controlar los bloques libres y ocupados
t_list* pids_por_bloque;

uint32_t tamanio_total_memoria;
char * algoritmo_alocacion;
*/

//-------------------------Definicion de funciones----------------------------
/*
//Inicializa memoria con particiones fijas
void inicializar_memoria_particiones_fijas(uint32_t mem_size, uint32_t num_particiones, char* algoritmo) {
    tamanio_total_memoria = mem_size;
    memoria_usuario = malloc(tamanio_total_memoria);  // Espacio de memoria contiguo
    algoritmo_alocacion = malloc(strlen(algoritmo)*sizeof(char) + 1);
    strcpy(algoritmo_alocacion,algoritmo);
    cantidad_particiones_memoria = num_particiones;
    //bitmap_particiones = malloc(sizeof(t_bitarray));
    lista_miniPCBs = list_create();
    bitmap_particiones = crear_bitmap(cantidad_particiones_memoria);
    pids_por_bloque = list_create();

    
    
    //PRUEBA
  //  printf("bitmap:\n");
    //printf("bitmap prueba %d:\n",bitarray_test_bit(bitmap_particiones, 1) ? 1 : 0);
    //bitarray_set_bit(bitmap_particiones, 1);
    //printf("bitmap:%d\n",bitarray_get_max_bit(bitmap_particiones));
//printf("El valor del bit en la posicion %d es: %d\n", 1, bitarray_test_bit(bitmap_particiones, 2) ? 1 : 0);

}
*/

// Funcion para Asignar Memoria
/*void* alocar_memoria(uint32_t size) {
    t_particion* best_partition = NULL;
    t_particion* particion_actual = lista_particiones;

    while (particion_actual != NULL) {
        if (particion_actual->is_free && particion_actual->size >= size) {
            if (strcmp(algoritmo_alocacion, "FIRST") == 0) {
                particion_actual->is_free = false;
                return (void*)((char*)memoria_usuario + particion_actual->start);
            } else if (strcmp(algoritmo_alocacion, "BEST") == 0) {
                if (best_partition == NULL || particion_actual->size < best_partition->size) {
                    best_partition = particion_actual;
                }
            } else if (strcmp(algoritmo_alocacion, "WORST") == 0) {
                if (best_partition == NULL || particion_actual->size > best_partition->size) {
                    best_partition = particion_actual;
                }
            }
        }
        particion_actual = particion_actual->next;
    }

    if (best_partition != NULL) {
        best_partition->is_free = false;
        return (void*)((char*)memoria_usuario + best_partition->start);
    }

    return NULL; // No se encontró un hueco adecuado
}*/



void inicializar_proceso(uint32_t pid, uint32_t tamanio){
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
    //list_add(nuevo_proceso->hilos,nuevo_hilo);TODO: FIXME: se me creaba el primer sin instrucciones
    

 
    uint32_t indice_bloque_a_liberar = buscar_indice_bloque_por_pid(pids_por_bloque,pid);

    t_pid_por_bloque* bloque_x_pid = list_get(pids_por_bloque,indice_bloque_a_liberar);

    nuevo_proceso->base = calcular_base_proceso_fijas(bloque_x_pid->bloque, lista_particiones);

    nuevo_proceso->limite = nuevo_proceso->base +tamanio;

    //list_add(nuevo_proceso->hilos,nuevo_hilo);
    pthread_mutex_lock(&mutex_lista_miniPCBs);
    list_add(lista_miniPCBs,nuevo_proceso);
    pthread_mutex_unlock(&mutex_lista_miniPCBs);
}


//Crear un Proceso
int crear_proceso_fijas(uint32_t tam_proceso, t_list* lista_de_particiones, uint32_t pid) {
    printf("Entro crear proceso\n");
    printf("tam_proceso:%d,pid:%d,algoritmo_alocacion:%s\n",tam_proceso,pid,algoritmo_alocacion);
    //encontrar hueco libre y marcar bitmap, si no encuentra tira error
    uint32_t tamanio_bloque_actual = 0;
    bool bloque_libre_encontrado = false;

    if(tam_proceso == 0){
        return -1;
    }
    else{
        if (strcmp(algoritmo_alocacion, "FIRST") == 0) {

            for (int i = 0; i < list_size(lista_de_particiones); i++) {

                // Obtiene el puntero a char* desde la lista
                char* tamanio_bloque_str = (char*)list_get(lista_de_particiones, i);

                // Verifica que el puntero no sea nulo
                if (tamanio_bloque_str == NULL) {
                    printf("Error: puntero a tamaño de bloque es NULL para índice %d\n", i);
                    continue; // Salta al siguiente elemento de la lista
                }

                // Convierte el char* a uint32_t
                tamanio_bloque_actual = (uint32_t)atoi(tamanio_bloque_str);

                if (tam_proceso <= tamanio_bloque_actual && !bitarray_test_bit(bitmap_particiones,i)) {
                    bloque_libre_encontrado = true;

                    pthread_mutex_lock(&mutex_pids_por_bloque);
                    bitarray_set_bit(bitmap_particiones, i);
                    pthread_mutex_unlock(&mutex_pids_por_bloque);
                    t_pid_por_bloque* pid_por_bloque = malloc(sizeof(t_pid_por_bloque));
                    pid_por_bloque->pid = pid;
                    pid_por_bloque->bloque = i;
                    //PENDIENTE:Verificar que el pid no este ya en memoria
                    pthread_mutex_lock(&mutex_pids_por_bloque);
                    list_add(pids_por_bloque,pid_por_bloque);
                    pthread_mutex_unlock(&mutex_pids_por_bloque);
                    inicializar_proceso(pid, tamanio_bloque_actual); //VER UBICACION
                    return INICIAR_PROCESO_RTA_OK;
                }
            }

            if (!bloque_libre_encontrado) {
                printf("No encuentro bloque\n");
                return -1;
            }
        }
        else if(strcmp(algoritmo_alocacion, "BEST") == 0){
            uint32_t ultimo_bloque_best_fit = -1;
            uint32_t tamanio_ultimo_bloque_best_fit = 0;

            for (int i = 0; i < list_size(lista_de_particiones); i++){
                // Obtiene el puntero a char* desde la lista
                char* tamanio_bloque_str = (char*)list_get(lista_de_particiones, i);

                // Verifica que el puntero no sea nulo
                if (tamanio_bloque_str == NULL) {
                    printf("Error: puntero a tamaño de bloque es NULL para índice %d\n", i);
                    continue; // Salta al siguiente elemento de la lista
                }

                // Convierte el char* a uint32_t
                tamanio_bloque_actual = (uint32_t)atoi(tamanio_bloque_str);
                printf("El tamanio del bloque %d es: %d\n", i, tamanio_bloque_actual);
                if(tam_proceso<=tamanio_bloque_actual && !bitarray_test_bit(bitmap_particiones,i)){ //El proceso entra en el bloque actual
                    if(tamanio_ultimo_bloque_best_fit == 0 || (tamanio_bloque_actual<tamanio_ultimo_bloque_best_fit)){
                        tamanio_ultimo_bloque_best_fit = tamanio_bloque_actual;
                        ultimo_bloque_best_fit = i;
                        printf("El ultimo_bloque_best_fit es: %d\n", ultimo_bloque_best_fit);
                        bloque_libre_encontrado = true;
                    }
                }
            }

            if(!bloque_libre_encontrado){
                return -1;
            }
            else {
            
                pthread_mutex_lock(&mutex_bitmap_particiones);
                bitarray_set_bit(bitmap_particiones, ultimo_bloque_best_fit);
                pthread_mutex_unlock(&mutex_bitmap_particiones);
                 t_pid_por_bloque* pid_por_bloque = malloc(sizeof(t_pid_por_bloque));
                    pid_por_bloque->pid = pid;
                    pid_por_bloque->bloque = ultimo_bloque_best_fit;
                    //PENDIENTE:Verificar que el pid no este ya en memoria
                    pthread_mutex_lock(&mutex_pids_por_bloque);
                    list_add(pids_por_bloque,pid_por_bloque);
                    pthread_mutex_unlock(&mutex_pids_por_bloque);
                    print_lista_pid_por_bloque(pids_por_bloque);
                    inicializar_proceso(pid, tamanio_ultimo_bloque_best_fit); //VER UBICACION
                     return INICIAR_PROCESO_RTA_OK; 
            }
        }
        else if(strcmp(algoritmo_alocacion, "WORST") == 0){
            uint32_t ultimo_bloque_worst_fit = -1;
            uint32_t tamanio_ultimo_bloque_worst_fit = 0;

            for (int i = 0; i < list_size(lista_de_particiones); i++){
                // Obtiene el puntero a char* desde la lista
                char* tamanio_bloque_str = (char*)list_get(lista_de_particiones, i);

                // Verifica que el puntero no sea nulo
                if (tamanio_bloque_str == NULL) {
                    printf("Error: puntero a tamaño de bloque es NULL para índice %d\n", i);
                    continue; // Salta al siguiente elemento de la lista
                   

                }

                // Convierte el char* a uint32_t
                tamanio_bloque_actual = (uint32_t)atoi(tamanio_bloque_str);
        
                if(tam_proceso<=tamanio_bloque_actual && !bitarray_test_bit(bitmap_particiones,i)){ //El proceso entra en el bloque actual
                    if(tamanio_ultimo_bloque_worst_fit == 0 || (tamanio_bloque_actual>tamanio_ultimo_bloque_worst_fit)){
                        tamanio_ultimo_bloque_worst_fit = tamanio_bloque_actual;
                        ultimo_bloque_worst_fit = i;
                        bloque_libre_encontrado = true;
                    }
               
                }
            }

            if(!bloque_libre_encontrado){
                printf("saliro por !bloque_libre_encontrado");
                return -1;
            }
            else {
                printf("Elijo bloque %d\n", ultimo_bloque_worst_fit);
                pthread_mutex_lock(&mutex_bitmap_particiones);
                bitarray_set_bit(bitmap_particiones, ultimo_bloque_worst_fit);
                pthread_mutex_unlock(&mutex_bitmap_particiones);
                t_pid_por_bloque* pid_por_bloque = malloc(sizeof(t_pid_por_bloque));
                    pid_por_bloque->pid = pid;
                    pid_por_bloque->bloque = ultimo_bloque_worst_fit;
                    //PENDIENTE:Verificar que el pid no este ya en memoria
                    pthread_mutex_lock(&mutex_pids_por_bloque);
                    list_add(pids_por_bloque,pid_por_bloque);
                    pthread_mutex_unlock(&mutex_pids_por_bloque);
                    inicializar_proceso(pid, tamanio_ultimo_bloque_worst_fit); //VER UBICACION
                
                return INICIAR_PROCESO_RTA_OK;
            }
        }
        else{
            printf("Error: algoritmo incorrecto\n");
            return -1;
        }
            printf("saliro por return 0");
        return 0;
    }
}

void finalizar_proceso_fijas(uint32_t pid){
    uint32_t indice_bloque_a_liberar = buscar_indice_bloque_por_pid(pids_por_bloque,pid);

    t_pid_por_bloque* bloque_x_pid = list_get(pids_por_bloque,indice_bloque_a_liberar);
    printf("bloque_x_pid: %d\n",bloque_x_pid->bloque);
    pthread_mutex_lock(&mutex_bitmap_particiones);
    bitarray_clean_bit(bitmap_particiones,bloque_x_pid->bloque);
    pthread_mutex_unlock(&mutex_bitmap_particiones);
    pthread_mutex_lock(&mutex_pids_por_bloque);
    list_remove_and_destroy_element(pids_por_bloque,indice_bloque_a_liberar,free);
    pthread_mutex_unlock(&mutex_pids_por_bloque);

   eliminar_proceso_de_lista(pid);
    
}

//Finalizar proceso
/*void finalizar_proceso(void* direccion_proceso) {
    t_particion* particion_actual = lista_particiones;//lista de particiones de config

    // Recorrer la lista de particiones para encontrar la que corresponde al proceso
    while (particion_actual != NULL) {
        // Calcular la dirección de inicio de la partición en el bloque de memoria
        void* direccion_particion = (void*)((char*)memoria_usuario + particion_actual->start);

        // Comparar la dirección de la partición con la dirección de inicio del proceso
        if (direccion_particion == direccion_proceso) {
            if (!particion_actual->is_free) {  // Verificar si la partición está ocupada
                particion_actual->is_free = true;  // Marcar la partición como libre
                printf("Proceso eliminado y partición liberada en la dirección: %p\n", direccion_proceso);
            } else {
                printf("Error: la partición ya estaba libre.\n");
            }
            return; // Salir después de liberar la partición
        }
        particion_actual = particion_actual->next;
    }

    // Si no se encuentra la partición correspondiente, mostrar un mensaje de error
    printf("Error: no se encontró la partición correspondiente al proceso.\n");
}*/

// Funcion para leer 4 bytes desde una direccion física en memoria de usuario
/*uint32_t read_mem(uint32_t direccion_fisica) {
    // Verificar si la dirección está dentro del rango de memoria
    if (direccion_fisica + 4 > tamanio_total_memoria) {
        printf("Error: Dirección fuera del rango de memoria.\n");
        return 0;
    }

    uint32_t value;
    memcpy(&value, (char*)memoria_usuario + direccion_fisica, sizeof(uint32_t));

    printf("Valor leído desde la dirección %u: %u\n", direccion_fisica, value);
    return value;
}*/


/*
// Función para escribir 4 bytes en una dirección física en memoria de usuario
void write_mem(uint32_t direccion_fisica, uint32_t valor) {
    // Verificar si la dirección está dentro del rango de memoria
    if (direccion_fisica + 4 > tamanio_total_memoria) {
        printf("Error: Dirección fuera del rango de memoria.\n");
        return;
    }

    memcpy((char*)memoria_usuario + direccion_fisica, &valor, sizeof(uint32_t));
    printf("Valor %u escrito en la dirección %u.\n", valor, direccion_fisica);
}*/


/*
//Funcion que en base a la cantidad de frames crea bitmap
t_bitarray *crear_bitmap(int entradas){
    int ent = entradas;

    // Redondea a múltiplo de 8
    if (ent % 8 != 0){
        ent = redondear_a_multiplo_mas_cercano_de(8, ent); 
        //log_trace(logger_memoria, "tamanio inusual de memoria/pagina causo conflicto, redondeando al multiplo de 8 mas cercano: %i", ent);
    }

    // Calcula la cantidad de bytes necesarios para almacenar los bits
    int bytes_necesarios = ent / 8;

    // Asigna memoria para el bitarray
    void *puntero = malloc(bytes_necesarios);
    if (puntero == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria para el bitarray.\n");
        exit(EXIT_FAILURE);
    }

    // Crea el bitarray
    t_bitarray *bitmap = bitarray_create_with_mode(puntero, bytes_necesarios, LSB_FIRST);

    // Inicializa todos los bits a 0
    for (int i = 0; i < bitarray_get_max_bit(bitmap); i++) {
        bitarray_clean_bit(bitmap, i);
    }
printf("El valor del bit en la posicion %d es: %d\n", 1, bitarray_test_bit(bitmap, 2) ? 1 : 0);
    return bitmap;
}
*/
void uint32_to_string(uint32_t num, char *str, size_t size) {
    snprintf(str, size, "%u", num);
}

uint32_t buscar_indice_bloque_por_pid(t_list* lista, uint32_t pid) {
    for (int i = 0; i < list_size(lista); i++) {
        t_pid_por_bloque* pid_por_bloque = list_get(lista, i);
        if (pid_por_bloque->pid == pid) {
            return i;
        }
    }
    return -1; 
}



// Función para imprimir un elemento de tipo t_pid_por_bloque
void print_pid_por_bloque(void* element) {
    t_pid_por_bloque* pid_por_bloque = (t_pid_por_bloque*)element;
    printf("PID: %u, Bloque: %u\n", pid_por_bloque->pid, pid_por_bloque->bloque);
}

// Función para imprimir una lista de t_pid_por_bloque
void print_lista_pid_por_bloque(t_list* lista) {
    printf("Contenido de la lista de PID por bloque:\n");

    // Verifica que la lista no sea NULL
    if (lista == NULL) {
        printf("La lista es NULL.\n");
        return;
    }

    // Recorre cada elemento de la lista e imprime su contenido
    for (int i = 0; i < list_size(lista); i++) {
        t_pid_por_bloque* elemento = (t_pid_por_bloque*)list_get(lista, i);
        if (elemento != NULL) {
            print_pid_por_bloque(elemento);
        } else {
            printf("Elemento en la posición %d es NULL.\n", i);
        }
    }
}
//
uint32_t calcular_base_proceso_fijas(uint32_t bloque, t_list* particiones){
    uint32_t base = 0;

    if (bloque < 0 || bloque >= list_size(particiones)) {
        return -1; 
    }

    // Suma los tamaños de las particiones anteriores al índice dado
    for (int i = 0; i < bloque; i++) {
        uint32_t tamanio_actual = atoi(list_get(particiones,i));
        base += tamanio_actual;
    }

    return base;
}
/*
//Funcion que redondea el valor al multiplo cercano de base y retorna
int redondear_a_multiplo_mas_cercano_de(int base, int valor){
    int v = valor == 0 ? 1 : valor;
    return (int) ceil((float) v / (float) base) * base;
}
*/

void inicializar_hilo(uint32_t pid, uint32_t tid, char* nombre_archivo){
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
            log_warning(logger_memoria,"Se agrega tid %d a proceso : %d",hilo->tid,pid);
           // printf("Se agrega tid %d a proceso %d\n",hilo->tid, pid);
            encontrado = true;
        }
    }
    if(!encontrado){
        printf("No se asigno hilo ya que no se encuentra el proceso %d\n", pid);
    }

    
}