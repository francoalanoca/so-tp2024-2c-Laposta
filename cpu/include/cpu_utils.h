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

instr_t* fetch(int conexion, t_pcb* proceso);
tipo_instruccion decode(instr_t* instr);
void execute(instr_t* inst,tipo_instruccion tipo_inst, t_pcb* proceso, int conexion,t_list* tlb,  int socket_dispatch, int socket_interrupt);
void check_interrupt( int conexion_kernel);
void pedir_instruccion(t_pcb* proceso,int conexion);
void set(char* registro, uint32_t valor, t_pcb* proceso);
void sum(char* registro_destino, char* registro_origen, t_pcb* proceso);
void sub(char* registro_destino, char* registro_origen, t_pcb* proceso);
void jnz(char* registro, uint32_t inst, t_pcb* proceso);
void io_gen_sleep(char* nombre_interfaz, uint32_t unidades_de_trabajo, t_pcb* proceso, int conexion_kernel );
void generar_interrupcion_a_kernel(int conexion);
//t_proceso_memoria* crear_proceso_memoria(t_proceso* proceso);
void* crear_servidor_dispatch(char* ip_cpu);//
void* crear_servidor_interrupt(char* ip_cpu);//
registros identificarRegistro(char* registro);
uint32_t obtenerValorActualRegistro(registros id_registro, t_pcb* proceso);
//t_interfaz* elegir_interfaz(char* interfaz, t_proceso* proceso);
void enviar_interfaz_a_kernel(char* nombre_interfaz, uint32_t tamanio_nombre, uint32_t unidades_de_trabajo, int conexion);
uint32_t mmu(uint32_t direccion_logica, uint32_t tamanio_pag, int conexion, t_log* logger);
bool verificar_existencia_en_tlb(uint32_t pid, uint32_t nro_pagina, uint32_t indice);
//char* uint32_to_string(uint32_t number);
//char* concatenar_cadenas(const char* str1, const char* str2);
//uint32_t string_a_uint32(const char* str);
void read_mem(char* registro_datos, char* registro_direccion, t_pcb* proceso, t_log* logger, int conexion);
void write_mem(char* registro_direccion, char* registro_datos, t_pcb* proceso, t_log* logger,int conexion);
void wait_inst(char* recurso, int conexion_kernel);
void signal_inst(char* recurso, int conexion_kernel);
void pedir_valor_a_memoria(uint32_t dir_fisica, uint32_t pid, uint32_t tamanio, int conexion);
void exit_inst( int conexion_kernel);


void solicitar_wait_kernel(t_pcb* pcb,uint32_t recurso_tamanio ,char* recurso, int conexion_kernel);
void solicitar_signal_kernel(t_pcb* pcb,uint32_t recurso_tamanio,char* recurso, int conexion_kernel);



uint32_t valor_repetido_tlb(uint32_t pid, uint32_t nro_pag);
void obtenerTamanioPagina(int conexion);

void limpiarCadena(char* cadena);
uint32_t obtenerTamanioRegistro(registros id_registro);
void generar_interrupcion_a_kernel(int conexion);
#endif //CPU_UTILS_H