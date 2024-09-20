#ifndef CPU_UTILS_H
#define CPU_UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include "../include/init_cpu.h"
#include <utils/utils.h>
#include <signal.h>
#include <utils/hello.h>
#include<commons/config.h>
#include <pthread.h>
#include <math.h>

typedef enum
{
    PC,
	AX,
	BX,
	CX,
	DX,
    EX,
    FX,
    GX,
    HX,
    REG_NO_ENC
}registros;

instr_t* fetch(int conexion, t_proceso* proceso);
tipo_instruccion decode(instr_t* instr);
void execute(instr_t* inst,tipo_instruccion tipo_inst, t_proceso* proceso, int conexion,  int socket_dispatch, int socket_interrupt);
void check_interrupt( int conexion_kernel);
void pedir_instruccion(t_proceso* proceso,int conexion);
void set(char* registro, uint32_t valor, t_proceso* proceso);
void sum(char* registro_destino, char* registro_origen, t_proceso* proceso);
void sub(char* registro_destino, char* registro_origen, t_proceso* proceso);
void jnz(char* registro, uint32_t inst, t_proceso* proceso);
void enviar_process_create_a_kernel(char* nombre_pseudocodigo, char* tamanio_proceso, char* prioridad_hilo, int socket_dispatch);
void generar_interrupcion_a_kernel(int conexion);
//t_proceso_memoria* crear_proceso_memoria(t_proceso* proceso);
void* crear_servidor_dispatch(char* ip_cpu);//
void* crear_servidor_interrupt(char* ip_cpu);//
registros identificarRegistro(char* registro);
uint32_t obtenerValorActualRegistro(registros id_registro, t_proceso* proceso);
//t_interfaz* elegir_interfaz(char* interfaz, t_proceso* proceso);

//devuelve la direccion fisica
uint32_t mmu(uint32_t direccion_logica, int conexion, int pid);

//char* uint32_to_string(uint32_t number);
//char* concatenar_cadenas(const char* str1, const char* str2);
//uint32_t string_a_uint32(const char* str);
void read_mem(char* registro_datos, char* registro_direccion, t_proceso* proceso, t_log* logger, int conexion);
void write_mem(char* registro_direccion, char* registro_datos, t_proceso* proceso, t_log* logger,int conexion);
void mutex_lock_inst(char* recurso, int conexion_kernel);
void mutex_unlock_inst(char* recurso, int conexion_kernel);
void pedir_valor_a_memoria(uint32_t dir_fisica, uint32_t pid, uint32_t tamanio, int conexion);
void exit_inst( int conexion_kernel);


void solicitar_mutex_lock_kernel(t_proceso* pcb,uint32_t recurso_tamanio ,char* recurso, int conexion_kernel);
void solicitar_mutex_unlock_kernel(t_proceso* pcb,uint32_t recurso_tamanio,char* recurso, int conexion_kernel);



void obtener_base_particion(int conexion, int pid);

void limpiarCadena(char* cadena);

void generar_interrupcion_a_kernel(int conexion);
void enviar_contexto_a_memoria(t_proceso* proceso, int conexion);
#endif //CPU_UTILS_H