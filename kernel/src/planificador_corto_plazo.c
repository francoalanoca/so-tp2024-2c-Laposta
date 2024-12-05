#include "../include/init_kernel.h"

void inicializar_hilos_planificador()
{

    if (strcmp(config_kernel->algoritmo_planif, "FIFO") == 0)
    {
        log_info(logger_kernel, "Planificador FIFO seleccionado");
        crear_hilo_planificador_fifo();
    }
    else if (strcmp(config_kernel->algoritmo_planif, "PRIORIDADES") == 0)
    {
        log_info(logger_kernel, "Planificador por Prioridades seleccionado");
        crear_hilo_planificador_prioridades();
    }
    else if (strcmp(config_kernel->algoritmo_planif, "CMN") == 0)
    {
        log_info(logger_kernel, "Planificador por Colas Multinivel seleccionado");
        crear_hilo_planificador_colas_multinivel();
    }
    else
    {
        log_error(logger_kernel, "Planificador no reconocido");
    }
}
void crear_hilo_planificador_fifo()
{
    pthread_create(&(hilos->hilo_fifo), NULL, planificar_fifo, NULL);
    pthread_detach(hilos->hilo_fifo);
    log_info(logger_kernel, "Hilo de Planificador FIFO creado correctamente");
}
void crear_hilo_planificador_prioridades()
{
    pthread_create(&(hilos->hilo_prioridades), NULL, planificar_prioridades, NULL);
    pthread_detach(hilos->hilo_prioridades);
    log_info(logger_kernel, "Hilo de Planificador por Prioridades creado correctamente");
}

void crear_hilo_planificador_colas_multinivel()
{
    pthread_create(&(hilos->hilo_colas_multinivel), NULL, planificar_colas_multinivel, NULL);
    pthread_detach(hilos->hilo_colas_multinivel);
    log_info(logger_kernel, "Hilo de Planificador por Colas Multinivel creado correctamente");
}

void *planificar_fifo()
{
    // mueve procesos de ready a exec
    while (1)
    {

        sem_wait(&(semaforos->contador_threads_en_ready));
        sem_wait(&(semaforos->espacio_en_cpu));
        // saco de ready

        t_tcb *tcb = NULL;
        tcb = remover_de_lista(lista_ready,0,&(semaforos->mutex_lista_ready));

        // agrego a exec
        agregar_a_lista(tcb,lista_exec,&(semaforos->mutex_lista_exec));

        enviar_thread_a_cpu(tcb,config_kernel->conexion_cpu_dispatch);
    }
}

void *planificar_prioridades()
{
    while (1)
    {
        sem_wait(&(semaforos->contador_threads_en_ready));
        sem_wait(&(semaforos->espacio_en_cpu));
        // saco de ready
        sem_wait(&(semaforos->mutex_lista_ready));
        t_tcb *tcb = NULL;
        int indice_del_prioritario = buscar_indice_de_mayor_prioridad();
        tcb = list_remove(lista_ready, indice_del_prioritario);
        sem_post(&(semaforos->mutex_lista_ready));
        // agrego a exec
        agregar_a_lista(tcb,lista_exec,&(semaforos->mutex_lista_exec));
        enviar_thread_a_cpu(tcb,config_kernel->conexion_cpu_dispatch);
    }
}

int buscar_indice_de_mayor_prioridad()
{
    t_tcb *tcb_mayor_prioridad = NULL;
    int indice_mayor_prioridad = -1;

    // Recorremos la lista
    for (int i = 0; i < list_size(lista_ready); i++)
    {
        t_tcb *tcb_actual = (t_tcb *)list_get(lista_ready, i);

        // Si es el primer elemento o el actual tiene mayor prioridad (menor valor)
        if (tcb_mayor_prioridad == NULL || tcb_actual->prioridad < tcb_mayor_prioridad->prioridad)
        {
            tcb_mayor_prioridad = tcb_actual;
            indice_mayor_prioridad = i;
        }
    }
    return indice_mayor_prioridad;
}

