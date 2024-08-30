#ifndef UTILS_MEMORIA_H
#define UTILS_MEMORIA_H



#include "../include/init_memoria.h"

//----------------------------------Estructuras---------------------------------

typedef struct{
    uint32_t pid;                     //pcb del proceso
    char *archivo_pseudocodigo;     //nombre del proceso
} t_m_crear_proceso;



typedef struct{

    /* data */

}t_m_crear_hilo;




typedef struct{

    /* data */

} t_m_contexto;


typedef struct{
    uint32_t pid;
    uint32_t program_counter;
}t_proceso_memoria;


typedef struct{
    uint32_t pid;
    uint32_t direccion_fisica;
    uint32_t tamanio;
    char* valor;
} t_escribir_leer;


typedef struct{

    /* data */

} t_dump;


//----------------------------------Variables Externs-------------------------



//----------------------------------Prototipos---------------------------------

t_m_crear_proceso* deserializar_iniciar_proceso(t_list*  lista_paquete );
void enviar_respuesta_iniciar_proceso(t_m_crear_proceso* crear_proceso ,int socket_kernel);
uint32_t deserializar_finalizar_proceso(valores);
void enviar_respuesta_finalizar_proceso(pid_proceso_a_finalizar, socket_kernel);


t_proceso_memoria* deserializar_solicitud_instruccion(valores);
void enviar_respuesta_instruccion(instruccion, socket_cliente);
t_escribir_leer* deserializar_read_memoria(t_list*  lista_paquete );
void enviar_respuesta_read_memoria(void* respuesta_leer, int socket_cpu);
t_escribir_leer* deserializar_write_memoria(t_list*  lista_paquete);
void enviar_respuesta_write_memoria(char* respuesta_escribir, int socket_cliente);


#endif /* MEMORIA_H */