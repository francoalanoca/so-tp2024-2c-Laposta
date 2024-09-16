#include <../include/init_kernel.h>

void iniciar_hilo_manejo_bloqueados(){
    pthread_create(&(hilos->hilo_bloqueados), NULL, manejar_bloqueados, NULL);
    pthread_detach(hilos->hilo_fifo);
    log_info(logger_kernel, "Hilo de Planificador FIFO creado correctamente");
}
void *manejar_bloqueados(){
    while (1)
    {
        if (!list_is_empty(interfaz_io->threads_en_espera))
        {
            if(!interfaz_io->en_ejecucion){
                
            sem_wait(&(semaforos->mutex_interfaz_io));
                asignar_interfaz((t_tcb*)list_remove(interfaz_io->threads_en_espera,0));
            sem_post(&(semaforos->mutex_interfaz_io));
            }
        }
       
    }
    
}
void asignar_interfaz(t_tcb *tcb){
    pthread_t hilo_interfaz;
    interfaz_io->en_ejecucion=true;
    pthread_create(&hilo_interfaz,NULL,procesar_espera_io,tcb);
    pthread_detach(hilo_interfaz);
}
//hace la io, libera la interfaz y agrega el thread a READY
void *procesar_espera_io(void * t){
    t_tcb* tcb=(t_tcb*)t;
    usleep(tcb->tiempo_de_io*1000 );
    interfaz_io->en_ejecucion=false;
    sem_wait(&(semaforos->mutex_lista_ready));
        list_add(lista_ready,tcb);
    sem_wait(&(semaforos->mutex_lista_ready));
    log_info(logger_kernel," Fin de IO: ## (<%d>:<%d>) finalizó IO y pasa a READY",tcb->pid,tcb->tid);
    sem_post(&(semaforos->contador_threads_en_ready));
}

