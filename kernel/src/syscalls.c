#include <../include/init_kernel.h>
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
        log_info(logger_kernel,"memoria creo el hilo");
        close(socket_memoria);
        return tcb_th;
    }
    close(socket_memoria);
    return NULL;
}
//TODO: deberiamos tener un mutext para cada proceso.Aca modifico su estuctura
void mutex_create(char* nombre_mutex,int pid_mutex){
    t_mutex* mutex_nuevo=malloc(sizeof(t_mutex));
    mutex_nuevo->recurso=nombre_mutex;
    mutex_nuevo->thread_asignado=NULL;
    mutex_nuevo->estado=SIN_ASIGNAR;//sin ASIGNAR
    mutex_nuevo->lista_threads_bloquedos=list_create();
    sem_wait(&(semaforos->mutex_lista_global_procesos));
    t_pcb* pcb=NULL;
    pcb=buscar_proceso_por(pid_mutex);
    sem_post(&(semaforos->mutex_lista_global_procesos));

    list_add(pcb->lista_mutex,mutex_nuevo);

}
//coloca el thread en la cola de espera de IO
void ejecutar_io(int tiempo){
    //quito de el thread de exec
    sem_wait(&(semaforos->mutex_lista_exec));
    t_tcb* tcb=list_get(lista_exec,0);
    sem_post(&(semaforos->mutex_lista_exec));
    tcb ->tiempo_de_io=tiempo;

    pasar_execute_a_blocked();
    //agrego a io
    agregar_a_lista(tcb,lista_espera_io,&(semaforos->mutex_lista_espera_io));
    log_info(logger_kernel,"## (<%d>:<%d>)- Bloqueado por: <IO>",tcb->pid,tcb->tid);
    sem_post(&(semaforos->sem_io_solicitud));
    
}

//TODO: revisar semaforos 
void mutex_lock(char* recurso){
    t_mutex* mutex=NULL;
    sem_wait(&(semaforos->mutex_lista_exec));
    t_tcb* tcb_ejecutando=(t_tcb*)list_get(lista_exec,0);
    sem_post(&(semaforos->mutex_lista_exec));

    log_warning(logger_kernel,"buscando mutex para lockear ");  
    mutex=buscar_mutex(recurso,tcb_ejecutando->pid);
    if(mutex!=NULL){
        if(mutex->estado==SIN_ASIGNAR){
            log_warning(logger_kernel,"lockeando mutex: %s",mutex->recurso);
            asignar_mutex(tcb_ejecutando,mutex);
            //continua ejecutando el mismo tcb-->vuelvo a  enviar el mismo tcb a ejecutar
            enviar_thread_a_cpu(tcb_ejecutando,config_kernel->conexion_cpu_dispatch);
        }else{
            //se bloquea--> quito el tcb de exec y lo mando a espera de mutex y bloq
            // y marco la cpu como libre
            remover_de_lista(lista_exec,0,&(semaforos->mutex_lista_exec));
            
            list_add(mutex->lista_threads_bloquedos,tcb_ejecutando);
            
            agregar_a_lista(tcb_ejecutando,lista_blocked,&(semaforos->mutex_lista_blocked));
            
            sem_post(&(semaforos->espacio_en_cpu));

        }   
    }else{//si no existe el mutex->mando a exit el tcb que hizo el lock y activo el planificador
            remover_de_lista(lista_exec,0,&(semaforos->mutex_lista_exec));
            agregar_a_lista(tcb_ejecutando,lista_exit,&(semaforos->mutex_lista_exit));
            sem_post(&(semaforos->espacio_en_cpu));
    }
}

//pasa el mutex al siguiente tcb en espera o no hace nada si el mutex no existe
void mutex_unlock(char* recurso, t_tcb* tcb){
    //controlo que el mutex exista y este asignado al tcb
    t_mutex* mutex_a_desbloquear=NULL;
    t_mutex* mutex_existe=buscar_mutex(recurso,tcb->pid);
    if(mutex_existe!=NULL){
        mutex_a_desbloquear=quitar_mutex_a_thread(recurso,tcb);
            
        if(mutex_a_desbloquear!=NULL){//si lo tenia asignado
        //asgino al primero que esperaba el mutex
            t_tcb* tcb_con_mutex=asignar_mutex_al_siguiente_thread(mutex_a_desbloquear);
        //desbloqueo tcb_con_mutex porque ya se le asigno el mutex 
            buscar_en_lista_y_cancelar(lista_blocked,tcb_con_mutex->tid,tcb_con_mutex->pid,&(semaforos->mutex_lista_blocked));
        //envio el tcb con mutex a ready
             agregar_a_lista(tcb,lista_ready,&(semaforos->mutex_lista_ready));
             sem_post(&(semaforos->contador_threads_en_ready));
        
        }
            //continua ejecutando el que hizo la syscall
            enviar_thread_a_cpu(tcb,config_kernel->conexion_cpu_dispatch);
    }else{ //no existe mutex-> mando hilo a exit y activo el planificador
            remover_de_lista(lista_exec,0,&(semaforos->mutex_lista_exec));
            agregar_a_lista(tcb,lista_exit,&(semaforos->mutex_lista_exit));
            sem_post(&(semaforos->espacio_en_cpu));
        }

}

