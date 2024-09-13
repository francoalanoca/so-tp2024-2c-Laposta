#ifndef KERNEL_H_
#define KERNEL_H_
#include <utils/utils.h>
//#include "manejo_colas.h"
#include "semaphore.h"
#include <commons/collections/list.h>
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

typedef enum estado{
    NEW,
    READY,
    EXEC,
    BLOCKED,
    EXIT
} t_estado;
typedef struct
{
    //atrib minimos requeridos
    int pid;
    int contador_AI_tids;//contador auto-incremental TODO: FIXME: deberia asegurar con mutex??
    t_list *lista_tids;
    t_list *lista_mutex;
    t_estado estado;

    //atrib de creacion 
    int tamanio_proceso;
    char* ruta_pseudocodigo;
    int prioridad_th_main;
} t_pcb;

typedef struct
{
    /* data */
    int tid;
    int prioridad;
    int pid;
    int quantum_th;
    t_estado estado;
} t_tcb;

typedef struct
{
    int cod_op;//recurso, identificador del mutex
    t_list* lista_threads_bloquedos;
    bool locked;
} t_mutex;

//---------------------- HILOS ------------------------------
typedef struct{
    //TODO: agregar todos los hilos aca
    pthread_t hilo_planif_largo_plazo;
    pthread_t hilo_fifo;
    pthread_t hilo_prioridades;
    pthread_t hilo_colas_multinivel;
}t_hilos;

extern t_hilos *hilos;

//---------------------------SEMAFOROS------------------------



typedef struct{
    //TODO: agregar todos los semaforos globales aca
    sem_t mutex_lista_new;
    sem_t mutex_lista_ready;
    sem_t mutex_lista_exit;
    sem_t mutex_lista_exec;
    sem_t mutex_lista_blocked;
    sem_t inicializar_planificador;
    sem_t sem_procesos_new;//contado de procesos en new
    sem_t sem_procesos_ready;
    sem_t sem_espacio_liberado_por_proceso;
    sem_t mutex_lista_global_procesos;
}t_semaforos;
extern t_semaforos* semaforos;

//------------------------------LISTAS-------------------------
extern t_list* lista_ready; 
extern t_list* lista_exec;
extern t_list* lista_blocked;
extern t_list* lista_exit;
extern t_list* lista_new;
extern t_list* lista_procesos_global;

int conectar_a_memoria();
void generar_conexiones_a_cpu();
void procesar_conexion_interrupt(void *socket);
void procesar_conexion_dispatch(void *socket);
void iniciar_modulo(char *ruta_config);
void cargar_config_kernel(char *ruta_config);
void process_create(char* ruta_instrucciones,int tamanio_proceso,int prioridad_hilo_main);
t_pcb* crear_pcb(int tam_proceso,char*archivo_instrucciones,int prioridad) ;
void añadir_tid_a_proceso(t_pcb* pcb);
void enviar_solicitud_espacio_a_memoria(void* pcb_solicitante,int socket);
int recibir_resp_de_memoria_a_solicitud(int socket_memoria);

void inicializar_hilos_largo_plazo();
void inicializar_hilos_planificador();
void crear_hilo_planificador_fifo();
void crear_hilo_planificador_prioridades();
void crear_hilo_planificador_colas_multinivel();
void *planificar_fifo();
void *planificar_prioridades();
void *planificar_colas_multinivel();
void agregar_a_cola(t_pcb *pcb,t_list* lista,sem_t* sem);
void pasar_new_a_ready();
void pasar_ready_a_exit();
void pasar_new_a_exit();
void inicializar_semaforos();
void pasar_ready_a_execute();
void pasar_execute_a_ready();
void pasar_blocked_a_exit();
void pasar_blocked_a_ready();
void pasar_execute_a_exit();
void pasar_execute_a_blocked();
void* planificar_procesos();
void inicializar_listas();
t_pcb* buscar_proceso_por(int pid_buscado);
void mostrar_pcb(t_pcb* pcb, t_log* logger);
t_tcb* thread_create(char* pseudocodigo,int prio,int pid);

#endif /* KERNEL_H_ */