#include "../include/init_kernel.h"

void iniciar_planificador_largo_plazo()
{
    pthread_create(&(hilos->hilo_planif_largo_plazo), NULL, planificar_largo_plazo, NULL);
}

void *planificar_largo_plazo()
{
    // Crear mecanismo para que el hilo muera antes de que el kernel termine
    while (true)
    {
        // Obtenemos procesos para pasar de NEW a READY por FIFO.
        log_info(logger_kernel, "ESPERO proceso en NEW");
        sem_wait(&(semaforos->sem_procesos_new));
        log_info(logger_kernel, "EVENTO proceso en NEW");
       
        t_pcb *pcb = (t_pcb *)cola_mutex_peek(cola_de_new);

        // chequeamos si el pcb no es null, ya que la cola puede estar vacia si justo se finalizaron todos los procesos
        if (pcb == NULL) {
            
            continue;
        }else{
            int socket_memoria=conectar_a_memoria();
            enviar_solicitud_espacio_a_memoria(pcb,socket_memoria);
            int respuesta=recibir_resp_de_memoria_a_solicitud(socket_memoria);
            if(respuesta==OK){
                 cola_mutex_pop(cola_de_new);
                 agregar_proceso_a_ready(pcb);
                 
            }else{
                //el proceso continua en new hasta que se elimine otro proceso(EXIT)
                 sem_post(&(semaforos->sem_espacio_liberado_por_proceso));
            }
           
        }
      
    }
}