// terminar thread: quitar TID de pcb, enviar mensaje a memoria, mandar a ready los tcb joineados al thread eliminado, destruir tcb
void thread_exit(t_tcb *tcb){
    bool se_logro_eliminar=quitar_tid_de_proceso(tcb);
    if(se_logro_eliminar){
    pthread_t hilo_manejo_exit;

    desbloquear_hilos_por_fin_de_hilo(tcb);
    
    //liberar memoria
    pthread_create(&hilo_manejo_exit,NULL,enviar_a_memoria_thread_saliente,(void*)tcb);
    pthread_detach(hilo_manejo_exit);
    
    }else
        log_info(logger_kernel, " EL hilo no existe o ya fue eliminado del proceso");
}

void thread_cancel(int tid_a_cancelar,int pid)
{
    t_pcb* pcb=buscar_proceso_por(pid);
    int indice_de_tid=buscar_indice_de_tid_en_proceso(pcb,tid_a_cancelar);

    if(indice_de_tid!=-1){//el tcb aun no se elimino
        //busco donde este el pcb}
        t_tcb* tcb_a_cancelar=NULL;
        if(tcb_a_cancelar==NULL){//si se quiere cancelar el mimo hilo, aunque DIJERON QUE ESTE CASO NO LO EVALUAN
            tcb_a_cancelar=buscar_en_lista_y_cancelar(lista_exec,tid_a_cancelar,pid,&(semaforos->mutex_lista_exec));
        }
        if(tcb_a_cancelar==NULL){
            tcb_a_cancelar=buscar_en_lista_y_cancelar(lista_blocked,tid_a_cancelar,pid,&(semaforos->mutex_lista_blocked));
        }
           if(tcb_a_cancelar==NULL){
            tcb_a_cancelar=buscar_en_lista_y_cancelar(lista_ready,tid_a_cancelar,pid,&(semaforos->mutex_lista_ready));
        }
         if(tcb_a_cancelar!=NULL) {//muevo a exit
            agregar_a_lista(tcb_a_cancelar,lista_exit,&(semaforos->mutex_lista_exit));
            thread_exit(tcb_a_cancelar);
            
            }
    }
 //hace nada
}
//pone a tcb en bloqueados y esperar
void thread_join(t_tcb* tcb_en_exec, int tid_target){
    t_pcb* pcb=buscar_proceso_por(tcb_en_exec->pid);
    log_warning(logger_kernel,"buscando pcb target para thread_join");
    mostrar_pcb(pcb,logger_kernel);
    
    int posicion_tid=buscar_indice_de_tid_en_proceso(pcb,tid_target);
    
    if(posicion_tid !=-1){//existe tid_target--> busco su tcb en colas
            t_tcb* tcb_target=NULL;
        if(tcb_target==NULL){
            tcb_target=(t_tcb*)buscar_en_lista_tcb(lista_ready,tid_target,pcb->pid,&(semaforos->mutex_lista_ready));
        }
         if(tcb_target==NULL){
            tcb_target=(t_tcb*)buscar_en_lista_tcb(lista_blocked,tid_target,pcb->pid,&(semaforos->mutex_lista_blocked));
        }
         if(tcb_target!=NULL) { 
            tcb_en_exec->thread_target=tcb_target;
            remover_de_lista(lista_exec,0,&(semaforos->mutex_lista_exec));
            agregar_a_lista(tcb_en_exec,lista_blocked,&(semaforos->mutex_lista_blocked));
            sem_post(&(semaforos->espacio_en_cpu));
            
            }
    }else    //continua ejecutando el que hizo la syscall
        enviar_thread_a_cpu(tcb_en_exec,config_kernel->conexion_cpu_dispatch);
}
