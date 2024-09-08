#include "../include/memoria_usuario.h"

//-------------------------Definicion de variables globales----------------------
void* memoria_usuario;                          //espacio de usuario
t_list* lista_particiones;              //lista de las particiones
t_list* lista_miniPCBs;                 //lista de los procesos
uint32_t cantidad_particiones_memoria;  //seria tam_memoria / tam_pagina
t_bitarray *bitmap_particiones;         //bitmap para controlar los bloques libres y ocupados
//t_dictionary* pids_por_bloque;
t_list* pids_por_bloque;

uint32_t tamanio_total_memoria;
char * algoritmo_alocacion;


//-------------------------Definicion de funciones----------------------------
//Inicializa memoria con particiones fijas
void inicializar_memoria_particiones_fijas(uint32_t mem_size, uint32_t num_particiones, char* algoritmo) {
    tamanio_total_memoria = mem_size;
    memoria_usuario = malloc(tamanio_total_memoria);  // Espacio de memoria contiguo
    algoritmo_alocacion = malloc(strlen(algoritmo)*sizeof(char));
    strcpy(algoritmo_alocacion,algoritmo);
    cantidad_particiones_memoria = num_particiones;
    //bitmap_particiones = malloc(sizeof(t_bitarray));
    bitmap_particiones = crear_bitmap(cantidad_particiones_memoria);
    //pids_por_bloque = dictionary_create();
    pids_por_bloque = list_create();

    //ingresar_valor_diccionario(pids_por_bloque,4,43);
    
    
    //PRUEBA
  //  printf("bitmap:\n");
    //printf("bitmap prueba %d:\n",bitarray_test_bit(bitmap_particiones, 1) ? 1 : 0);
    //bitarray_set_bit(bitmap_particiones, 1);
    //printf("bitmap:%d\n",bitarray_get_max_bit(bitmap_particiones));
//printf("El valor del bit en la posicion %d es: %d\n", 1, bitarray_test_bit(bitmap_particiones, 2) ? 1 : 0);

}

//Inicializa memoria con particiones dinamicas
void inicializar_memoria_particiones_dinamicas(size_t mem_size, char* algoritmo) {
    tamanio_total_memoria = mem_size;
    memoria_usuario = malloc(tamanio_total_memoria);  // Espacio de memoria contiguo
    algoritmo_alocacion = algoritmo;

    // Crear una sola partición que cubre todo el espacio
    t_particion* particion = (t_particion*)malloc(sizeof(t_particion));
    particion->start = 0;
    particion->size = tamanio_total_memoria;
    particion->is_free = true;
    particion->next = NULL;
    lista_particiones = particion;
}

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

