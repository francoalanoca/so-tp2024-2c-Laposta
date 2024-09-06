#include "../include/init_kernel.h"

t_list* lista_new;
t_list* lista_ready;
t_list* lista_exec;
t_list* lista_block;
t_list* lista_exit;


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

//READY, EXEC y BLOCKED tienen que reservar el semaforo de grado_multiprogramacion, si ya estan en uno de esos tres no cambio nada
void pasar_new_a_ready() {
    log_info(logger_kernel, "Pasando procesos de NEW a READY");
    mover_procesos(lista_new, lista_ready, &mutex_lista_new, &mutex_lista_ready, READY); 
}
void pasar_ready_a_exit() { //se usara? ver finalizar proceso
    mover_procesos(lista_ready, lista_exit, &mutex_lista_ready, &mutex_lista_exit, EXIT);
}
void pasar_new_a_exit() { //se usara? ver finalizar proceso
    mover_procesos(lista_new, lista_exit, &mutex_lista_new, &mutex_lista_exit, EXIT);
}

// void iniciar_planificador_largo_plazo()
// {
//     pthread_create(&(hilos->hilo_planif_largo_plazo), NULL, planificar_largo_plazo, NULL);
// }

// void *planificar_largo_plazo()
// {
//     // Crear mecanismo para que el hilo muera antes de que el kernel termine
//     while (true)
//     {
//         // Obtenemos procesos para pasar de NEW a READY por FIFO.
//         log_info(logger_kernel, "ESPERO proceso en NEW");
//         sem_wait(&(semaforos->sem_procesos_new));
//         log_info(logger_kernel, "EVENTO proceso en NEW");
       
//         t_pcb *pcb = (t_pcb *)cola_mutex_peek(cola_de_new);

//         // chequeamos si el pcb no es null, ya que la cola puede estar vacia si justo se finalizaron todos los procesos
//         if (pcb == NULL) {
            
//             continue;
//         }else{
//             int socket_memoria=conectar_a_memoria();
//             enviar_solicitud_espacio_a_memoria(pcb,socket_memoria);
//             int respuesta=recibir_resp_de_memoria_a_solicitud(socket_memoria);
//             if(respuesta==OK){
//                  cola_mutex_pop(cola_de_new);
//                  agregar_proceso_a_ready(pcb);
                 
//             }else{
//                 //el proceso continua en new hasta que se elimine otro proceso(EXIT)
//                  sem_post(&(semaforos->sem_espacio_liberado_por_proceso));
//             }
           
//         }
      
//     }
// }

