#include <../include/init_kernel.h>
t_pcb* crear_pcb(int tam_proceso,char* archivo_instrucciones) {
    t_pcb* pcb = malloc(sizeof(t_pcb));
    pcb->contador_AI_tids=0;//inicializa el contador de tids del proceso
    pcb->lista_mutex=list_create();
    pcb->lista_tids=list_create();
    pcb->tamanio_proceso=tam_proceso;
    pcb->pid=pid_AI_global;
    pid_AI_global++;
    pcb->ruta_pseudocodigo=strdup(archivo_instrucciones);
    free(archivo_instrucciones);
    //TCB-main
    añadir_tid_a_proceso(pcb);

    return pcb;
}
void añadir_tid_a_proceso(t_pcb* pcb){
    list_add(pcb->lista_tids,&(pcb->contador_AI_tids));
    //TODO: deberia crear el tcb aca??????
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
//******************      SYSCALLS         *******************************
void process_create(char* ruta_instrucciones,int tam_proceso,int prioridad_hilo_main){
    log_info(logger_kernel, "Crear proceso: %s",ruta_instrucciones);

    t_pcb* pcb_nuevo;
    pcb_nuevo=crear_pcb(tam_proceso,ruta_instrucciones);

    agregar_proceso_a_new(pcb_nuevo);
  
}
