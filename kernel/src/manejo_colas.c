#include "../include/manejo_colas.h"

t_cola_mutex *cola_mutex_crear()
{
    t_cola_mutex *cola_mutex = malloc(sizeof(t_cola_mutex));

    cola_mutex->cola = queue_create();
    pthread_mutex_init(&(cola_mutex->mutex), NULL);
    return cola_mutex;
}

int cola_mutex_tamanio(t_cola_mutex *cola_mutex)
{
    pthread_mutex_lock(&(cola_mutex->mutex));
    int tamanio = list_size(cola_mutex->cola->elements);
    pthread_mutex_unlock(&(cola_mutex->mutex));
    return tamanio;
}

void cola_mutex_destruir(t_cola_mutex *cola_mutex)
{
    pthread_mutex_destroy(&(cola_mutex->mutex)); // libero mutex
    queue_destroy(cola_mutex->cola); // libero cola y 
    free(cola_mutex);
}

void cola_mutex_destruir_y_destruir_elementos(t_cola_mutex *cola_mutex, void (*element_destroyer)(void *))
{
    pthread_mutex_destroy(&(cola_mutex->mutex)); // libero mutex
    queue_destroy_and_destroy_elements(cola_mutex->cola, element_destroyer); // libero cola y 
    free(cola_mutex);
}

t_list *cola_mutex_lista(t_cola_mutex *cola_mutex)
{
    return cola_mutex->cola->elements;
}

bool cola_mutex_remover_elemento(t_cola_mutex *cola_mutex, void *elemento)
{
    pthread_mutex_lock(&(cola_mutex->mutex));
    bool exito = list_remove_element(cola_mutex->cola->elements, elemento);
    pthread_mutex_unlock(&(cola_mutex->mutex));
    return exito;
}

void cola_mutex_push(t_cola_mutex *cola_mutex, void *elem)
{
    pthread_mutex_lock(&(cola_mutex->mutex));
    queue_push(cola_mutex->cola, elem);
    pthread_mutex_unlock(&(cola_mutex->mutex));
}

void *cola_mutex_pop(t_cola_mutex *cola_mutex)
{
    pthread_mutex_lock(&(cola_mutex->mutex));
    if (!queue_is_empty(cola_mutex->cola)) {
        void *info = queue_pop(cola_mutex->cola);
        pthread_mutex_unlock(&(cola_mutex->mutex));
        return info;
    }
    pthread_mutex_unlock(&(cola_mutex->mutex));
    return NULL;
}

void *cola_mutex_tomar_minimo(t_cola_mutex *cola_mutex, void* (*minimo)(void*, void*)) {
    pthread_mutex_lock(&(cola_mutex->mutex));
    void* elem = list_get_minimum(cola_mutex->cola->elements, minimo);
    list_remove_element(cola_mutex->cola->elements, elem);
    pthread_mutex_unlock(&(cola_mutex->mutex));
    return elem;
}
void* cola_mutex_peek(t_cola_mutex* cola_mutex){
 pthread_mutex_lock(&(cola_mutex->mutex));
    if (!queue_is_empty(cola_mutex->cola)) {
        void *info = queue_peek(cola_mutex->cola);
        pthread_mutex_unlock(&(cola_mutex->mutex));
        return info;
    }
    pthread_mutex_unlock(&(cola_mutex->mutex));
    return NULL;
}

// ---------------------------------------------------