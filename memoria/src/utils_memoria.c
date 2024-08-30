#include "../include/utils_memoria.h"

//Editar todas segun sea conveniente

/*---------------------------- KERNEL-------------------------*/

//Memoria deserializa lo enviado de Kernel
t_m_crear_proceso* deserializar_iniciar_proceso(t_list*  lista_paquete ){

    //Creamos una variable de tipo struct que ira guardando todo del paquete y le asignamos tamaño
    t_m_crear_proceso* crear_proceso = malloc(sizeof(t_m_crear_proceso));
    
    crear_proceso->pid = *(uint32_t*)list_get(lista_paquete, 0);
    printf("Pid recibido: %d \n", crear_proceso->pid);

    printf("Nombre del proceso: %s \n", (char*) list_get(lista_paquete, 1));
    crear_proceso->archivo_pseudocodigo = (char*) list_get(lista_paquete, 1);
    
    return crear_proceso;
}


//Memoria envia proceso creado a Kernel
void enviar_respuesta_iniciar_proceso(t_m_crear_proceso* crear_proceso ,int socket_kernel) {
    t_paquete* paquete_crear_proceso;
 
    paquete_crear_proceso = crear_paquete(INICIAR_PROCESO_RTA);
 
    agregar_a_paquete(paquete_crear_proceso, &crear_proceso->pid,  sizeof(uint32_t));
     
    enviar_paquete(paquete_crear_proceso, socket_kernel);   
    printf("Proceso enviado: %i\n", crear_proceso->pid); 
    eliminar_paquete(paquete_crear_proceso);
    printf("PAQUETE ELIMINADO\n"); 
}


uint32_t deserializar_finalizar_proceso(t_list*  lista_paquete ){

    uint32_t proceso_a_finalizar;
    
    proceso_a_finalizar = *(uint32_t*)list_get(lista_paquete, 0);
    printf("Pid recibido: %d \n", proceso_a_finalizar);

    return proceso_a_finalizar;
}


void enviar_respuesta_finalizar_proceso(uint32_t pid_proceso_a_finalizar ,int socket_kernel) {
    t_paquete* paquete_finalizar_proceso;
 
    paquete_finalizar_proceso = crear_paquete(FINALIZAR_PROCESO_RTA);
 
    agregar_a_paquete(paquete_finalizar_proceso, &pid_proceso_a_finalizar,  sizeof(uint32_t));
    
    enviar_paquete(paquete_finalizar_proceso, socket_kernel);   
    printf("Proceso enviado \n"); 
    eliminar_paquete(paquete_finalizar_proceso); 
}


/*---------------------------- CPU-------------------------*/

//Memoria deserializa/serializa lo enviado por Cpu





void enviar_respuesta_contexto(t_m_contexto* pcbproceso, int socket_cpu) {
    t_paquete* paquete_cpu = crear_paquete(SOLICITUD_CONTEXTO_RTA); // Tipo de paquete que indica envío a CPU

    // Agregar información del PCB al paquete
    agregar_a_paquete(paquete_cpu, &pcbproceso->pid, sizeof(int));

    // Agregar los registros de la CPU al paquete individualmente
    agregar_a_paquete(paquete_cpu, &pcbproceso->registros->PC, sizeof(uint32_t));
    agregar_a_paquete(paquete_cpu, &pcbproceso->registros->AX, sizeof(uint8_t));
    agregar_a_paquete(paquete_cpu, &pcbproceso->registros->BX, sizeof(uint8_t));
    agregar_a_paquete(paquete_cpu, &pcbproceso->registros->CX, sizeof(uint8_t));
    agregar_a_paquete(paquete_cpu, &pcbproceso->registros->DX, sizeof(uint8_t));
    agregar_a_paquete(paquete_cpu, &pcbproceso->registros->EX, sizeof(uint32_t));
    agregar_a_paquete(paquete_cpu, &pcbproceso->registros->FX, sizeof(uint32_t));
    agregar_a_paquete(paquete_cpu, &pcbproceso->registros->GX, sizeof(uint32_t));
    agregar_a_paquete(paquete_cpu, &pcbproceso->registros->HX, sizeof(uint32_t));
    //falta base y limite

    // Enviar el paquete a la CPU
    enviar_paquete(paquete_cpu, socket_cpu); //SOCKET MAL
    printf("Contexto enviado %s\n", pcbproceso); 
    // Liberar recursos del paquete
    eliminar_paquete(paquete_cpu);
}


