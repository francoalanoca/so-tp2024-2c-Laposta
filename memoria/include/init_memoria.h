#ifndef INIT_MEMORIA_H
#define INIT_MEMORIA_H


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <utils/utils.h>

#include <commons/bitarray.h>




//----------------------------------Estructuras---------------------------------

typedef struct{
    uint32_t PUERTO_ESCUCHA;
    char* IP_FILESYSTEM;
    uint32_t PUERTO_FILESYSTEM;
    uint32_t TAM_MEMORIA;
    char* PATH_INSTRUCCIONES;
    uint32_t RETARDO_RESPUESTA;
    char* ESQUEMA;
    char* ALGORITMO_BUSQUEDA;
    char** PARTICIONES;
    char* LOG_LEVEL;
} t_config_memoria;


typedef struct{
    uint32_t pid;
    t_list *lista_de_instrucciones;
} t_miniPCB;






//----------------------------------Variables Externs-------------------------


extern int socket_memoria;
extern int socket_cpu;
extern int socket_kernel;
extern int socket_filesystem;

extern t_log *logger_memoria;
extern t_config *file_cfg_memoria;
extern t_config_memoria *cfg_memoria;

extern void* memoria;
extern t_list* lista_particiones;
extern t_list* lista_miniPCBs;
extern pthread_mutex_t mutex_memoria;
extern uint32_t cantidad_particiones_memoria;
     
extern t_bitarray *bitmap_particiones;



//----------------------------------Prototipos---------------------------------


//int inicializar_configuraion(char* path_config);
t_config_memoria *cfg_memoria_start();
int init(char *path_config);
int checkProperties(char *path_config);
int cargar_configuracion(char *path_config);

int inicializar_memoria();
int redondear_a_multiplo_mas_cercano_de(int base, int valor);
t_bitarray *crear_bitmap(int entradas);

void cerrar_programa();


#endif /* MEMORIA_H */