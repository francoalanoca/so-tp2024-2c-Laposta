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


typedef struct {
    uint32_t PC;
    uint8_t AX, BX, CX, DX, EX, FX, GX, HX;
} t_registro_cpu;

typedef struct{
    t_registro_cpu registros;
    //base y limite
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
uint32_t deserializar_finalizar_proceso(t_list*  lista_paquete );
void enviar_respuesta_finalizar_proceso(uint32_t pid_proceso_a_finalizar ,int socket_kernel);

void enviar_respuesta_contexto(t_m_contexto* pcbproceso, int socket_cpu);
t_proceso_memoria* deserializar_solicitud_instruccion(t_list*  lista_paquete );
void enviar_respuesta_instruccion(char* proxima_instruccion ,int socket_cpu);
t_escribir_leer* deserializar_read_memoria(t_list*  lista_paquete );
void enviar_respuesta_read_memoria(void* respuesta_leer, int socket_cpu);
t_escribir_leer* deserializar_write_memoria(t_list*  lista_paquete);
void enviar_respuesta_write_memoria(char* respuesta_escribir, int socket_cliente);


#endif /* MEMORIA_H */