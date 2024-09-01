#ifndef MEMORIA_USUARIO_H
#define MEMORIA_USUARIO_H

#include <pthread.h>
#include <commons/string.h>
#include <commons/bitarray.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <utils/utils.h>


//----------------------------------Estructuras---------------------------------

// Estructura para las particiones
typedef struct{
    uint32_t start;            // Posición de inicio en el espacio de memoria
    uint32_t size;             // Tamaño de la partición
    bool is_free;            // Estado de la partición: libre u ocupada
    struct t_particion* next;  // Puntero a la siguiente partición (lista enlazada)
} t_particion;


//----------------------------------Variables Externs-------------------------
extern void* memoria_usuario;                        
extern t_list* lista_particiones;             
extern t_list* lista_miniPCBs;  
extern uint32_t tamanio_total_memoria;  
extern char * algoritmo_alocacion;          


//----------------------------------Prototipos---------------------------------
void inicializar_memoria_particiones_fijas(uint32_t mem_size, uint32_t num_particiones, char* algoritmo);

void inicializar_memoria_particiones_dinamicas(size_t mem_size, char* algoritmo);

void* alocar_memoria(uint32_t size);

uint32_t crear_proceso(uint32_t tam_proceso, t_list* lista_de_particiones);

void finalizar_proceso(void* direccion_proceso);

uint32_t read_mem(uint32_t direccion_fisica);

void write_mem(uint32_t direccion_fisica, uint32_t valor);

#endif 