t_proceso_memoria* deserializar_solicitud_instruccion(t_list*  lista_paquete ){

    t_proceso_memoria* proxima_instruccion = malloc(sizeof(t_proceso_memoria));
    
    proxima_instruccion->pid = *(uint32_t*)list_get(lista_paquete, 0);
    printf("Pid recibido: %d \n", proxima_instruccion->pid);
    
    proxima_instruccion->program_counter = *(uint32_t*)list_get(lista_paquete, 1);
    printf("Program counter: %d \n", proxima_instruccion->program_counter);

    return proxima_instruccion;
}


void enviar_respuesta_instruccion(char* proxima_instruccion ,int socket_cpu) {
    t_paquete* paquete_instruccion;
 
    paquete_instruccion = crear_paquete(SOLICITUD_INSTRUCCION_RTA);

    agregar_a_paquete(paquete_instruccion, proxima_instruccion,  strlen(proxima_instruccion) + 1);          
    
    enviar_paquete(paquete_instruccion, socket_cpu);   
    printf("Instruccion enviada %s\n", proxima_instruccion); 
    eliminar_paquete(paquete_instruccion);
}


t_escribir_leer* deserializar_read_memoria(t_list*  lista_paquete ){

    t_escribir_leer* peticion_valor = malloc(sizeof(t_escribir_leer));
    
    peticion_valor->pid = *(uint32_t*)list_get(lista_paquete, 0);
    printf("Pid recibido: %d \n", peticion_valor->pid);
    
    peticion_valor->direccion_fisica = *(uint32_t*)list_get(lista_paquete, 1);
    printf("Direccion fisica: %d \n", peticion_valor->direccion_fisica);

    peticion_valor->tamanio = *(uint32_t*)list_get(lista_paquete, 2);
    printf("Tamanio proceso: %d \n", peticion_valor->tamanio);

    return peticion_valor;
}


void enviar_respuesta_read_memoria(void* respuesta_leer, int socket_cpu) {
    t_paquete* paquete_valor;

    // Verificar que respuesta_leer no sea NULL
    if (respuesta_leer == NULL) {
        fprintf(stderr, "Error: respuesta_leer es NULL\n");
        return;
    }

    // Asegurarse de que respuesta_leer está terminada en nulo
    char* cadena = (char*) respuesta_leer;
    size_t longitud = strlen(cadena);
    if (cadena[longitud] != '\0') {
        fprintf(stderr, "Error: la cadena no está correctamente terminada en nulo\n");
        return;
    }


    paquete_valor = crear_paquete(READ_MEMORIA_RTA);

    uint32_t tamanio_respuesta_leer = (longitud * sizeof(char)) + 1;

    agregar_a_paquete(paquete_valor, &tamanio_respuesta_leer, sizeof(uint32_t)); 
    agregar_a_paquete(paquete_valor, respuesta_leer, tamanio_respuesta_leer);          

    printf("respuesta_leer: %s , tamanio %d \n", (char*) respuesta_leer, tamanio_respuesta_leer); 
    
    enviar_paquete(paquete_valor, socket_cpu);   
    printf("Se envió respuesta de lectura \n"); 

    eliminar_paquete(paquete_valor);
}




t_escribir_leer* deserializar_write_memoria(t_list*  lista_paquete){

    t_escribir_leer* peticion_guardar = malloc(sizeof(t_escribir_leer));
    
    peticion_guardar->pid = *(uint32_t*)list_get(lista_paquete, 0);
    printf("Pid recibido: %d \n", peticion_guardar->pid);
    
    peticion_guardar->direccion_fisica = *(uint32_t*)list_get(lista_paquete, 1);
    printf("Direccion fisica: %d \n", peticion_guardar->direccion_fisica);

    peticion_guardar->tamanio = *(uint32_t*)list_get(lista_paquete, 2);
    printf("Tamanio proceso: %d \n", peticion_guardar->tamanio);

    peticion_guardar->valor = list_get(lista_paquete, 3);
    printf("Valor: %s \n", peticion_guardar->valor);

    return peticion_guardar;
}






void enviar_respuesta_write_memoria(char* respuesta_escribir, int socket_cliente){
    t_paquete* paquete_valor;

    paquete_valor = crear_paquete(WRITE_MEMORIA_RTA);

    //agregar_a_paquete(paquete_valor, &respuesta_escribir,  sizeof(void*));

    enviar_paquete(paquete_valor, socket_cliente);
    printf("Se envio respuesta de guardado \n"); 
   eliminar_paquete(paquete_valor);
}