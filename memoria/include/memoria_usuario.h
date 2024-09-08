#ifndef MEMORIA_USUARIO_H
#define MEMORIA_USUARIO_H

#include <pthread.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <commons/collections/dictionary.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <utils/utils.h>


//----------------------------------Estructuras---------------------------------

// Estructura para las particiones dinamicas

typedef struct{
    uint32_t pid;                   // Id del proceso al que pertenece la particion
    uint32_t tid;                   // Id del hilo al que pertenece la particion
    uint32_t inicio;                // Posición de inicio en el espacio de memoria
    uint32_t tamanio;               // Tamaño de la partición
    bool ocupado;                   // Estado de la partición: libre u ocupada
    struct t_particion* siguiente;  // Puntero a la siguiente partición (lista enlazada)
} t_particion_dinamica;


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

uint32_t crear_proceso(uint32_t tam_proceso, t_list* lista_de_particiones, uint32_t pid);

t_bitarray *crear_bitmap(int entradas);

void finalizar_proceso(void* direccion_proceso);

uint32_t read_mem(uint32_t direccion_fisica);

void write_mem(uint32_t direccion_fisica, uint32_t valor);

void uint32_to_string(uint32_t num, char *str, size_t size);

void print_element(char *key, void *value);

void ingresar_valor_diccionario(t_dictionary* diccionario, uint32_t clave, uint32_t valor);

void eliminar_valor_diccionario(t_dictionary* diccionario, uint32_t clave);

#endif 