#include"../include/server_cpu.h"
char* puerto_dispatch;
char * puerto_interrupt;
int fd_mod2 = -1;
int fd_mod3 = -1;
//pcb *pcb_actual;

void* crear_servidor_dispatch(char* ip_cpu){
    log_info(logger_cpu, "empieza crear_servidor_dispatch");

    log_info(logger_cpu, "valor de PUERTO_ESCUCHA_DISPATCH: %s", cfg_cpu->PUERTO_ESCUCHA_DISPATCH);
    

    puerto_dispatch = malloc((strlen(cfg_cpu->PUERTO_ESCUCHA_DISPATCH) + 1) * sizeof(char));
if (puerto_dispatch != NULL) {
    strcpy(puerto_dispatch, cfg_cpu->PUERTO_ESCUCHA_DISPATCH);
}
else{
    log_info(logger_cpu,"error al asignar memoria a variable del puerto");
}
    //strcpy(puerto_dispatch, cfg_cpu->PUERTO_ESCUCHA_DISPATCH);
    log_info(logger_cpu, "crea puerto_dispatch");
    printf("El puerto_dispatch es: %s", puerto_dispatch);
    fd_mod2 = iniciar_servidor(logger_cpu, "SERVER CPU DISPATCH", ip_cpu,  puerto_dispatch);
    log_info(logger_cpu, "inicio servidor");
    if (fd_mod2 == 0) {
        log_error(logger_cpu, "Fallo al crear el servidor, cerrando cpu");
        return EXIT_FAILURE;
    }
    

//log_info(logger_cpu, "POST SEMAFORO");
  //     sem_post(&sem_valor_instruccion);
    while (server_escuchar_dispatch(logger_cpu, "DISPATCH", (uint32_t)fd_mod2,&conexion_kernel_dispatch));
}

int server_escuchar_dispatch(t_log *logger, char *server_name, int server_socket, int *global_socket) {
    log_info(logger_cpu, "entra a server escuchar");
    int cliente_socket = esperar_cliente(logger, server_name, server_socket);
    log_info(logger_cpu, "cliente conectado socket %d", cliente_socket);
    *global_socket = cliente_socket; 
    
    sem_post(&sem_conexion_dispatch_iniciado);
    if (cliente_socket != -1) {
        pthread_t atenderProcesoNuevo;
         t_procesar_conexion_args *args = malloc(sizeof(t_procesar_conexion_args));
        args->log = logger;
        args->fd = cliente_socket;
        args->server_name = server_name;
        // Agregar el socket a la lista global
       // int* socket_ptr = malloc(sizeof(int));
      //  *socket_ptr = cliente_socket;
      //  list_add(lista_sockets_global, socket_ptr);
    
        pthread_create(&atenderProcesoNuevo, NULL,(void*)procesar_conexion_dispatch,(void*)args);
        pthread_detach(atenderProcesoNuevo);
        return 1;
    }
    return 0;
}

/*
void procesar_conexion(int cliente_socket){
    printf("El socket del cliente es: %d", cliente_socket);
}*/

