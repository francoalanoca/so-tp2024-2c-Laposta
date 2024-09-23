#include <../include/init_kernel.h>

t_config_kernel *config_kernel;
t_log *logger_kernel;
t_semaforos *semaforos;
t_hilos *hilos;
int pid_AI_global;
//t_io *interfaz_io;
int socket_cpu;
void iniciar_modulo(char *ruta_config)
{
    logger_kernel = log_create("logs_kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
    cargar_config_kernel(ruta_config);
    pid_AI_global = 0;
    semaforos = malloc(sizeof(t_semaforos));
    hilos = malloc(sizeof(t_hilos));
    inicializar_listas();
    inicializar_semaforos();
    inicializar_hilos_planificador(); // corto_plazo
    inicializar_hilos_largo_plazo();
    inicializar_hilo_intefaz_io();
}
void cargar_config_kernel(char *ruta_config)
{
    t_config *config = iniciar_config(ruta_config, logger_kernel);
    config_kernel = malloc(sizeof(t_config_kernel));
    config_kernel->algoritmo_planif = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    config_kernel->ip_cpu = config_get_string_value(config, "IP_CPU");
    config_kernel->ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    config_kernel->puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    config_kernel->log_level = config_get_string_value(config, "LOG_LEVEL");
    config_kernel->puerto_dispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
    config_kernel->puerto_interrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");
    config_kernel->quantum = config_get_int_value(config, "QUANTUM");

    log_info(logger_kernel, "configuracion cargada");
}

void inicializar_semaforos()
{
    sem_init(&(semaforos->mutex_lista_new), 0, 1);
    sem_init(&(semaforos->mutex_lista_ready), 0, 1);
    sem_init(&(semaforos->mutex_lista_exit), 0, 1);
    sem_init(&(semaforos->mutex_lista_exec), 0, 1);
    sem_init(&(semaforos->mutex_lista_blocked), 0, 1);
    sem_init(&(semaforos->mutex_lista_espera_io), 0, 1);
    sem_init(&(semaforos->inicializar_planificador), 0, 0);
    sem_init(&(semaforos->sem_procesos_new), 0, 0);
    sem_init(&(semaforos->mutex_lista_global_procesos), 0, 1);
    sem_init(&(semaforos->espacio_en_cpu), 0, 1);
    sem_init(&(semaforos->contador_threads_en_ready), 0, 0);
    //sem_init(&(semaforos->mutex_interfaz_io), 0, 1);
    //sem_init(&(semaforos->contador_tcb_en_io),0,0);
    sem_init(&(semaforos->sem_io_sleep_en_uso),0,1); //revisar aca si empieza con 1 0 0
    sem_init(&(semaforos->sem_io_solicitud),0,0);
    sem_init(&(semaforos->sem_sleep_io),0,0);
}

int conectar_a_memoria()
{
    int conexion_memoria = crear_conexion(
        logger_kernel, "MEMORIA", config_kernel->ip_memoria, config_kernel->puerto_memoria);
    return conexion_memoria;
}
void generar_conexiones_a_cpu()
{
    pthread_t conexion_cpu_dispatch_hilo;
    pthread_t conexion_cpu_interrupt_hilo;
    log_info(logger_kernel, "ip cpu:%s",config_kernel->ip_cpu);

    log_info(logger_kernel, "ip cpu:%s",config_kernel->puerto_dispatch);
    // conecta a por dispatch a cpu
    config_kernel->conexion_cpu_dispatch = crear_conexion(
        logger_kernel, "CPU", config_kernel->ip_cpu, config_kernel->puerto_dispatch);
        if(config_kernel->conexion_cpu_dispatch>0)
            log_info(logger_kernel, "conectado a cpu");

    socket_cpu=crear_conexion(logger_kernel,"CPU",config_kernel->ip_cpu,config_kernel->puerto_dispatch);
        if(config_kernel->conexion_cpu_dispatch>0)
            log_info(logger_kernel, "conectado a cpu2");
    pthread_create(&conexion_cpu_dispatch_hilo, NULL, (void *)procesar_conexion_dispatch, (void *)&(config_kernel->conexion_cpu_dispatch));
    pthread_detach(conexion_cpu_dispatch_hilo);
    

    // conecta a por interrupt a cpu
    config_kernel->conexion_cpu_interrupt = crear_conexion(logger_kernel, "CPU", config_kernel->ip_cpu, config_kernel->puerto_interrupt);
    // pthread_create(&conexion_cpu_interrupt_hilo, NULL, (void*) procesar_conexion_interrupt, (void *)&(config_kernel->conexion_cpu_interrupt));
    // pthread_detach(conexion_cpu_interrupt_hilo);
}

// void iniciar_interfaz_io()
// {
//     t_io *interfaz_io = malloc(sizeof(t_io));
//     interfaz_io->en_ejecucion = false;
//     interfaz_io->thread_en_io = NULL;
//     interfaz_io->threads_en_espera = list_create();
// }

void procesar_conexion_dispatch(void *socket)
{
    // TODO: operaciones a ejecutar
    while (1)
    {

        int fd_conexion_cpu = *((int *)socket);
        int operacion = recibir_operacion(fd_conexion_cpu);
        switch (operacion)
        {
        case PROCESO_CREAR:
            log_info(logger_kernel, "se recibio instruccion INICIAR PROCESO");
            t_list *params_para_creacion = recibir_paquete(fd_conexion_cpu);
            char *ruta_codigo = list_get(params_para_creacion, 0);
            int tamanio_proceso = *((int *)list_get(params_para_creacion, 1));
            int prioridad_main = *((int *)list_get(params_para_creacion, 2));

            process_create(ruta_codigo, tamanio_proceso, prioridad_main);

            break;
        case HILO_CREAR:
            log_info(logger_kernel, "se recibio instruccion INICIAR HILO");
            t_list *params_thread = recibir_paquete(fd_conexion_cpu);
            char *codigo_th = list_get(params_thread, 0);
            int prioridad_th = *((int *)list_get(params_thread, 1));
            int pid_asociado = *((int *)list_get(params_thread, 2));
            thread_create(codigo_th, prioridad_th, pid_asociado);

            break;
        case MUTEX_CREAR: // recurso,pid
            log_info(logger_kernel, "se recibio instruccion MUTEX_CREATE");
            t_list *params_mutex_create = recibir_paquete(fd_conexion_cpu);
            char *nombre_mutex = list_get(params_mutex_create, 0);
            int pid_mutex = *((int *)list_get(params_mutex_create, 1));
            mutex_create(nombre_mutex, pid_mutex);

            break;
        case IO_EJECUTAR: // PID, TID, tiempo de io en milisegundos
            log_info(logger_kernel, "se recibio instruccion IO");
            t_list *params_io = recibir_paquete(fd_conexion_cpu);

            int tiempo_io = *((int *)list_get(params_io, 1));
            //ejecutar_io(tiempo_io);

            break;
        case MUTEX_BLOQUEAR: // recurso.CPU me devuleve el control-> debo mandar algo a ejecutar
            log_info(logger_kernel, "se recibio instruccion MUTEX_LOCK");
            t_list *params_lock = recibir_paquete(fd_conexion_cpu);
            char *recurso = list_get(params_lock, 0);
            mutex_lock(recurso);

            break;
        case MUTEX_DESBLOQUEAR://recurso. CPU me devuleve el control-> debo mandar algo a ejecutar
            log_info(logger_kernel, "se recibio instruccion MUTEX_UNLOCK");
            t_list *params_unlock = recibir_paquete(fd_conexion_cpu);
            char *recurso_unlok = (char*)list_get(params_unlock, 0);
            sem_wait(&(semaforos->mutex_lista_exec));
            t_tcb *th_unlock = (t_tcb *)list_get(lista_exec, 0);
            sem_wait(&(semaforos->mutex_lista_exec));
            mutex_unlock(recurso_unlok,th_unlock);
        break;
        case HILO_JUNTAR://tid_target. CPU me devuleve el control-> debo mandar algo a ejecutar
            log_info(logger_kernel, "se recibio instruccion HILO_JUNTAR");
            t_list *params_juntar = recibir_paquete(fd_conexion_cpu);
            int tid_target = *((int *)list_get(params_juntar, 0));
             sem_wait(&(semaforos->mutex_lista_exec));
            t_tcb *th_en_exec = (t_tcb *)list_get(lista_exec, 0);
            sem_wait(&(semaforos->mutex_lista_exec));
            thread_join(th_en_exec,tid_target);
        break;

        case HILO_SALIR:
            log_info(logger_kernel, "se recibio instruccion FINALIZAR_HILO");
            // quito de exec
            sem_wait(&(semaforos->mutex_lista_exec));
            t_tcb *thread_saliente = (t_tcb *)list_get(lista_exec, 0);
            sem_wait(&(semaforos->mutex_lista_exec));
            thread_exit(thread_saliente);
            pasar_execute_a_exit();
            // marca la cpu como libre
            sem_post(&(semaforos->espacio_en_cpu));
            
            
            break;
        case HILO_CANCELAR:
            log_info(logger_kernel, "se recibio instruccion CANCELAR_HILO");
            t_list *params_th_cancel = recibir_paquete(fd_conexion_cpu);
            int tid = *((int *)list_get(params_th_cancel, 0));
            sem_wait(&(semaforos->mutex_lista_exec));
            t_tcb *thread_asociado = (t_tcb *)list_get(lista_exec, 0);
            sem_wait(&(semaforos->mutex_lista_exec));
            thread_cancel(tid,thread_asociado->pid);

            break;

        default:

         break;
        }
    }
}


void mostrar_pcb(t_pcb *pcb, t_log *logger)
{
    // Verificamos que el PCB no sea NULL
    if (pcb == NULL)
    {
        log_error(logger, "El PCB es NULL");
        return;
    }

    log_info(logger, "Mostrando información del PCB:");
    log_info(logger, "PID: %d", pcb->pid);
    log_info(logger, "Contador auto-incremental de TIDs: %d", pcb->contador_AI_tids);

    // Mostrar lista de TIDs
    if (pcb->lista_tids != NULL)
    {
        log_info(logger, "Lista de TIDs:");
        for (int i = 0; i < list_size(pcb->lista_tids); i++)
        {
            int *tid = (int *)list_get(pcb->lista_tids, i);
            log_info(logger, "\tTID #%d: %d", i, *tid);
        }
    }
    else
    {
        log_warning(logger, "La lista de TIDs es NULL");
    }

    // Mostrar lista de Mutex
    if (pcb->lista_mutex != NULL)
    {
        log_info(logger, "Lista de Mutex:");
        for (int i = 0; i < list_size(pcb->lista_mutex); i++)
        {
            t_mutex *mutex_aux = list_get(pcb->lista_mutex, i);
            log_info(logger, "\tMutex #%d: %s", i, mutex_aux->recurso);
        }
    }
    else
    {
        log_warning(logger, "La lista de Mutex es NULL");
    }

    log_info(logger, "Tamaño del proceso: %d", pcb->tamanio_proceso);

    if (pcb->ruta_pseudocodigo != NULL)
    {
        log_info(logger, "Ruta del pseudocódigo: %s", pcb->ruta_pseudocodigo);
    }
    else
    {
        log_warning(logger, "La ruta del pseudocódigo es NULL");
    }
}
