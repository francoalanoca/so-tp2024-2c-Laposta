#include <../include/init_kernel.h>
t_pcb* crear_pcb(int tam_proceso,char* archivo_instrucciones,int prioridad_th0) {
    t_pcb* pcb = malloc(sizeof(t_pcb));
    pcb->contador_AI_tids=0;//inicializa el contador de tids del proceso
    pcb->lista_mutex=list_create();
    pcb->lista_tids=list_create();
    pcb->tamanio_proceso=tam_proceso;
    pcb->pid=pid_AI_global;
    pid_AI_global++;
    pcb->prioridad_th_main=prioridad_th0;
    pcb->ruta_pseudocodigo=strdup(archivo_instrucciones);
 
    return pcb;
}
void añadir_tid_a_proceso(t_pcb* pcb){
    list_add(pcb->lista_tids,&(pcb->contador_AI_tids));
    pcb->contador_AI_tids++;
}

void enviar_solicitud_espacio_a_memoria(t_pcb* pcb,int socket){
    //memoria solo necesita tamanio de proceso y el pid 
    t_paquete* paquete_a_enviar=crear_paquete(INICIAR_PROCESO);
    agregar_a_paquete(paquete_a_enviar,&(pcb->pid),sizeof(uint32_t));
    log_info(logger_kernel,"valor de pid:%d",pcb->pid);
    agregar_a_paquete(paquete_a_enviar,&(pcb->tamanio_proceso),sizeof(uint32_t));
    log_info(logger_kernel,"valor de tamanio:%d",pcb->tamanio_proceso);
    enviar_paquete(paquete_a_enviar,socket);
}
int recibir_resp_de_memoria_a_solicitud(int socket_memoria){
    return recibir_operacion(socket_memoria);

}

int asignar_tid(t_pcb* pcb){
    int tid_a_asignar=pcb->contador_AI_tids;
    list_add(pcb->lista_tids,&(tid_a_asignar));
    pcb->contador_AI_tids++;
    return tid_a_asignar;
}
t_tcb* crear_tcb(int prioridad_th,int pid){
    t_tcb* nuevo_tcb=malloc(sizeof(t_tcb));
    nuevo_tcb->prioridad=prioridad_th;
    nuevo_tcb->quantum_th=config_kernel->quantum;
    t_pcb* pcb=buscar_proceso_por(pid);
    nuevo_tcb->tid=asignar_tid(pcb);
    nuevo_tcb->pid=pid;
    return nuevo_tcb;
}
void enviar_a_memoria_creacion_thread(t_tcb* tcb_nuevo,char* pseudo,int socket){
    t_paquete* paquete_a_enviar=crear_paquete(INICIAR_HILO);
    int longitud=strlen(pseudo)+1;
    agregar_a_paquete(paquete_a_enviar,&(tcb_nuevo->pid),sizeof(int));
    log_info(logger_kernel,"valor de pid:%d",tcb_nuevo->pid);
    agregar_a_paquete(paquete_a_enviar,&(tcb_nuevo->tid),sizeof(int));
    log_info(logger_kernel,"valor de tid:%d",tcb_nuevo->tid);
    agregar_a_paquete(paquete_a_enviar,pseudo,longitud);
    log_info(logger_kernel,"ruta pseudocodigo:%s",pseudo);
    enviar_paquete(paquete_a_enviar,socket);  
}
t_pcb* buscar_proceso_por(int pid_buscado){
    t_pcb* un_pcb=NULL;
    
    for(int i=0;i<list_size(lista_procesos_global);i++){
        un_pcb=list_get(lista_procesos_global,i);
        if(un_pcb->pid==pid_buscado){
            return un_pcb;
        }
    }
   
    return un_pcb;
}
void enviar_thread_a_cpu(t_tcb* tcb_a_ejetucar){
    t_paquete * paquete=crear_paquete(PROCESO_EJECUTAR);
    agregar_a_paquete(paquete,&(tcb_a_ejetucar->pid),sizeof(int));
    agregar_a_paquete(paquete,&(tcb_a_ejetucar->tid),sizeof(int));
    enviar_paquete(paquete,config_kernel->conexion_cpu_dispatch);
}
//******************      SYSCALLS         *******************************
void process_create(char* ruta_instrucciones,int tam_proceso,int prioridad_hilo_main){
    t_pcb* pcb_nuevo=NULL;
    pcb_nuevo=crear_pcb(tam_proceso,ruta_instrucciones, prioridad_hilo_main);

    sem_wait(&(semaforos->mutex_lista_global_procesos));
        list_add(lista_procesos_global,pcb_nuevo);
    sem_post(&(semaforos->mutex_lista_global_procesos));

    sem_wait(&(semaforos->mutex_lista_new));
         list_add(lista_new,pcb_nuevo);
    sem_post(&(semaforos->mutex_lista_new));
    sem_post(&(semaforos->sem_procesos_new));
    log_info(logger_kernel, "Crear proceso: %s",ruta_instrucciones);
}

t_tcb* thread_create(char* pseudo_codigo,int prioridad_th,int pid){
    t_tcb* tcb_th=crear_tcb(prioridad_th, pid);
    int socket_memoria=conectar_a_memoria();
    enviar_a_memoria_creacion_thread( tcb_th,pseudo_codigo, socket_memoria);
    int rta_memoria=recibir_resp_de_memoria_a_solicitud(socket_memoria);
    if(rta_memoria==INICIAR_HILO_RTA_OK){
        log_info(logger_kernel,"memoria cargo las estructuras de  thread");
    }
    close(socket_memoria);
    return tcb_th;

}
//TODO: deberiamos tener un mutext para cada proceso.Aca modifico su estuctura
void mutex_create(char* nombre_mutex,int pid_mutex){
    t_mutex* mutex_nuevo=malloc(sizeof(t_mutex));
    mutex_nuevo->recurso=nombre_mutex;
    mutex_nuevo->tid_asignado=-1;
    mutex_nuevo->estado=SIN_ASIGNAR;//sin ASIGNAR
    mutex_nuevo->lista_threads_bloquedos=list_create();
    sem_wait(&(semaforos->mutex_lista_global_procesos));
    t_pcb* pcb=NULL;
    pcb=buscar_proceso_por(pid_mutex);
    sem_post(&(semaforos->mutex_lista_global_procesos));

    list_add(pcb->lista_mutex,mutex_nuevo);

}

