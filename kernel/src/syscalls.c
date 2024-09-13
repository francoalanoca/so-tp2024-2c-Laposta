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
    free(archivo_instrucciones);
    //TCB-main
    añadir_tid_a_proceso(pcb);

    return pcb;
}
void añadir_tid_a_proceso(t_pcb* pcb){
    list_add(pcb->lista_tids,&(pcb->contador_AI_tids));
    pcb->contador_AI_tids++;
}

void enviar_solicitud_espacio_a_memoria(void* pcb_solicitante,int socket){
    t_pcb* pcb=(t_pcb*) pcb_solicitante;
    //memoria solo necesita tamanio de proceso y el pid 
    t_paquete* paquete_a_enviar=crear_paquete(INICIAR_PROCESO);
    agregar_a_paquete(paquete_a_enviar,&(pcb->tamanio_proceso),sizeof(int));
    agregar_a_paquete(paquete_a_enviar,&(pcb->pid),sizeof(int));
    enviar_paquete(paquete_a_enviar,socket);
}
int recibir_resp_de_memoria_a_solicitud(int socket_memoria){
   int respuesta=recibir_operacion(socket_memoria); 
   return respuesta;
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
    agregar_a_paquete(paquete_a_enviar,&longitud,sizeof(int));
    agregar_a_paquete(paquete_a_enviar,pseudo,longitud);
    agregar_a_paquete(paquete_a_enviar,&(tcb_nuevo->tid),sizeof(int));
    agregar_a_paquete(paquete_a_enviar,&(tcb_nuevo->pid),sizeof(int));
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
//******************      SYSCALLS         *******************************
void process_create(char* ruta_instrucciones,int tam_proceso,int prioridad_hilo_main){
    log_info(logger_kernel, "Crear proceso: %s",ruta_instrucciones);

    t_pcb* pcb_nuevo=NULL;
    pcb_nuevo=crear_pcb(tam_proceso,ruta_instrucciones, prioridad_hilo_main);

    sem_wait(&(semaforos->mutex_lista_global_procesos));
        list_add(lista_procesos_global,pcb_nuevo);
    sem_post(&(semaforos->mutex_lista_global_procesos));

    sem_wait(&(semaforos->mutex_lista_new));
         list_add(lista_new,pcb_nuevo);
    sem_post(&(semaforos->mutex_lista_new));
    sem_post(&(semaforos->sem_procesos_new));

}

t_tcb* thread_create(char* pseudo_codigo,int prioridad_th,int pid){
    t_tcb* tcb_th=crear_tcb(prioridad_th, pid);
    int socket_memoria=conectar_a_memoria();
    enviar_a_memoria_creacion_thread( tcb_th,pseudo_codigo, socket_memoria);
    int rta_memoria=recibir_resp_de_memoria_a_solicitud(socket_memoria);
    if(rta_memoria==OK){
        log_info(logger_kernel,"memoria cargo las estructuras de  thread");
    }
    return tcb_th;

}

