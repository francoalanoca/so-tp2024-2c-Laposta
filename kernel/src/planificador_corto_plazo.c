#include "../include/init_kernel.h"

char* planificador_actual;


void inicializar_hilos_planificador(){
    planificador_actual = config_kernel->algoritmo_planif; //Revisar esto
     if(strcmp(planificador_actual,"FIFO") == 0){
        log_info(logger_kernel,"Planificador FIFO seleccionado");
        crear_hilo_planificador_fifo();
    }
    else if(strcmp(planificador_actual,"PRIORIDADES") == 0){
        log_info(logger_kernel,"Planificador por Prioridades seleccionado");
        crear_hilo_planificador_prioridades();
    }else if(strcmp(planificador_actual,"COLAS_MULTINIVEL") == 0){
        log_info(logger_kernel,"Planificador por Colas Multinivel seleccionado");
        crear_hilo_planificador_colas_multinivel();
    }
    else{
        log_error(logger_kernel,"Planificador no reconocido");
    }
}
void crear_hilo_planificador_fifo(){
    pthread_create(&hilos->hilo_fifo,NULL,planificar_fifo,NULL);
    pthread_detach(hilos->hilo_fifo);
    log_info(logger_kernel,"Hilo de Planificador FIFO creado correctamente");
}
void crear_hilo_planificador_prioridades(){
    pthread_create(&hilos->hilo_prioridades,NULL,planificar_prioridades,NULL);
    pthread_detach(hilos->hilo_prioridades);
    log_info(logger_kernel,"Hilo de Planificador por Prioridades creado correctamente");
}

void crear_hilo_planificador_colas_multinivel(){
    pthread_create(&hilos->hilo_colas_multinivel,NULL,planificar_colas_multinivel,NULL);
    pthread_detach(hilos->hilo_colas_multinivel);
    log_info(logger_kernel,"Hilo de Planificador por Colas Multinivel creado correctamente");
}

void planificar_fifo(){
    while(1){
        //RELLENAR
    }
}

void planificar_prioridades(){
    while(1){
        //RELLENAR
    }
}

void planificar_colas_multinivel(){
    while(1){
        //RELLENAR
    }
}

void pasar_ready_a_execute() {
    mover_procesos(lista_ready, lista_exec, &mutex_lista_ready, &mutex_lista_exec, EXEC);
}
void pasar_execute_a_ready() {
    mover_procesos(lista_exec, lista_ready, &mutex_lista_exec, &mutex_lista_ready, READY);
}
void pasar_blocked_a_exit() {
    mover_procesos(lista_blocked, lista_exit, &mutex_lista_blocked, &mutex_lista_exit, EXIT);
}
void pasar_blocked_a_ready() {
    mover_procesos(lista_blocked, lista_ready, &mutex_lista_blocked, &mutex_lista_ready, READY);
}
void pasar_execute_a_exit() {
    mover_procesos(lista_exec, lista_exit, &mutex_lista_exec, &mutex_lista_exit, EXIT);
}
void pasar_execute_a_blocked() {
    mover_procesos(lista_exec, lista_blocked, &mutex_lista_exec, &mutex_lista_blocked, BLOCKED);
}