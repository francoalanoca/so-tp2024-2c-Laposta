#include "../include/memoria_usuario.h"

//-------------------------Definicion de variables globales----------------------
void* memoria_usuario;                          //espacio de usuario
t_list* lista_particiones;              //lista de las particiones
t_list* lista_miniPCBs;                 //lista de los procesos
uint32_t cantidad_particiones_memoria;  //seria tam_memoria / tam_pagina
t_bitarray *bitmap_particiones;         //bitmap para controlar los bloques libres y ocupados

uint32_t tamanio_total_memoria;
char * algoritmo_alocacion;


//-------------------------Definicion de funciones----------------------------
//Inicializa memoria con particiones fijas
void inicializar_memoria_particiones_fijas(uint32_t mem_size, uint32_t num_particiones, char* algoritmo) {
    tamanio_total_memoria = mem_size;
    memoria_usuario = malloc(tamanio_total_memoria);  // Espacio de memoria contiguo
    algoritmo_alocacion = malloc(strlen(algoritmo)*sizeof(char));
    strcpy(algoritmo_alocacion,algoritmo);
    bitmap_particiones = crear_bitmap(cantidad_particiones_memoria);

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
void* alocar_memoria(uint32_t size) {
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
}

//Crear un Proceso
uint32_t crear_proceso(uint32_t tam_proceso, t_list* lista_de_particiones) {
    //encontrar hueco libre y marcar bitmap, si no encuentra tira error
    uint32_t tamanio_bloque_actual = 0;
    bool bloque_libre_encontrado = false;

    if(tam_proceso == 0){
        return -1;
    }
    else{
        if(strcmp(algoritmo_alocacion, "FIRST") == 0){
            for (int i = 0; i < list_size(lista_de_particiones); i++){
                tamanio_bloque_actual = (uint32_t) list_get(lista_de_particiones, i);
                if(tam_proceso<tamanio_bloque_actual){
                    bloque_libre_encontrado = true;
                    bitarray_set_bit(bitmap_particiones, i);
                }
            }

            if(!bloque_libre_encontrado){
                return -1;
            }

        }
        else if(strcmp(algoritmo_alocacion, "BEST") == 0){

        }
        else if(strcmp(algoritmo_alocacion, "WORST") == 0){

        }
        else{
            printf("Error: algoritmo incorrecto\n");
        }
        
    }
}

//Finalizar proceso
void finalizar_proceso(void* direccion_proceso) {
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
}

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
