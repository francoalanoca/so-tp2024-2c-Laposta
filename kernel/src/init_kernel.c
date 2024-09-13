#include <../include/init_kernel.h>

t_config_kernel* config_kernel;
t_log* logger_kernel;
t_semaforos* semaforos;
t_hilos* hilos;
int pid_AI_global;

void iniciar_modulo( char *ruta_config){
    logger_kernel=log_create("logs_kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
    cargar_config_kernel(ruta_config);
    pid_AI_global=0;
    semaforos=malloc(sizeof(t_semaforos));
    hilos=malloc(sizeof(t_hilos));
    inicializar_listas();
    inicializar_semaforos();
    inicializar_hilos_planificador();//corto_plazo
    inicializar_hilos_largo_plazo();
    
   
 
   
}
void cargar_config_kernel(char* ruta_config){
    t_config* config=iniciar_config(ruta_config,logger_kernel);
    config_kernel=malloc(sizeof(t_config_kernel));
    config_kernel->algoritmo_planif=config_get_string_value(config,"ALGORITMO_PLANIFICACION");
    config_kernel->ip_cpu=config_get_string_value(config,"IP_CPU");
    config_kernel->ip_memoria=config_get_string_value(config,"IP_MEMORIA");
    config_kernel->puerto_memoria=config_get_string_value(config,"PUERTO_MEMORIA");
    config_kernel->log_level=config_get_string_value(config,"LOG_LEVEL");
    config_kernel->puerto_dispatch=config_get_string_value(config,"PUERTO_CPU_DISPATCH");
    config_kernel->puerto_interrupt=config_get_string_value(config,"PUERTO_CPU_INTERRUPT");
    config_kernel->quantum=config_get_int_value(config,"QUANTUM");
    
    log_info(logger_kernel,"configuracion cargada");
}

void inicializar_semaforos(){
    semaforos=malloc(sizeof(t_semaforos));
    sem_init(&(semaforos->mutex_lista_new), 0, 1);
    sem_init(&(semaforos->mutex_lista_ready), 0, 1);
	sem_init(&(semaforos->mutex_lista_exit), 0, 1);
	sem_init(&(semaforos->mutex_lista_exec), 0 ,1);
	sem_init(&(semaforos->mutex_lista_blocked), 0 ,1);
    sem_init(&(semaforos->inicializar_planificador), 0, 0);
    sem_init(&(semaforos->sem_procesos_new), 0, 0);
    sem_init(&(semaforos->mutex_lista_global_procesos), 0 ,1);
}

int conectar_a_memoria(){
    int conexion_memoria = crear_conexion(
        logger_kernel
        ,"MEMORIA"
        ,config_kernel->ip_memoria
        ,config_kernel->puerto_memoria);
    return conexion_memoria;
}
void generar_conexiones_a_cpu() {

	pthread_t conexion_cpu_dispatch_hilo;
	pthread_t conexion_cpu_interrupt_hilo;
    //conecta a por dispatch a cpu
	config_kernel->conexion_cpu_dispatch = crear_conexion(
        logger_kernel
        ,"CPU"
        ,config_kernel->ip_cpu
        ,config_kernel->puerto_dispatch
    );
	
	pthread_create(&conexion_cpu_dispatch_hilo,NULL,(void*) procesar_conexion_dispatch,(void *)&(config_kernel->conexion_cpu_dispatch));
	pthread_detach(conexion_cpu_dispatch_hilo);

    //conecta a por interrupt a cpu
	config_kernel->conexion_cpu_interrupt= crear_conexion(
        logger_kernel
        ,"CPU"
        ,config_kernel->ip_cpu
        ,config_kernel->puerto_interrupt);
	pthread_create(&conexion_cpu_interrupt_hilo, NULL, (void*) procesar_conexion_interrupt, (void *)&(config_kernel->conexion_cpu_interrupt));
	pthread_detach(conexion_cpu_interrupt_hilo);

}
void procesar_conexion_interrupt(void* socket){
    //TODO: operaciones a ejecutar
    int fd_conexion_cpu=*((int*)socket);
    int operacion=recibir_operacion(fd_conexion_cpu);
    switch (operacion)
    {
    case 1:
        /* code */
        break;
    
    default:
        break;
    }
}

void procesar_conexion_dispatch(void* socket){
    //TODO: operaciones a ejecutar
    int fd_conexion_cpu=*((int*)socket);
    int operacion=recibir_operacion(fd_conexion_cpu);
    switch (operacion)
    {
    case INICIAR_PROCESO :
            log_info(logger_kernel,"se recibio instruccion INICIAR PROCESO");
             t_list* params_para_creacion=recibir_paquete(fd_conexion_cpu);
             char* ruta_codigo=list_get(params_para_creacion,0);
             int tamanio_proceso=*((int*)list_get(params_para_creacion,1));
             int prioridad_main=*((int*)list_get(params_para_creacion,2));

             process_create(ruta_codigo,tamanio_proceso,prioridad_main);
             list_destroy(params_para_creacion);
        /* code */
        break;
    
    default:
        break;
    }
}

void mostrar_pcb(t_pcb* pcb, t_log* logger) {
    // Verificamos que el PCB no sea NULL
    if (pcb == NULL) {
        log_error(logger, "El PCB es NULL");
        return;
    }

    log_info(logger, "Mostrando información del PCB:");
    log_info(logger, "PID: %d", pcb->pid);
    log_info(logger, "Contador auto-incremental de TIDs: %d", pcb->contador_AI_tids);
    
    // Mostrar lista de TIDs
    if (pcb->lista_tids != NULL) {
        log_info(logger, "Lista de TIDs:");
        for (int i = 0; i < list_size(pcb->lista_tids); i++) {
            int* tid = (int*) list_get(pcb->lista_tids, i);
            log_info(logger, "\tTID #%d: %d", i, *tid);
        }
    } else {
        log_warning(logger, "La lista de TIDs es NULL");
    }

    // Mostrar lista de Mutex
    if (pcb->lista_mutex != NULL) {
        log_info(logger, "Lista de Mutex:");
        for (int i = 0; i < list_size(pcb->lista_mutex); i++) {
            t_mutex *mutex_aux = list_get(pcb->lista_mutex, i);
            log_info(logger, "\tMutex #%d: %d", i, mutex_aux->cod_op);
        }
    } else {
        log_warning(logger, "La lista de Mutex es NULL");
    }

    log_info(logger, "Tamaño del proceso: %d", pcb->tamanio_proceso);

    if (pcb->ruta_pseudocodigo != NULL) {
        log_info(logger, "Ruta del pseudocódigo: %s", pcb->ruta_pseudocodigo);
    } else {
        log_warning(logger, "La ruta del pseudocódigo es NULL");
    }
}

