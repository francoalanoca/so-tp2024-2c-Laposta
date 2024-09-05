#ifndef KERNEL_H_
#define KERNEL_H_
#include <utils/utils.h>
#include "manejo_colas.h"
#include "semaphore.h"
#define HILO_MAIN 0
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

} t_config_kernel;

extern t_config_kernel *config_kernel;
extern t_log *logger_kernel;
extern int pid_AI_global;//contador de pids de procesos

//----------------- colas

extern t_cola_mutex *cola_de_ready;
extern t_cola_mutex *cola_de_new;
extern t_cola_mutex *cola_de_exit;
typedef struct
{
    //atrib minimos requeridos
    int pid;
    int contador_AI_tids;//contador auto-incremental TODO: FIXME: deberia asegurar con mutex??
    t_list *lista_tids;
    t_list *lista_mutex;

    //atrib de creacion 
    int tamanio_proceso;
    char* ruta_pseudocodigo;
} t_pcb;
typedef struct
{
    /* data */
    int tid;
    int prioridad;

    t_pcb* pcb;
    int quantum_th;
} t_tcb;

typedef struct
{
    int cod_op;//recurso, identificador del mutex
    t_list* lista_threads_bloquedos;
    bool locked;
} t_mutex;
typedef enum
{
    MUTEX_CREATE,
    MUTEX_LOCK,
    MUTEX_UNLOCK,
} t_op_mutex;

typedef struct{
    //TODO: agregar todos los semaforos globales aca
    sem_t sem_procesos_new;
    sem_t sem_procesos_ready;
    sem_t sem_espacio_liberado_por_proceso;
}t_semaforos;
extern t_semaforos* semaforos;
typedef struct{
    //TODO: agregar todos los hilos aca
    pthread_t hilo_planif_largo_plazo;
}t_hilos;
extern t_hilos *hilos;

int conectar_a_memoria();
void generar_conexiones_a_cpu();
void procesar_conexion(void *socket);
void iniciar_modulo(char *ruta_config);
void cargar_config_kernel(char *ruta_config);
void process_create(char* ruta_instrucciones,int tamanio_proceso,int prioridad_hilo_main);
void agregar_proceso_a_new(t_pcb* pcb);
void agregar_proceso_a_ready(t_pcb* pcb);
void inicializar_estructuras_new();
void inicializar_estructuras_ready();
void inicializar_estructuras_exit();
void *planificar_largo_plazo();
void iniciar_planificador_largo_plazo();
t_pcb* crear_pcb(int tam_proceso,char*archivo_instrucciones) ;
void añadir_tid_a_proceso(t_pcb* pcb);
void enviar_solicitud_espacio_a_memoria(void* pcb_solicitante,int socket);
int recibir_resp_de_memoria_a_solicitud(int socket_memoria);

#endif /* KERNEL_H_ */