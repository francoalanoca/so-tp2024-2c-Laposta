#ifndef INIT_CPU_H
#define INIT_CPU_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <commons/string.h>
#include <utils/utils.h>
#include <semaphore.h>
#include <pthread.h>

extern bool interrupcion_kernel;
extern t_list* lista_sockets_global;
extern int conexion_kernel_dispatch;
extern int conexion_kernel_interrupt;
extern instr_t *prox_inst;
extern sem_t sem_valor_instruccion;
extern int socket_memoria;
extern char* valor_registro_obtenido;
extern sem_t sem_valor_registro_recibido;
extern int rta_resize;
extern sem_t sem_valor_resize_recibido;
extern sem_t sem_valor_tamanio_pagina;
extern sem_t sem_servidor_creado;
extern sem_t sem_interrupcion_kernel;
extern sem_t sem_check_interrupcion_kernel;
extern sem_t sem_conexion_interrupt_iniciado;
extern sem_t sem_conexion_dispatch_iniciado;
extern pthread_mutex_t mutex_proceso_actual;
extern pthread_mutex_t mutex_proceso_interrumpido_actual;
extern pthread_mutex_t mutex_interrupcion_kernel;
extern t_pcb* proceso_actual;
extern  uint32_t base;
typedef struct {
    t_log *log;
    int fd;
    char *server_name;
} t_procesar_conexion_args;

typedef struct {
    uint32_t pid; // Tamaño del payload
    uint32_t program_counter;
} t_pcb;

typedef struct 
{
    uint32_t PC;
    uint32_t AX;
    uint32_t BX;
    uint32_t CX;
    uint32_t DX;
    uint32_t EX;
    uint32_t FX;
    uint32_t GX;
    uint32_t HX;
}t_registros_CPU;

extern t_registros_CPU* registros_cpu;
typedef enum
{
    SET,
    READ_MEM,
    WRITE_MEM,
	SUM,
    SUB,
    JNZ,
    LOG
}tipo_instruccion;

typedef enum
{
    DUMP_MEMORY,
    IO,
    PROCESS_CREATE,
    THREAD_CREATE ,
    THREAD_JOIN,
    THREAD_CANCEL,
    MUTEX_CREATE,
    MUTEX_LOCK,
    MUTEX_UNLOCK,
    THREAD_EXIT,
    PROCESS_EXIT   
}sycall;

typedef struct {
    uint8_t idLength;
    tipo_instruccion id; // el id seria el nombre de la instruccion
    uint8_t param1Length;
    char* param1;
    uint8_t param2Length;
    char* param2;
    uint8_t param3Length;
    char* param3;
    uint8_t param4Length;
    char* param4;
    uint8_t param5Length;
    char* param5;
} instr_t;


int checkProperties(char *path_config);

int cargar_configuracion(char *path);

int init(char *path_config);

int hacer_handshake (int socket_cliente);


void cerrar_programa();

extern t_log *logger_cpu;
extern t_config *file_cfg_cpu;

typedef struct
{
    char *IP_MEMORIA;
    char* PUERTO_MEMORIA;
    char* PUERTO_ESCUCHA_DISPATCH;
    char* PUERTO_ESCUCHA_INTERRUPT;
    char *LOG_LEVEL;
} t_config_cpu;

extern t_config_cpu *cfg_cpu;


static t_config_cpu *cfg_cpu_start()
{
    t_config_cpu *cfg = malloc(sizeof(t_config_cpu));
    return cfg;
}

#endif /* INIT_CPU_H */