void procesar_conexion_dispatch(void *v_args){
     t_procesar_conexion_args *args = (t_procesar_conexion_args *) v_args;
    t_log *logger = malloc(sizeof(t_log));
    logger = args->log;
    int cliente_socket = args->fd;
    char *server_name = args->server_name;
    free(args);

     op_code cop;
 
    while (cliente_socket != -1) {
   
        if (recv(cliente_socket, &cop, sizeof(uint32_t), MSG_WAITALL) != sizeof(uint32_t)) {
            log_info(logger, "DISCONNECT! KERNEL");

            break;
        }
           printf("COP:%d\n",cop);

        switch (cop){
    
            case PROCESO_EJECUTAR:
            {
                printf("Ejecutando procesoo\n");
                t_list* lista_paquete_proceso_ejecutar = recibir_paquete(cliente_socket);
                t_proceso* proceso = proceso_deserializar(lista_paquete_proceso_ejecutar); 
                pthread_mutex_lock(&mutex_proceso_actual);
                proceso_actual = proceso; //Agregar a lista de procesos?               
                //TODO: AGrego aca solicitud contexto para cualquier instruccion
                solicitar_contexto_a_memoria(proceso,socket_memoria);
                sem_wait(&sem_valor_base_particion);
                pthread_mutex_unlock(&mutex_proceso_actual);

                //list_destroy(lista_paquete_proceso_ejecutar); //guarda con esto
                //free(proceso);
                printf("pase free proceso\n"); 
                break;
            }
            default:
            {
                printf("Codigo de operacion no identifcado\n");
                break;
            }
            
           
        }   
 
    }
}
void procesar_conexion_interrupt(void *v_args){
     t_procesar_conexion_args *args = (t_procesar_conexion_args *) v_args;
    t_log *logger = malloc(sizeof(t_log));
    logger = args->log;
    int cliente_socket = args->fd;
    char *server_name = args->server_name;
    free(args);

     op_code cop;

    while (cliente_socket != -1) {
    
 
        if (recv(cliente_socket, &cop, sizeof(uint32_t), MSG_WAITALL) != sizeof(uint32_t)) {
            log_info(logger, "DISCONNECT! KERNEL");

            break;
        }
        pthread_mutex_lock(&mutex_interrupcion_kernel);  
           printf("COP:%d\n",cop);


        switch (cop){       
            case FIN_DE_QUANTUM:
            {
                log_info(logger_cpu, "## Llega interrupción al puerto Interrupt"); // LOG OBLIGATORIO
                pthread_mutex_lock(&mutex_proceso_actual);
                proceso_actual = NULL;               
                pthread_mutex_unlock(&mutex_proceso_actual);
                
                break;
            }

            default:
            {
                printf("Codigo de operacion no identifcado\n");
                break;
            }
            
           
        }   
        pthread_mutex_unlock(&mutex_interrupcion_kernel);  
  
    }

 
}

void atender_memoria(int *socket_mr) {
   int socket_memoria_server = *socket_mr;
  // free(socket);
    op_code cop;
  
    while (socket_memoria_server != -1) {

        if (recv(socket_memoria_server, &cop, sizeof(int32_t), MSG_WAITALL) != sizeof(int32_t)) {
            log_info(logger_cpu, "DISCONNECT! Atender memoria");

            break;
        }
        switch (cop) {

            case SOLICITUD_INSTRUCCION_RTA:
                
                {
                log_info(logger_cpu, "SE RECIBE INSTRUCCION DE MEMORIA");
                    
                t_list* lista_paquete_instruccion_rec = recibir_paquete(socket_memoria_server);
                    log_info(logger_cpu, "Paquete recibido");
                instr_t* instruccion_recibida = instruccion_deserializar(lista_paquete_instruccion_rec);
                    log_info(logger_cpu, "EL codigo de instrucción es %d ",instruccion_recibida->id);
                if(instruccion_recibida != NULL){
                    prox_inst = instruccion_recibida;
                    log_info(logger_cpu, "EL codigo de instrucción es %d ",prox_inst->id);
                    //SEMAFORO QUE ACTIVA EL SEGUIMIENTO DEL FLUJO EN FETCH
                    list_destroy_and_destroy_elements(lista_paquete_instruccion_rec,free);
                    /*free(instruccion_recibida->param1);
                    free(instruccion_recibida->param2);
                    free(instruccion_recibida->param3);
                    free(instruccion_recibida->param4);
                    free(instruccion_recibida->param5);
                    free(instruccion_recibida);*/
                    log_info(logger_cpu, "POST SEMAFORO");
                    sem_post(&sem_valor_instruccion);
                }
                else{
                    log_info(logger_cpu, "ERROR AL  RECIBIR INSTRUCCION DE MEMORIA");
                    list_destroy_and_destroy_elements(lista_paquete_instruccion_rec,free);
                    /*free(instruccion_recibida->param1);
                    free(instruccion_recibida->param2);
                    free(instruccion_recibida->param3);
                    free(instruccion_recibida->param4);
                    free(instruccion_recibida->param5);
                    free(instruccion_recibida);*/
                }
                break;
                }
            case SOLICITUD_CONTEXTO_RTA: 
                t_list* lista_paquete_contexto = recibir_paquete(socket_memoria_server);
                proceso_actual=malloc(sizeof(t_proceso));//TODO: FIXME: ahhhhhhhhhhhhhhhhh
                deserializar_contexto_(proceso_actual, lista_paquete_contexto); // ojo con semarofo de proceso actual
                sem_post(&sem_valor_base_particion);
                list_destroy(lista_paquete_contexto);
            break;
            case DEVOLUCION_CONTEXTO_RTA_OK: 
                t_list* lista_paquete_ctx_rta = recibir_paquete(socket_memoria_server);
                int pid_v= *(uint32_t*)list_get(lista_paquete_ctx_rta,0);
                int tid_v = *(uint32_t*)list_get(lista_paquete_ctx_rta,1);
                
                list_destroy(lista_paquete_ctx_rta);
            break;            
            default:
                {
                    log_error(logger_cpu, "Operacion invalida enviada desde Memoria:%d",cop);
                    break;
                }
            break;
        }
    }
}