//Crear un Proceso
uint32_t crear_proceso(uint32_t tam_proceso, t_list* lista_de_particiones, uint32_t pid) {
    printf("Entro crear proceso\n");
    printf("Tamanio lista:%d\n", list_size(lista_de_particiones));
    //encontrar hueco libre y marcar bitmap, si no encuentra tira error
    uint32_t tamanio_bloque_actual = 0;
    bool bloque_libre_encontrado = false;

    if(tam_proceso == 0){
        return -1;
    }
    else{
        if (strcmp(algoritmo_alocacion, "FIRST") == 0) {
            printf("Entro FIRST\n");

            for (int i = 0; i < list_size(lista_de_particiones); i++) {
                printf("Entro loop %d\n", i);

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

                if (tam_proceso < tamanio_bloque_actual) {
                    bloque_libre_encontrado = true;
                    printf("Elijo bloque %d\n", i);
                    bitarray_set_bit(bitmap_particiones, i);
                    //ingresar_valor_diccionario(pids_por_bloque,i,pid);
                    t_pid_por_bloque* pid_por_bloque = malloc(sizeof(t_pid_por_bloque));
                    pid_por_bloque->pid = pid;
                    pid_por_bloque->bloque = i;
                    //PENDIENTE:Verificar que el pid no este ya en memoria
                    list_add(pids_por_bloque,pid_por_bloque);
                    return 0;
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
                if(tam_proceso<tamanio_bloque_actual){ //El proceso entra en el bloque actual
                    if(tamanio_ultimo_bloque_best_fit == 0 || (tamanio_bloque_actual<tamanio_ultimo_bloque_best_fit)){
                        tamanio_ultimo_bloque_best_fit = tamanio_bloque_actual;
                        ultimo_bloque_best_fit = i;
                        bloque_libre_encontrado = true;
                    }
                }
            }

            if(!bloque_libre_encontrado){
                return -1;
            }
            else{
                printf("Elijo bloque %d\n", ultimo_bloque_best_fit);
                bitarray_set_bit(bitmap_particiones, ultimo_bloque_best_fit);
                //ingresar_valor_diccionario(pids_por_bloque,ultimo_bloque_best_fit,pid);
                //printf("Elemento agregado al diccionario. Nuevo estado del diccionario:\n");
   // dictionary_iterator(pids_por_bloque, print_element);
                 t_pid_por_bloque* pid_por_bloque = malloc(sizeof(t_pid_por_bloque));
                    pid_por_bloque->pid = pid;
                    pid_por_bloque->bloque = ultimo_bloque_best_fit;
                    //PENDIENTE:Verificar que el pid no este ya en memoria
                    list_add(pids_por_bloque,pid_por_bloque);
                    printf("Se agrego a lista. Estado actual:\n");
                    print_lista_pid_por_bloque(pids_por_bloque);

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
                printf("El tamanio del bloque %d es: %d\n", i, tamanio_bloque_actual);
                if(tam_proceso<tamanio_bloque_actual){ //El proceso entra en el bloque actual
                    if(tamanio_ultimo_bloque_worst_fit == 0 || (tamanio_bloque_actual>tamanio_ultimo_bloque_worst_fit)){
                        tamanio_ultimo_bloque_worst_fit = tamanio_bloque_actual;
                        ultimo_bloque_worst_fit = i;
                        bloque_libre_encontrado = true;
                    }
                }
            }

            if(!bloque_libre_encontrado){
                return -1;
            }
            else{
                printf("Elijo bloque %d\n", ultimo_bloque_worst_fit);
                bitarray_set_bit(bitmap_particiones, ultimo_bloque_worst_fit);
                //ingresar_valor_diccionario(pids_por_bloque,ultimo_bloque_worst_fit,pid);
                t_pid_por_bloque* pid_por_bloque = malloc(sizeof(t_pid_por_bloque));
                    pid_por_bloque->pid = pid;
                    pid_por_bloque->bloque = ultimo_bloque_worst_fit;
                    //PENDIENTE:Verificar que el pid no este ya en memoria
                    list_add(pids_por_bloque,pid_por_bloque);
            }
        }
        else{
            printf("Error: algoritmo incorrecto\n");
            return -1;
        }
        return 0;
    }
}

void finalizar_proceso_fijas(uint32_t pid){
    //buscar bloque segun pid del diccionario y eliminar del bitmap
 //printf("Elemento agreagado a diccionario, nuevo estado del diccionario:\n");
   // dictionary_iterator(pids_por_bloque, print_element);

    /*char key_str[sizeof(uint32_t)];
    memcpy(key_str, &pid, sizeof(uint32_t));
        // Imprimir cada byte de key_str en formato hexadecimal
    printf("key_str (en hexadecimal): ");
    for (int i = 0; i < sizeof(uint32_t); i++) {
        printf("%02x ", (unsigned char)key_str[i]);
    }
    printf("\n");*/
    uint32_t indice_bloque_a_liberar = buscar_indice_bloque_por_pid(pids_por_bloque,pid);

    printf("El indice en la lista del bloque a liberar es: %d\n",indice_bloque_a_liberar);

    t_pid_por_bloque* bloque_x_pid = list_get(pids_por_bloque,indice_bloque_a_liberar);
    printf("bloque_x_pid: %d\n",bloque_x_pid->bloque);
    //void *bloque_a_liberar = dictionary_get(pids_por_bloque, key_str);
    //uint32_t valor_bloque_a_liberar;
    /*if (bloque_a_liberar != NULL) {
        valor_bloque_a_liberar = *(uint32_t *)bloque_a_liberar; // Convertir y acceder al valor
        printf("Valor recuperado: %u\n", valor_bloque_a_liberar);
    } else {
        printf("No se encontró un elemento con la clave %u.\n", pid);
    }*/
    bitarray_clean_bit(bitmap_particiones,bloque_x_pid->bloque);
   // eliminar_valor_diccionario(pids_por_bloque,pid);
   list_remove_and_destroy_element(pids_por_bloque,indice_bloque_a_liberar,free);
    
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
uint32_t read_mem(uint32_t direccion_fisica) {
    // Verificar si la dirección está dentro del rango de memoria
    if (direccion_fisica + 4 > tamanio_total_memoria) {
        printf("Error: Dirección fuera del rango de memoria.\n");
        return 0;
    }

    uint32_t value;
    memcpy(&value, (char*)memoria_usuario + direccion_fisica, sizeof(uint32_t));

    printf("Valor leído desde la dirección %u: %u\n", direccion_fisica, value);
    return value;
}

// Función para escribir 4 bytes en una dirección física en memoria de usuario
void write_mem(uint32_t direccion_fisica, uint32_t valor) {
    // Verificar si la dirección está dentro del rango de memoria
    if (direccion_fisica + 4 > tamanio_total_memoria) {
        printf("Error: Dirección fuera del rango de memoria.\n");
        return;
    }

    memcpy((char*)memoria_usuario + direccion_fisica, &valor, sizeof(uint32_t));
    printf("Valor %u escrito en la dirección %u.\n", valor, direccion_fisica);
}


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

void uint32_to_string(uint32_t num, char *str, size_t size) {
    snprintf(str, size, "%u", num);
}

//Imprime valor de diccionario
void print_element(char* key, void* value) {
    printf("Clave: %s, Valor: %d\n", key, *(int*)value);
}

void verificar_diccionario(t_dictionary* diccionario, const char* mensaje) {
    printf("%s\n", mensaje);
    dictionary_iterator(diccionario, print_element);
}

/*void ingresar_valor_diccionario(t_dictionary* diccionario, uint32_t clave, uint32_t valor) {
    char clave_string[12];
    uint32_to_string(clave, clave_string, sizeof(clave_string));

    // Imprimir el estado antes de la modificación
    printf("Antes de agregar: Clave: %s, Valor: %d\n", clave_string, valor);
    printf("Estado del diccionario antes de la inserción:\n");
    dictionary_iterator(pids_por_bloque, print_element);

    // Agregar o actualizar el valor en el diccionario
    dictionary_put(pids_por_bloque, clave_string, &valor);

    // Imprimir el estado después de la modificación
    printf("Elemento agregado al diccionario. Nuevo estado del diccionario:\n");
    dictionary_iterator(pids_por_bloque, print_element);
}


void eliminar_valor_diccionario(t_dictionary* diccionario, uint32_t clave){
    char clave_string[12];
    uint32_to_string(clave,clave_string,sizeof(clave_string));
    //dictionary_remove(pids_por_bloque,clave_string);
    dictionary_remove_and_destroy(pids_por_bloque,clave_string,free);
    printf("Elemento eliminado de diccionario, nuevo estado del diccionario:\n");
    dictionary_iterator(pids_por_bloque, print_element);
}*/

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
