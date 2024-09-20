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
    nuevo_tcb->tiempo_de_io=0;
    nuevo_tcb->mutex_asignados=list_create();
    nuevo_tcb->thread_target=NULL;
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
//TODO: revisar semaforos
t_pcb* buscar_proceso_por(int pid_buscado){
    t_pcb* un_pcb=NULL;
    
    for(int i=0;i<list_size(lista_procesos_global);i++){
        un_pcb=(t_pcb*)list_get(lista_procesos_global,i);
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


//TODO: revisar semaforos
//busca el proceso en la lista global de procesos y dentro del proceso el mutex 
t_mutex* buscar_mutex(char* recurso,int pid){
    t_mutex *mutex=NULL;
    sem_wait(&(semaforos->mutex_lista_global_procesos));
    t_pcb *pcb=buscar_proceso_por(pid);
    sem_post(&(semaforos->mutex_lista_global_procesos));
    for (int i = 0; i < list_size(pcb->lista_mutex ); i++)
    {
        t_mutex* mutex_aux=(t_mutex*)list_get(pcb->lista_mutex,i);
        if(strcmp(recurso,mutex_aux->recurso)==0){
            mutex=(t_mutex*)list_get(pcb->lista_mutex,i);
            break;
        }
        
    }
    return mutex;
    
}
void asignar_mutex(t_tcb * tcb, t_mutex* mutex){
        mutex->estado=ASIGNADO;
        mutex->thread_asignado=tcb;
        list_add(tcb->mutex_asignados,tcb);
}


void* enviar_a_memoria_thread_saliente(void* t){
    t_tcb* tcb=(t_tcb*)t;
    t_paquete *paquete=crear_paquete(FINALIZAR_HILO);
    agregar_a_paquete(paquete,&(tcb->pid),sizeof(int));
    agregar_a_paquete(paquete,&(tcb->tid),sizeof(int));
    int fd_memoria=conectar_a_memoria();
    enviar_paquete(paquete,fd_memoria);
   
    
    //espero respuesta de memoria;

    int rta=recibir_resp_de_memoria_a_solicitud(fd_memoria);
    if(rta==FINALIZAR_HILO_RTA_OK){
        log_info(logger_kernel, "## (<%d>:<%d>) Finaliza el hilo",tcb->pid,tcb->tid);
    }
    else
         log_info(logger_kernel, "## (<%d>:<%d>) MEMORIA no logro Finalizar el hilo",tcb->pid,tcb->tid);
    close(fd_memoria); 
    eliminar_paquete(paquete);
}

void destruir_tcb(t_tcb* tcb){
    if (tcb == NULL) {
        return; 
    }
    // Liberar la memoria reservada por el TCB
    free(tcb);
}
//TODO: uso semaforos??

//devuelve el indice del tid si existe o -1 si no
int buscar_indice_de_tid_en_proceso(t_pcb *pcb,int tid){
    int posicion=-1;
   for(int i=0;i<list_size(pcb->lista_tids);i++){
        bool pcb_tiene_tid=*((int *)list_get(pcb->lista_tids,i))==tid;
        if(pcb_tiene_tid){
            posicion=i;
            return posicion;
        }
    }
    return posicion;
}
//TODO: uso semaforos??
//elimina el tid(si existe) del pcb
 bool quitar_tid_de_proceso(t_tcb* tcb_saliente){
    bool exito_eliminando_de_pcb=false;
    t_pcb *pcb=buscar_proceso_por(tcb_saliente->pid);
    int posicion_a_eliminar=buscar_indice_de_tid_en_proceso(pcb,tcb_saliente->tid);
    if(posicion_a_eliminar!=-1){
        list_remove(pcb->lista_tids,posicion_a_eliminar);
        exito_eliminando_de_pcb=true;
    }
        
    return exito_eliminando_de_pcb;
 }
//busca un tcb en una lista por su tid y pid y lo remueve
t_tcb* buscar_en_lista_y_cancelar(t_list* lista,int tid,int pid,sem_t* sem){
    sem_wait(sem);
    for(int i=0;i<list_size(lista);i++){
        t_tcb* tcb=(t_tcb*)list_get(lista,i);
        if(tcb->tid==tid && tcb->pid==pid){
            list_remove(lista,i);
            sem_post(sem);
            return tcb;
        }
    }
    sem_post(sem);
    return NULL;
}
t_tcb* buscar_en_lista_tcb(t_list* lista,int tid,int pid,sem_t* sem){
    sem_wait(sem);
    for(int i=0;i<list_size(lista);i++){
        t_tcb* tcb=(t_tcb*)list_get(lista,i);
        if(tcb->tid==tid && tcb->pid==pid){
            sem_post(sem);
            return tcb;
        }
    }
    sem_post(sem);
    return NULL;
}
t_mutex* quitar_mutex_a_thread(char* recurso,t_tcb* tcb){
    t_mutex *mutex=NULL;
    for(int i=0;i<list_size(tcb->mutex_asignados);i++){
         mutex=(t_mutex*)list_get(tcb->mutex_asignados,i);
        if(strcmp(recurso,mutex->recurso)==0){
            list_remove(tcb->mutex_asignados,1);
            return mutex;
        }
    }
    return mutex;
}
t_tcb* asignar_mutex_al_siguiente_thread(t_mutex* mutex){
    t_tcb* tcb=NULL;
    if(list_size(mutex->lista_threads_bloquedos)){
   
    tcb=(t_tcb*)list_remove(mutex->lista_threads_bloquedos,0);
    
        list_add(tcb->mutex_asignados,mutex);
        mutex->thread_asignado=tcb;
    }
    return tcb;
}


void interfaz_io(){
    int io_en_ejecucion = 0;
    while(1){
        sem_wait (&(semaforos->sem_io_solicitud));

        if (io_en_ejecucion == 0){
            io_en_ejecucion = 1;
            sem_wait(&(semaforos->sem_io_en_uso)); //IO EN USO

            sem_wait(&(semaforos->mutex_lista_exec));
            t_tcb* tcb_usando_io = list_remove (lista_exec,0);
            sem_post(&(semaforos->mutex_lista_exec));

            sem_post(&(semaforos->sem_sleep_io));


        }else if(io_en_ejecucion == 1){
            sem_wait(&(semaforos->mutex_lista_espera_io));
            t_tcb* tcb_espera = list_remove (lista_espera_io,0);
            sem_post(&(semaforos->mutex_lista_espera_io));

            agregar_a_lista(tcb_espera,lista_exec,&semaforos->mutex_lista_exec);

            sem_post(&(semaforos->sem_io_solicitud));
        }

        sem_wait(&(semaforos->sem_io_en_uso));
        t_tcb* tcb_espera = list_remove (lista_espera_io,0);

    }
}

void verificacion_sleep_io(){
    while(1){

        }
}

void* hilo_sleep_io(int tiempo){
    while(1){
        sem_wait(&(semaforos->sem_sleep_io));
        sleep(tiempo);
        sem_post(&(semaforos->sem_sleep_io));
    }
}