int hacer_handshake (int socket_cliente){
    uint32_t handshake  = HANDSHAKE;
    printf("El handshake a enviar es: %d", handshake);
    send(socket_cliente, &handshake, sizeof(uint32_t), NULL);
    return recibir_operacion(socket_cliente);
}

t_proceso *proceso_deserializar(t_list*  lista_paquete_proceso ) {
    t_proceso *proceso_nuevo = malloc(sizeof(t_proceso));
   
    proceso_nuevo->pid = *((uint32_t*)list_get(lista_paquete_proceso, 0));
    log_info(logger_cpu, "recibi el pid:%u",proceso_nuevo->pid);

    proceso_nuevo->tid = *((uint32_t*)list_get(lista_paquete_proceso, 1));
    log_info(logger_cpu, "recibi el tid:%u",proceso_nuevo->tid);

    proceso_nuevo->registros_cpu.PC = 0;
    proceso_nuevo->registros_cpu.AX = 0;
    proceso_nuevo->registros_cpu.BX = 0;
    proceso_nuevo->registros_cpu.CX = 0;
    proceso_nuevo->registros_cpu.DX = 0;
    proceso_nuevo->registros_cpu.EX = 0;
    proceso_nuevo->registros_cpu.FX = 0;
    proceso_nuevo->registros_cpu.GX = 0;
    proceso_nuevo->registros_cpu.HX = 0;
    proceso_nuevo->registros_cpu.base = 0;
    proceso_nuevo->registros_cpu.limite = 0;
	return proceso_nuevo;
}

int server_escuchar_interrupt(t_log *logger, char *server_name, int server_socket, int *global_socket) {
    log_info(logger_cpu, "entra a server escuchar");
    int cliente_socket = esperar_cliente(logger, server_name, server_socket);
    log_info(logger_cpu, "sale de esperar_cliente");
    *global_socket = cliente_socket;
    sem_post(&sem_conexion_interrupt_iniciado);
    if (cliente_socket != -1) {
        pthread_t atenderProcesoNuevo;
         t_procesar_conexion_args *args = malloc(sizeof(t_procesar_conexion_args));
        args->log = logger;
        args->fd = cliente_socket;
        args->server_name = server_name;
        //pthread_create(&atenderProcesoNuevo, NULL,procesar_conexion,cliente_socket);//TODO:Redefinir procesar_conexion para que reciba un PCB
        pthread_create(&atenderProcesoNuevo, NULL,(void*)procesar_conexion_interrupt,(void*)args);
        pthread_detach(atenderProcesoNuevo);
        return 1;
    }
    return 0;
}