void *planificar_colas_multinivel()
{
    while (1)
    {
        sem_wait(&(semaforos->contador_threads_en_ready));
        sem_wait(&(semaforos->espacio_en_cpu));
        // saco de ready
        sem_wait(&(semaforos->mutex_lista_ready));
        t_tcb *tcb = NULL;
        int indice_del_prioritario = buscar_indice_de_mayor_prioridad();

        tcb = list_remove(lista_ready, indice_del_prioritario);
        
        sem_post(&(semaforos->mutex_lista_ready));
        // agrego a exec
        agregar_a_lista(tcb,lista_exec,&(semaforos->mutex_lista_exec));
        enviar_thread_a_cpu(tcb,config_kernel->conexion_cpu_dispatch);
        pthread_t hilo_quantum;
        pthread_create(&hilo_quantum, NULL, interrupcion_quantum, (void*)tcb);
        pthread_detach(hilo_quantum);
    }
}
 // -----------------------------------------------------------------------------------------


void verificar_fin_ejecucion_prev_quantum(){
    while(1){
        sem_wait(&(semaforos->sem_finalizacion_ejecucion_cpu));
        //Destruir hilo de quantum
       // pthread_cancel(hilos->hilo_quantum);
       // log_info(logger_kernel,"Se finalizo el hilo de quantum");
    }
}
void iniciar_quantum()
{
    pthread_t hilo_quantum;
    pthread_create(&hilo_quantum, NULL, interrupcion_quantum, NULL);
    pthread_detach(hilo_quantum);
}
void *interrupcion_quantum(void* tcb_v){
    t_tcb* tcb_ant=(t_tcb*)tcb_v;
    sem_wait(&(semaforos->mutex_lista_exec));
    t_tcb *tcb = list_get(lista_exec, 0);
    sem_post(&(semaforos->mutex_lista_exec));
    usleep(config_kernel->quantum * 1000);
    if(tcb->pid==tcb_ant->pid &&tcb->tid==tcb_ant->tid){
    log_error(logger_kernel,"enviada interrupt, fin de Q de pid:%d, tid:%d , id_quantum=%d",tcb->pid,tcb->tid,contador_id_quantums);
    enviar_interrumpir_cpu(tcb, FIN_DE_QUANTUM);

    }
     contador_id_quantums++;
    //TODO: que pasa si se interrumpe antes el tcb, por ej por IO
    return NULL;
}
// -----------------------------------------------------------------------------------------

void enviar_interrumpir_cpu(t_tcb* tcb, int motivo_interrrupt){
    t_paquete* paquete=crear_paquete(FIN_DE_QUANTUM);
    agregar_a_paquete(paquete,&(tcb->pid),sizeof(int));
    agregar_a_paquete(paquete,&(tcb->tid),sizeof(int));
    agregar_a_paquete(paquete,&contador_id_quantums,sizeof(int));
    enviar_paquete(paquete,config_kernel->conexion_cpu_interrupt);
    eliminar_paquete(paquete);
}
void pasar_ready_a_execute()
{
    mover_procesos(lista_ready, lista_exec, &(semaforos->mutex_lista_ready), &(semaforos->mutex_lista_exec), EXEC);
}
void pasar_execute_a_ready()
{
    mover_procesos(lista_exec, lista_ready, &(semaforos->mutex_lista_exec), &(semaforos->mutex_lista_ready), READY);
}
void pasar_blocked_a_exit()
{
    mover_procesos(lista_blocked, lista_exit, &(semaforos->mutex_lista_blocked), &(semaforos->mutex_lista_exit), EXIT);
}
void pasar_blocked_a_ready()
{
    mover_procesos(lista_blocked, lista_ready, &(semaforos->mutex_lista_blocked), &(semaforos->mutex_lista_ready), READY);
}
void pasar_execute_a_exit()
{
    mover_procesos(lista_exec, lista_exit, &(semaforos->mutex_lista_exec), &(semaforos->mutex_lista_exit), EXIT);
}
void pasar_execute_a_blocked()
{
    mover_procesos(lista_exec, lista_blocked, &(semaforos->mutex_lista_exec), &(semaforos->mutex_lista_blocked), BLOCKED);
}