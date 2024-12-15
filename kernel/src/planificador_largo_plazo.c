#include "../include/init_kernel.h"

//No se si esto esta bien aca
t_list* lista_new;
t_list* lista_ready;
t_list* lista_exec;
t_list* lista_blocked;
t_list* lista_exit;
t_list* lista_procesos_global;
t_list* lista_espera_io;
void inicializar_listas() {
    lista_new = list_create();
    lista_ready = list_create();
    lista_exec = list_create();
    lista_exit = list_create();
    lista_blocked = list_create();
    lista_procesos_global=list_create();
    lista_espera_io = list_create();
}
void inicializar_hilos_largo_plazo(){
    pthread_create(&(hilos->hilo_planif_largo_plazo),NULL,planificar_procesos,NULL);
    pthread_create(&(hilos->hilo_finalizacion_procesos_memoria),NULL,manejo_liberacion_memoria,NULL);

}

void* planificar_procesos() {
    while (1) {
        // Esperamos que un proceso sea agregado a la lista NEW
        sem_wait(&(semaforos->sem_procesos_new));

        t_pcb* un_pcb = NULL;
        int indice_pcb_elegido = -1;

        // Bloqueamos acceso a la lista NEW para buscar un PCB elegible
        sem_wait(&(semaforos->mutex_lista_new));

        for (int i = 0; i < list_size(lista_new); i++) {
            t_pcb* pcb_actual = list_get(lista_new, i);
            if (pcb_actual != NULL) {
                int socket_memoria = conectar_a_memoria();
                enviar_solicitud_espacio_a_memoria(pcb_actual, socket_memoria);
                int respuesta = recibir_resp_de_memoria_a_solicitud(socket_memoria);
                close(socket_memoria);

                if (respuesta == INICIAR_PROCESO_RTA_OK) {
                    un_pcb = pcb_actual; // Encontramos un PCB que puede ser iniciado
                    indice_pcb_elegido = i;
                    break; // Salimos del bucle
                } else {
                    log_info(logger_kernel, 
                             "El PCB con PID:%d no puede iniciar, esperando espacio en memoria.", 
                             pcb_actual->pid);
                }
            }
        }

        sem_post(&(semaforos->mutex_lista_new)); // Liberamos el mutex de la lista NEW

        // Si no se encontró un proceso elegible, esperamos la liberación de memoria
        if (un_pcb == NULL) {
            log_info(logger_kernel, 
                     "No se encontró ningún proceso elegible en la lista NEW. Esperando recursos.");
            continue; // Vuelve al inicio del bucle, esperando un nuevo semáforo
        }

        // Crear el thread principal del proceso y mover el TCB a READY
        log_info(logger_kernel, "Recibí OK para crear el proceso con PID:%d", un_pcb->pid);
        t_tcb* tcb = thread_create(un_pcb->ruta_pseudocodigo, un_pcb->prioridad_th_main, un_pcb->pid);

        // Removemos el PCB de la lista NEW y agregamos el TCB a la lista READY
        remover_de_lista(lista_new, indice_pcb_elegido, &(semaforos->mutex_lista_new));
        agregar_a_lista(tcb, lista_ready, &(semaforos->mutex_lista_ready));

        log_info(logger_kernel, "Nuevo proceso con PID:%d y TID:%d movido a READY.", tcb->pid, tcb->tid);

        // Notificamos al planificador de corto plazo que hay un thread en READY
        sem_post(&(semaforos->contador_threads_en_ready));
    }

    return NULL;
}


void manejo_liberacion_memoria(){
    while(1){
            sem_wait(&(semaforos->sem_espacio_liberado_por_proceso));
        if(list_is_empty(lista_new)){
            log_info(logger_kernel,"No hay procesos en memoria para liberar \n");
        }else{
            log_info(logger_kernel,"Se libero espacio en memoria");
            sem_post(&(semaforos->sem_procesos_new));
        }
    }
}


void mover_procesos(t_list* lista_origen, t_list* lista_destino, sem_t* sem_origen, sem_t* sem_destino, t_estado nuevo_estado) {
    if (!list_is_empty(lista_origen)) {
        sem_wait(sem_origen);
        t_tcb* tcb = (t_tcb*)list_remove(lista_origen, 0);
        sem_post(sem_origen);
        if (nuevo_estado == EXIT){
            log_info(logger_kernel, "Hilo con TID %d y PID %d movido a la lista EXIT", tcb->tid, tcb->pid);
            //aca deberia mandar a memoria la eliminacion del hilo
            pthread_t hilo_manejo_exit;
            pthread_create(&hilo_manejo_exit,NULL,enviar_a_memoria_proceso_saliente,(void*)tcb);
            pthread_detach(hilo_manejo_exit);
            
        }else{
            sem_wait(sem_destino);
            tcb->estado = nuevo_estado;
            list_add(lista_destino, tcb);
            sem_post(sem_destino);

        if (nuevo_estado == NEW) {
            log_info(logger_kernel, "Hilo con TID %d y PID %d movido a la lista NEW", tcb->tid, tcb->pid);
        }
        else if(nuevo_estado == READY){
            log_info(logger_kernel, "Hilo con TID %d y PID %d movido a la lista READY", tcb->tid, tcb->pid);
        }
        else if(nuevo_estado == EXEC){
            log_info(logger_kernel, "Hilo con TID %d y PID %d movido a la lista EXEC", tcb->tid, tcb->pid);
        }
        else if(nuevo_estado == BLOCKED){
            log_info(logger_kernel, "Hilo con TID %d y PID %d movido a la lista BLOCKED", tcb->tid, tcb->pid);
        }else 
            log_info(logger_kernel, "No hay hilos en la lista origen");
        }
        }
        
}


void agregar_a_cola(t_pcb *pcb,t_list* lista,sem_t* sem){
    sem_wait(sem);
    list_add(lista,pcb);
    sem_post(sem);
}
void pasar_new_a_ready() {
    mover_procesos(lista_new, lista_ready, &(semaforos->mutex_lista_new), &(semaforos->mutex_lista_ready), READY); 
}
void pasar_ready_a_exit() { //se usara? ver finalizar proceso
    mover_procesos(lista_ready, lista_exit, &(semaforos->mutex_lista_ready), &(semaforos->mutex_lista_exit), EXIT);
}
void pasar_new_a_exit() { //se usara? ver finalizar proceso
    mover_procesos(lista_new, lista_exit, &(semaforos->mutex_lista_new), &(semaforos->mutex_lista_exit), EXIT);
}
void agregar_a_lista(t_tcb *tcb,t_list* lista,sem_t* sem){
    sem_wait(sem);
    list_add(lista,tcb);
    sem_post(sem);
}
t_tcb* remover_de_lista(t_list* lista,int indice, sem_t* mutex){
    t_tcb* tcb=NULL;
    sem_wait(mutex);
    tcb=(t_tcb*)list_remove(lista,indice);
    sem_post(mutex);
    return tcb;
}