void* crear_servidor_interrupt(char* ip_cpu){
    log_info(logger_cpu, "empieza crear_servidor_interrupt");

    log_info(logger_cpu, "valor de PUERTO_ESCUCHA_INTERRUPT: %s", cfg_cpu->PUERTO_ESCUCHA_INTERRUPT);
    

    puerto_interrupt = malloc((strlen(cfg_cpu->PUERTO_ESCUCHA_INTERRUPT) + 1) * sizeof(char));
if (puerto_interrupt != NULL) {
    strcpy(puerto_interrupt, cfg_cpu->PUERTO_ESCUCHA_INTERRUPT);
}
else{
    log_info(logger_cpu,"error al asignar memoria a variable del puerto");
}
    //strcpy(puerto_interrupt, cfg_cpu->PUERTO_ESCUCHA_INTERRUPT);
    log_info(logger_cpu, "crea puerto_interrupt");
    printf("El puerto_interrupt es: %s", puerto_interrupt);
    fd_mod3 = iniciar_servidor(logger_cpu, "SERVER CPU INTERRUPT", ip_cpu,  puerto_interrupt);
    log_info(logger_cpu, "inicio servidor");
    if (fd_mod3 == 0) {
        log_error(logger_cpu, "Fallo al crear el servidor, cerrando cpu");
        return EXIT_FAILURE;
    }
log_info(logger_cpu, "va a escuchar");
    while (server_escuchar_interrupt(logger_cpu, "INTERRUPT", (uint32_t)fd_mod3,&conexion_kernel_interrupt));
}



instr_t* instruccion_deserializar(t_list* lista_paquete_inst){
    instr_t *instruccion_nueva = malloc(sizeof(instr_t));
    log_info(logger_cpu, "Paquete recibido Entro en instruccion deserealizar");
    char* instruccion = list_get(lista_paquete_inst, 0);
    log_info(logger_cpu,"instruccioiin recibida: %s", instruccion);
    armar_instr(instruccion_nueva,instruccion);
  
	return instruccion_nueva;
}

char* deserealizar_valor_memoria(t_list*  lista_paquete ){
    //uint32_t tamanio_valor_recibido = *(uint32_t*)list_get(lista_paquete, 0);
    //char* valor_recibido = malloc(tamanio_valor_recibido);
    //valor_recibido = list_get(lista_paquete, 1);
    uint32_t tamanio_valor_recibido = *(uint32_t*)list_get(lista_paquete, 0);
    printf("Recibo tamanio_valor_recibido:%u\n",tamanio_valor_recibido);
    char* valor_recibido = malloc(tamanio_valor_recibido);
    valor_recibido = strdup(list_get(lista_paquete, 1));
    printf("Recibo valor_recibido:%s\n",valor_recibido);
    printf("Devuelvo valor_recibido\n");
	return valor_recibido;
}



void armar_instr(instr_t *instr, const char *input) {
    // Copia la cadena de entrada para no modificar el original
    char *input_copy = strdup(input);
    char *token = strtok(input_copy, " ");
    if (token == NULL) {
        free(input_copy);
        return;
    }
      log_info(logger_cpu, "el token es %s", token );
    // Primer token es el id
    instr->id = str_to_tipo_instruccion(token);
    instr->idLength = strlen(token);

    // Inicializo estructura
    instr->param1Length = 0;
    instr->param1 = NULL;
    instr->param2Length = 0;
    instr->param2 = NULL;
    instr->param3Length = 0;
    instr->param3 = NULL;
    instr->param4Length = 0;
    instr->param4 = NULL;
    instr->param5Length = 0;
    instr->param5 = NULL;

    int param_count = 0;
    while ((token = strtok(NULL, " ")) != NULL) {
        switch (param_count) {
            case 0:
                instr->param1 = strdup(token);
                instr->param1Length = strlen(token);
                break;
            case 1:
                instr->param2 = strdup(token);
                instr->param2Length = strlen(token);
                break;
            case 2:
                instr->param3 = strdup(token);
                instr->param3Length = strlen(token);
                break;
            default:
                break;
        }
        param_count++;
    }

    free(input_copy);
}

void free_instr(instr_t *instr) {
    
    if (instr->param1 != NULL) free(instr->param1);
    if (instr->param2 != NULL) free(instr->param2);
    if (instr->param3 != NULL) free(instr->param3);
    if (instr->param4 != NULL) free(instr->param4);
    if (instr->param5 != NULL) free(instr->param5);
}