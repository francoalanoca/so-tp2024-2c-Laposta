#include "../include/init_kernel.h"

//No se si esto esta bien aca
t_list* lista_new;
t_list* lista_ready;
t_list* lista_exec;
t_list* lista_blocked;
t_list* lista_exit;
t_list* lista_procesos_global;
void inicializar_listas() {
    lista_new = list_create();
    lista_ready = list_create();
    lista_exec = list_create();
    lista_exit = list_create();
    lista_blocked = list_create();
    lista_procesos_global=list_create();
}
void  inicializar_hilos_largo_plazo(){
    pthread_create(&(hilos->hilo_planif_largo_plazo),NULL,planificar_procesos,NULL);

}

void* planificar_procesos(){
    
    while (1) {
              log_info(logger_kernel, "Iniciando planificador de largo plazo");
        sem_wait(&(semaforos->sem_procesos_new));//se ingreso un proceso
      

        sem_wait(&(semaforos->mutex_lista_new));
        t_pcb* un_pcb=NULL;
        un_pcb=list_get(lista_new,0);
        sem_post(&(semaforos->mutex_lista_new));
                    // chequeamos si el pcb no es null
         log_info(logger_kernel, "pcb obtenido de NEW");

        if (un_pcb == NULL) {
            
            continue;
        }else{
            int socket_memoria=conectar_a_memoria();
            enviar_solicitud_espacio_a_memoria(un_pcb,socket_memoria);
          
            int respuesta=recibir_resp_de_memoria_a_solicitud(socket_memoria);
            close(socket_memoria);
            if(respuesta==INICIAR_PROCESO_RTA_OK){
                log_info(logger_kernel,"recibi ok para crear proceso");
               t_tcb* tcb=NULL;
               tcb=thread_create(un_pcb->ruta_pseudocodigo,un_pcb->prioridad_th_main ,un_pcb->pid);//creo el thread main y lo envio a ready 
                //FIXME: REMUEVO el pcb de new por fifo, el pcb aun esta en lista_global_procesos
                 sem_wait(&(semaforos->mutex_lista_new));
                    list_remove(lista_new,0);
                 sem_post(&(semaforos->mutex_lista_new));
                 //agrego thread a ready
                 sem_wait(&(semaforos->mutex_lista_ready));
                    list_add(lista_ready,tcb);
                 sem_post(&(semaforos->mutex_lista_ready));
                //Le avisamos a planif_corto_plazo que tiene un thread en ready
                sem_post(&(semaforos->contador_threads_en_ready));
            }else{
                log_info(logger_kernel, "esperando liberacion de memoria \n");
                //el proceso continua en new hasta que se elimine otro proceso(EXIT)
                 sem_wait(&(semaforos->sem_espacio_liberado_por_proceso));
            }
           
          
        }
    }
    log_info(logger_kernel, "No hay procesos en la cola NEW");
    return NULL; 
}


void mover_procesos(t_list* lista_origen, t_list* lista_destino, sem_t* sem_origen, sem_t* sem_destino, t_estado nuevo_estado) {
    if (!list_is_empty(lista_origen)) {
        sem_wait(sem_origen);
        t_pcb* pcb = list_remove(lista_origen, 0); 
        sem_post(sem_origen);
        sem_wait(sem_destino);
        pcb->estado = nuevo_estado;
        list_add(lista_destino, pcb);
        sem_post(sem_destino);
        if (nuevo_estado == NEW) {
            log_trace(logger_kernel, "Proceso con PID %d movido a la lista NEW", pcb->pid);
        }
        else if(nuevo_estado == READY){
            log_trace(logger_kernel, "Proceso con PID %d movido a la lista READY", pcb->pid);
        }
        else if(nuevo_estado == EXEC){
            log_trace(logger_kernel, "Proceso con PID %d movido a la lista EXEC", pcb->pid);
        }
        else if(nuevo_estado == BLOCKED){
            log_trace(logger_kernel, "Proceso con PID %d movido a la lista BLOCKED", pcb->pid);
        }
        else if(nuevo_estado == EXIT){
            log_trace(logger_kernel, "Proceso con PID %d movido a la lista EXIT", pcb->pid);
        }
    }else 
        log_info(logger_kernel, "No hay procesos en la lista origen");
}


void agregar_a_cola(t_pcb *pcb,t_list* lista,sem_t* sem){
    list_add(lista,pcb);
}
void pasar_new_a_ready() {
    log_info(logger_kernel, "Pasando thread de NEW a READY");
    mover_procesos(lista_new, lista_ready, &(semaforos->mutex_lista_new), &(semaforos->mutex_lista_ready), READY); 
}
void pasar_ready_a_exit() { //se usara? ver finalizar proceso
    mover_procesos(lista_ready, lista_exit, &(semaforos->mutex_lista_ready), &(semaforos->mutex_lista_exit), EXIT);
}
void pasar_new_a_exit() { //se usara? ver finalizar proceso
    mover_procesos(lista_new, lista_exit, &(semaforos->mutex_lista_new), &(semaforos->mutex_lista_exit), EXIT);
}

