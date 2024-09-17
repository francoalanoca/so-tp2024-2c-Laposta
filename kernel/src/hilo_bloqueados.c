#include <../include/init_kernel.h>

void iniciar_hilo_manejo_bloqueados(){
    pthread_create(&(hilos->hilo_bloqueados), NULL, manejar_bloqueados, NULL);
    pthread_detach(hilos->hilo_fifo);
    log_info(logger_kernel, "Hilo de manejador de bloqueados por IO");
}
void *manejar_bloqueados(){

        while (1) {
        sem_wait(&(semaforos->contador_tcb_en_io)); // Espera a que haya procesos en la cola de IO
        sem_wait(&(semaforos->mutex_interfaz_io)); // Bloquea el acceso a la cola IO
        
        if (!list_is_empty(interfaz_io->threads_en_espera)) {
            // Obtiene el primer proceso en la cola de IO
            t_tcb* tcb = (t_tcb*)list_remove(interfaz_io->threads_en_espera, 0);
            sem_post(&(semaforos->mutex_interfaz_io));

            // Simula el tiempo de IO que debe esperar el proceso
            log_info(logger_kernel,"PID:%d TID:%d en IO por %d mseg...\n",tcb->pid, tcb->tid, tcb->tiempo_de_io);
            usleep(tcb->tiempo_de_io);

            // Enviar el proceso a la cola ready después de terminar la IO
            sem_wait(&(semaforos->mutex_lista_ready));  // Protege la cola de ready
            list_add(lista_ready, tcb);
            sem_post(&(semaforos->mutex_lista_ready)); // Libera el acceso a la cola de ready
            sem_post(&(semaforos->contador_threads_en_ready));
         log_info(logger_kernel," Fin de IO: ## (<%d>:<%d>) finalizó IO y pasa a READY",tcb->pid,tcb->tid);
        } else {
            sem_post(&(semaforos->mutex_interfaz_io));// Libera el mutex si la cola estaba vacía
        }
    }
    
}
