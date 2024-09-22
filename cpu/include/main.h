#ifndef CPU_MAIN_H
#define CPU_MAIN_H

#include <stdlib.h>
#include <stdio.h>
#include "../include/init_cpu.h"
#include <utils/utils.h>
#include <signal.h>
#include "../include/cpu_utils.h"
#include "../include/server_cpu.h"


typedef struct {
    int socket_memoria;
    t_proceso *proceso_actual;
    t_list *tlb;
    t_list* lista_conexion_kernel_dispatch;
    int conexion_kernel_interrupt;
} ciclo_params_t;


void ejecutar_ciclo(void* arg) ;






#endif //CPU_MAIN_H