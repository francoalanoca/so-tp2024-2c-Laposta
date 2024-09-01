#ifndef ESTRUCTURAS_SINCRONIZADAS_H_
#define ESTRUCTURAS_SINCRONIZADAS_H_
#include <commons/collections/queue.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct
{
    t_queue *cola;
    pthread_mutex_t mutex;
} t_cola_mutex;

/// @brief Crea una cola mutex para poder ser utilizada por las operacion de cola_mutex. Ya que la cola trabaja con punteros genericos, es
/// recomendable que la cola sea para 1 solo tipo de elemento.
/// @return 
t_cola_mutex *cola_mutex_crear();
/// @brief Obtiene el tamanio de cola_mutex
/// @param cola_mutex 
/// @return 
int cola_mutex_tamanio(t_cola_mutex *cola_mutex);
/// @brief Destruye la cola mutex
/// @param cola_mutex 
/// @param element_destroyer lambda que destruye los elementos: la funcion debe recibir como parametro un elemento de tipo void *
/// que deberia castearlo al tipo que corresponda y destruir el elemento dentro de la misma.
void cola_mutex_destruir_y_destruir_elementos(t_cola_mutex *cola_mutex, void (*element_destroyer)(void *));
/// @brief Obtiene la lista definida en la cola
/// @param cola_mutex 
/// @return 
t_list *cola_mutex_lista(t_cola_mutex *cola_mutex);
/// @brief Remueve un elemento de la cola
/// @param cola_mutex 
/// @param elemento 
/// @return 
bool cola_mutex_remover_elemento(t_cola_mutex *cola_mutex, void *elemento);
/// @brief Almacena un elemento al final de la cola de forma segura (thread-safe).
/// @param cola_mutex 
/// @param elem elemento a meter en la cola. Se **recomienda** que el elemento sea del mismo tipo que los demas elementos en la cola.
void cola_mutex_push(t_cola_mutex *cola_mutex, void *elem);
/// @brief Obtiene el primer elemento de la cola de forma segura (thread-safe)
/// @param cola_mutex 
/// @return puntero generico al elemento o NULL en caso de cola vacia.
void *cola_mutex_pop(t_cola_mutex *cola_mutex);
/// @brief Retorna el minimo de la cola según el comparador y lo quita de la misma.
/// @param cola_mutex 
/// @param minimum El comparador recibe dos elementos de la cola y debe retornar el minimo de los dos.
/// @return 
void *cola_mutex_tomar_minimo(t_cola_mutex *cola_mutex, void* (*minimo)(void*, void*));

void cola_mutex_destruir(t_cola_mutex *cola_mutex);
void* cola_mutex_peek(t_cola_mutex* cola_mutex);
#endif