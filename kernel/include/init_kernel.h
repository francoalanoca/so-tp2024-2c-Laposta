#ifndef KERNEL_H
#define KERNEL_H
#include "utils/utils.h"
typedef struct 
{
    /* data */
    char *ip_memoria;
    char *puerto_memoria;
    char *ip_cpu;
    char *puerto_dispatch;
    char *puerto_interrupt;
    char *algoritmo_planif;
    int quantum;
    char *log_level;
    int conexion_cpu_dispatch;
    int conexion_cpu_interrupt;

}t_config_kernel;

extern t_config_kernel *config_kernel;
extern t_log* logger_kernel;
typedef struct{
    int pid;
    t_list* lista_tids;
    t_list* lista_mutex;

}t_pcb;
typedef struct t_tcb
{
    /* data */
    int tid;
    int prioridad;
};
typedef struct t_proceso
{
    /* data */
    t_list* lista_hilos;
    t_p
};

typedef struct{
    int cod_op;
}t_mutex;
typedef enum{
    MUTEX_CREATE,
    MUTEX_LOCK,
    MUTEX_UNLOCK,
}t_op_mutex;
//----------------- colas
extern t_list* cola_new;
extern t_list* cola_ready;
extern t_list* cola_blocked;
extern t_list* cola_exit;
void iniciar_modulo( char *ruta_config);
void cargar_config_kernel(char* ruta_config);
#endif /* KERNEL_H */