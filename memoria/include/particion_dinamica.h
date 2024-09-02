#ifndef PARTICION_DINAMICA_H
#define PARTICION_DINAMICA_H

#include <pthread.h>
#include <commons/string.h>

#include <stdlib.h>
#include <stdio.h>
#include <utils/utils.h>

#include "../include/init_memoria.h"


//----------------------------------Estructuras---------------------------------



//----------------------------------Variables Externs-------------------------



//----------------------------------Prototipos---------------------------------
void crear_proceso_dinamico(uint32_t proceso_pid, uint32_t tamanio_proceso);

#endif 