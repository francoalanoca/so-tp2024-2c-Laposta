#ifndef UTILS_MEMORIA_H
#define UTILS_MEMORIA_H



#include "../include/init_memoria.h"





//----------------------------------Prototipos---------------------------------

t_m_crear_proceso* deserializar_iniciar_proceso(t_list*  lista_paquete );
void enviar_respuesta_iniciar_proceso(t_m_crear_proceso* crear_proceso ,int socket_kernel, op_code cod_ope);
t_m_crear_hilo* deserializar_iniciar_hilo(t_list*  lista_paquete );
void enviar_respuesta_iniciar_hilo(t_m_crear_hilo* crear_hilo ,int socket_kernel, op_code cod_ope);
uint32_t deserializar_finalizar_proceso(t_list*  lista_paquete );
void enviar_respuesta_finalizar_proceso(uint32_t pid_proceso_a_finalizar ,int socket_kernel, op_code cod_ope);
uint32_t deserializar_finalizar_hilo(t_list*  lista_paquete );
void enviar_respuesta_finalizar_hilo(uint32_t pid_proceso_a_finalizar ,uint32_t tid_proceso_a_finalizar,int socket_kernel,op_code cod_ope);

void enviar_respuesta_contexto(t_m_contexto* pcbproceso, int socket_cpu);
t_proceso_memoria* deserializar_solicitud_instruccion(t_list*  lista_paquete );
void enviar_respuesta_instruccion(char* proxima_instruccion ,int socket_cpu);
t_escribir_leer* deserializar_read_memoria(t_list*  lista_paquete );
void enviar_respuesta_read_memoria(void* respuesta_leer, int socket_cpu);
t_escribir_leer* deserializar_write_memoria(t_list*  lista_paquete);
void enviar_respuesta_write_memoria(char* respuesta_escribir, int socket_cliente);


#endif /* MEMORIA_H */