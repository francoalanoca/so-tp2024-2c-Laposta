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
    inicializar_hilo_verificacion_fin_de_ejecucion();
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

void inicializar_hilo_verificacion_fin_de_ejecucion(){
    pthread_t hilo_verificacion_fin_ejecucion;
    pthread_create(&hilo_verificacion_fin_ejecucion, NULL, (void *)verificar_fin_ejecucion_prev_quantum, NULL);
    pthread_detach(hilo_verificacion_fin_ejecucion);
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
    sem_init(&(semaforos->sem_finalizacion_ejecucion_cpu),0,0);

    sem_init(&(semaforos->mutex_conexion_dispatch),0,1);
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


    // pthread_create(&conexion_cpu_dispatch_hilo, NULL, (void *)procesar_conexion_dispatch, (void *)&(config_kernel->conexion_cpu_dispatch));
     pthread_create(&conexion_cpu_dispatch_hilo, NULL, (void *)procesar_conexion_dispatch, NULL);
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
void enviar_respuesta_syscall_a_cpu(int respuesta){
    log_info(logger_kernel,"enviada respuesat de syscall");
    t_paquete *paquete=crear_paquete(RESPUESTA_SYSCALL);
    agregar_a_paquete(paquete, &respuesta,sizeof(int) );
    enviar_paquete(paquete,config_kernel->conexion_cpu_interrupt);
    eliminar_paquete(paquete);
}
void procesar_conexion_dispatch()
{
    // TODO: operaciones a ejecutar
    while (1)
    {

        int fd_conexion_cpu = config_kernel->conexion_cpu_dispatch;
        int operacion = recibir_operacion(fd_conexion_cpu);
        log_info(logger_kernel, "se recibio el codigo de operacion: %d", operacion);
        switch (operacion)
        {
        case SEGMENTATION_FAULT: // Sigo el mismo comportamiento que PROCESO_SALIR
            sem_post (&(semaforos->sem_finalizacion_ejecucion_cpu));
            t_list *params_proceso_seg_fault = recibir_paquete(fd_conexion_cpu);
            log_info(logger_kernel, "se recibio instruccion SEGMENTATION_FAULT:");

            sem_wait(&(semaforos->mutex_lista_exec));
            t_tcb *proceso_a_finalizar_seg_fault = (t_tcb *)list_get(lista_exec, 0);
            sem_post(&(semaforos->mutex_lista_exec));

            cancelar_hilos_asociados(proceso_a_finalizar_seg_fault->pid);

            pasar_execute_a_exit();

            enviar_respuesta_syscall_a_cpu(REPLANIFICACION);
            sem_post(&(semaforos->espacio_en_cpu));

            break;
        case PROCESO_CREAR:
            log_info(logger_kernel, "se recibio instruccion INICIAR PROCESO");
            t_list *params_para_creacion = recibir_paquete(fd_conexion_cpu);
            char *ruta_codigo = list_get(params_para_creacion, 0);
            int tamanio_proceso = *((int *)list_get(params_para_creacion, 1));
            int prioridad_main = *((int *)list_get(params_para_creacion, 2));

            process_create(ruta_codigo, tamanio_proceso, prioridad_main);

            sem_wait(&(semaforos->mutex_lista_exec));
            t_tcb* sigue_ejecutando=(t_tcb*)list_get(lista_exec,0);
            sem_post(&(semaforos->mutex_lista_exec));
            
            //enviar_thread_a_cpu(sigue_ejecutando,fd_conexion_cpu);
            enviar_respuesta_syscall_a_cpu(CONTINUA_EJECUTANDO_HILO);
            break;
        case PROCESO_SALIR:
         t_list *params_proceso_salir = recibir_paquete(fd_conexion_cpu);
                log_info(logger_kernel, "se recibio instruccion PROCESO SALIR:");
                sem_post (&(semaforos->sem_finalizacion_ejecucion_cpu));
                //TODO: implementar la finalizacion de proceso: ELIMINACION DE HILOS RESTANTES (SI LOS HAY)

                sem_wait(&(semaforos->mutex_lista_exec));
                t_tcb *proceso_a_finalizar =(t_tcb*) list_get(lista_exec, 0);
                sem_post(&(semaforos->mutex_lista_exec));

                cancelar_hilos_asociados (proceso_a_finalizar->pid);

                pasar_execute_a_exit();
                //thread_exit(proceso_a_finalizar); //revisar esto, parace estar de mas
                enviar_respuesta_syscall_a_cpu(REPLANIFICACION);
                sem_post(&(semaforos->espacio_en_cpu));
                log_warning(logger_kernel, "HILOS EN EXIT");
                mostrar_tcbs(lista_exit,logger_kernel);
        	break;
        case HILO_CREAR:
            log_info(logger_kernel, "se recibio instruccion INICIAR HILO");
            t_list *params_thread = recibir_paquete(fd_conexion_cpu);
            char *codigo_th = list_get(params_thread, 0);
            int prioridad_th = *((int *)list_get(params_thread, 1));
            sem_wait(&(semaforos->mutex_lista_exec));
            int pid_asociado=((t_tcb *)list_get(lista_exec, 0))->pid;
            sem_post(&(semaforos->mutex_lista_exec));
            
            t_tcb* nuevo_tcb=thread_create(codigo_th, prioridad_th, pid_asociado);
            agregar_a_lista(nuevo_tcb,lista_ready,&(semaforos->mutex_lista_ready));
            sem_post(&(semaforos->contador_threads_en_ready));
            log_info(logger_kernel, "se agrego hilo a lista_ready");
 
           /* sem_wait(&(semaforos->mutex_lista_exec));
            sigue_ejecutando=(t_tcb*)list_get(lista_exec,0);
            sem_post(&(semaforos->mutex_lista_exec));
            
            enviar_thread_a_cpu(sigue_ejecutando,fd_conexion_cpu);*/
             enviar_respuesta_syscall_a_cpu(CONTINUA_EJECUTANDO_HILO);
            break;
        case MUTEX_CREAR: // recurso,pid
            t_list *params_mutex_create = recibir_paquete(fd_conexion_cpu);
             int pid_mutex = *((int *)list_get(params_mutex_create, 0));
            char *nombre_mutex = list_get(params_mutex_create, 1);
            log_info(logger_kernel, "se recibio instruccion MUTEX_CREATE %s",nombre_mutex);
           
            mutex_create(nombre_mutex, pid_mutex);
            
            sem_wait(&(semaforos->mutex_lista_exec));
            sigue_ejecutando=(t_tcb*)list_get(lista_exec,0);
            sem_post(&(semaforos->mutex_lista_exec));

            //enviar_thread_a_cpu(sigue_ejecutando,fd_conexion_cpu);
            enviar_respuesta_syscall_a_cpu(CONTINUA_EJECUTANDO_HILO);

            break;
        case IO_EJECUTAR: // PID, TID, tiempo de io en milisegundos
            log_info(logger_kernel, "se recibio instruccion IO");
            t_list *params_io = recibir_paquete(fd_conexion_cpu);

            int tiempo_io = *((int *)list_get(params_io, 1));
            ejecutar_io(tiempo_io);
            //la respuesta a la syscall es enviar otro hilo a cpu-->replanificar
            enviar_respuesta_syscall_a_cpu(REPLANIFICACION);
            sem_post(&(semaforos->espacio_en_cpu));
 
            break;
        case MUTEX_BLOQUEAR: // recurso.CPU me devuleve el control-> debo mandar algo a ejecutar
            sem_post (&(semaforos->sem_finalizacion_ejecucion_cpu));
            t_list *params_lock = recibir_paquete(fd_conexion_cpu);
            char *recurso = list_get(params_lock, 0);
            
            log_info(logger_kernel, "se recibio instruccion MUTEX_LOCK %s",recurso);

            mutex_lock(recurso);

            break;
        case MUTEX_DESBLOQUEAR://recurso. CPU me devuleve el control-> debo mandar algo a ejecutar
            sem_post (&(semaforos->sem_finalizacion_ejecucion_cpu));
            log_info(logger_kernel, "se recibio instruccion MUTEX_UNLOCK");
            t_list *params_unlock = recibir_paquete(fd_conexion_cpu);
            char *recurso_unlok = (char*)list_get(params_unlock, 0);
            sem_wait(&(semaforos->mutex_lista_exec));
            t_tcb *th_unlock = (t_tcb *)list_get(lista_exec, 0);
            sem_wait(&(semaforos->mutex_lista_exec));
            mutex_unlock(recurso_unlok,th_unlock);
        break;
        case HILO_JUNTAR://tid_target. CPU me devuleve el control-> debo mandar algo a ejecutar
            sem_post (&(semaforos->sem_finalizacion_ejecucion_cpu));
            t_list *params_juntar = recibir_paquete(fd_conexion_cpu);
            int tid_target = *((int *)list_get(params_juntar, 0));
             sem_wait(&(semaforos->mutex_lista_exec));
            t_tcb *th_en_exec = (t_tcb *)list_get(lista_exec, 0);
            sem_post(&(semaforos->mutex_lista_exec));
            log_info(logger_kernel, "se recibio instruccion HILO_JUNTAR a tid:%d",tid_target);
            
            thread_join(th_en_exec,tid_target);
        break;

        case HILO_SALIR:
            log_info(logger_kernel, "se recibio instruccion FINALIZAR_HILO");
            sem_post (&(semaforos->sem_finalizacion_ejecucion_cpu));
            recibir_paquete(fd_conexion_cpu);// recibo el paquete para no tener basura en el socket
            
            // quito de exec
            sem_wait(&(semaforos->mutex_lista_exec));
            t_tcb *thread_saliente = (t_tcb *)list_get(lista_exec, 0);
            sem_post(&(semaforos->mutex_lista_exec));
            thread_exit(thread_saliente);
            pasar_execute_a_exit();

        //  log_info(logger_kernel, "HILO EN EXEC luego de desalojar...");
        //  mostrar_tcbs(lista_exec,logger_kernel);
            // marca la cpu como libre
            enviar_respuesta_syscall_a_cpu(REPLANIFICACION);                        
            sem_post(&(semaforos->espacio_en_cpu));
            
            break;
        case HILO_CANCELAR:
            log_info(logger_kernel, "se recibio instruccion CANCELAR_HILO");
            t_list *params_th_cancel = recibir_paquete(fd_conexion_cpu);
            int tid = *((int *)list_get(params_th_cancel, 0));
            sem_wait(&(semaforos->mutex_lista_exec));
            t_tcb *thread_asociado = (t_tcb *)list_get(lista_exec, 0);
            sem_post(&(semaforos->mutex_lista_exec));
            thread_cancel(tid,thread_asociado->pid);
            //enviar_thread_a_cpu(thread_asociado,fd_conexion_cpu);
            enviar_respuesta_syscall_a_cpu(CONTINUA_EJECUTANDO_HILO);
            
            break;
        case PEDIDO_MEMORY_DUMP:
            log_info(logger_kernel, "se recibio instruccion DUMP_MEMORY");
            sem_post (&(semaforos->sem_finalizacion_ejecucion_cpu));
            memory_dump();
            // marca la cpu como libre
            enviar_respuesta_syscall_a_cpu(REPLANIFICACION);
            sem_post(&(semaforos->espacio_en_cpu));
            break;
        case FIN_DE_QUANTUM://Interrupcion
            log_info(logger_kernel, "se recibio instruccion FIN_DE_QUANTUM_OK");
            t_list *params_fin_q = recibir_paquete(fd_conexion_cpu);
            int pid_desalojo=*((int *)list_get(params_fin_q, 0));
            int tid_desalojo=*((int *)list_get(params_fin_q, 1));          
            manejar_interrupcion_fin_quantum();
        break;
        default:
            log_info(logger_kernel," OPERACION INVALIDA RECIBIDA DE CPU ");
            
            return EXIT_FAILURE;
         break;
        }
    }
}

void manejar_interrupcion_fin_quantum(){
    // Obtener el TCB del hilo que se ejecutó durante el quantum
    pasar_execute_a_ready();
    //habilito planificador y marco cpu como libre
    sem_post(&(semaforos->contador_threads_en_ready));
    sem_post(&(semaforos->espacio_en_cpu));
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
            int tid = *(int *)list_get(pcb->lista_tids, i);
            log_info(logger, "\tTID #%d: %d", i, tid);
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

void mostrar_tcbs(t_list* lista_tcb, t_log* logger) {
    if (lista_tcb == NULL || list_size(lista_tcb) == 0) {
        log_info(logger, "No hay hilos en la lista.");
        return;
    }

    log_info(logger, "Lista de TCBs:");
    for (int i = 0; i < list_size(lista_tcb); i++) {
        t_tcb* tcb = list_get(lista_tcb, i);

        log_info(logger, "TCB #%d:", i + 1);
        log_info(logger, "\tTID: %d", tcb->tid);
        log_info(logger, "\tPrioridad: %d", tcb->prioridad);
        log_info(logger, "\tPID: %d", tcb->pid);
        log_info(logger, "\tTiempo de IO: %d", tcb->tiempo_de_io);
        //log_info(logger, "\tEstado: %d", tcb->estado);  // Usa una función para convertir el estado a texto si es necesario

        if (tcb->thread_target != NULL) {
            t_tcb* target = (t_tcb*) tcb->thread_target;
            log_info(logger, "\tEsperando hilo TID: %d", target->tid);
        } else {
            log_info(logger, "\tNo está esperando ningún hilo.");
        }
        
        // Mostrar los mutex asignados
        if (tcb->mutex_asignados != NULL && list_size(tcb->mutex_asignados) > 0) {
            log_info(logger, "\tMutex asignados:");
            for (int j = 0; j < list_size(tcb->mutex_asignados); j++) {
                t_mutex* mutex = list_get(tcb->mutex_asignados, j);
                log_info(logger, "\t\tRecurso: %s", mutex->recurso);
            }
        } else {
            log_info(logger, "\tNo tiene mutex asignados.");
        }
    }
}