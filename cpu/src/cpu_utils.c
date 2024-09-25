#include"../include/cpu_utils.h"

int tamanioParams;
int tamanioInterfaces;
t_proceso* proceso_actual;

instr_t* fetch(int conexion, t_proceso* proceso){
    log_info(logger_cpu, "Voy a entrar a pedir_instruccion");
    log_info(logger_cpu, "PID: %u- TID:%u -FETCH- Program Counter: %u", proceso->pid,proceso->pid,proceso->registros_cpu.PC); //LOG OBLIGATORIO
    log_info(logger_cpu, "Voy a entrar a pedir_instruccion");
    pedir_instruccion(proceso, conexion); 
    //TODO:WAIT semaforo
    sem_wait(&sem_valor_instruccion);
    return prox_inst;
}

tipo_instruccion decode(instr_t* instr){
    log_info(logger_cpu, "EL codigo de instrucción es %d ",instr->id);
    return instr->id;//TODO: VER IMPLEMENTACION
   // return SET;
}


void execute(instr_t* inst,tipo_instruccion tipo_inst, t_proceso* proceso, int conexion, int socket_dispatch, int socket_interrupt){
    
        switch(tipo_inst){
            case SET:
            {   
                log_info(logger_cpu, "ENTRO EN SET ");
                log_info(logger_cpu, "PID: %u - Ejecutando: SET - %s %s", proceso->pid,inst->param1,inst->param2); //LOG OBLIGATORIO
                char *endptr;
                uint32_t param2_num = (uint32_t)strtoul(inst->param2, &endptr, 10);// Convertir la cadena a uint32_t
                set(inst->param1, param2_num, proceso);
                break;
            }
            case SUM:
            {
                log_info(logger_cpu, "PID: %u - Ejecutando: SUM - %s %s", proceso->pid,inst->param1,inst->param2); //LOG OBLIGATORIO
                sum(inst->param1, inst->param2,proceso);
                break;
            }
            case SUB:
            {
                log_info(logger_cpu, "PID: %u - Ejecutando: SUB - %s %s", proceso->pid,inst->param1,inst->param2); //LOG OBLIGATORIO
                sub(inst->param1, inst->param2,proceso);
                break;
            }            

            case JNZ:
            {
                log_info(logger_cpu, "PID: %u - Ejecutando: JNZ - %s %s", proceso->pid,inst->param1,inst->param2); //LOG OBLIGATORIO
                jnz(inst->param1, inst->param2,proceso);
                break;
            }        

            case READ_MEM:
            {
                log_info(logger_cpu, "PID: %u - Ejecutando: READ_MEM - %s %s %s %s %s", proceso->pid,inst->param1,inst->param2,inst->param3,inst->param4,inst->param5); //LOG OBLIGATORIO
                break;
            }   

            case WRITE_MEM:
            {
                log_info(logger_cpu, "PID: %u - Ejecutando: WRITE_MEM  - %s %s %s %s %s", proceso->pid,inst->param1,inst->param2,inst->param3,inst->param4,inst->param5); //LOG OBLIGATORIO
                break;
            }   

            case LOG:
            {
                log_info(logger_cpu, "PID: %u - Ejecutando: LOG", proceso->pid); //LOG OBLIGATORIO
                loguear(inst->param1);
                break;
            }

            // SYSCALLS:
            case DUMP_MEMORY:
            {
                log_info(logger_cpu, "PID: %u - Ejecutando: DUMP_MEMORY", proceso->pid);                
                enviar_contexto_a_memoria(proceso,conexion);
                enviar_dump_memory_a_kernel(socket_dispatch);
                break;
            }
            case IO:
            {
                log_info(logger_cpu, "PID: %u - Ejecutando: IO", proceso->pid);
                enviar_io_a_kernel(inst->param1,socket_dispatch);
                enviar_contexto_a_memoria(proceso,conexion);
                break;
            }
            case PROCESS_CREATE:
            {
                log_info(logger_cpu, "PID: %u - Ejecutando: PROCESS_CREATE", proceso->pid);
               
                enviar_process_create_a_kernel(inst->param1, inst->param2, inst->param3, socket_dispatch);
                enviar_contexto_a_memoria(proceso,conexion);
                
                break;
            }
            case THREAD_CREATE:
            {
                log_info(logger_cpu, "PID: %u - Ejecutando: THREAD_CREATE", proceso->pid);
                enviar_thread_create_a_kernel(inst->param1, inst->param2, socket_dispatch);
                enviar_contexto_a_memoria(proceso,conexion);
                break;
            }
            case THREAD_JOIN:
            {
                log_info(logger_cpu, "PID: %u - Ejecutando: THREAD_JOIN", proceso->pid);
                enviar_thread_cancel_a_kernel(inst->param1, socket_dispatch);
                enviar_contexto_a_memoria(proceso,conexion);
                break;
            }
            case THREAD_CANCEL:
            {
                log_info(logger_cpu, "PID: %u - Ejecutando: THREAD_CANCEL", proceso->pid);
                enviar_thread_join_a_kernel(inst->param1, socket_dispatch);
                enviar_contexto_a_memoria(proceso,conexion);
                break;
            }
            case MUTEX_CREATE:
            {
                log_info(logger_cpu, "PID: %u - Ejecutando: MUTEX_CREATE", proceso->pid);
                enviar_mutex_create_a_kernel(inst->param1, socket_dispatch);
                enviar_contexto_a_memoria(proceso,conexion);
                break;
            }
            case MUTEX_LOCK:
            {
                log_info(logger_cpu, "PID: %u - Ejecutando: MUTEX_LOCK", proceso->pid);
                enviar_mutex_lock_a_kernel(inst->param1, socket_dispatch); 
                enviar_contexto_a_memoria(proceso,conexion);
                break;
            }
            case MUTEX_UNLOCK:
            {
                log_info(logger_cpu, "PID: %u - Ejecutando: MUTEX_UNLOCK", proceso->pid);
                enviar_mutex_unlock_a_kernel(inst->param1, socket_dispatch);
                enviar_contexto_a_memoria(proceso,conexion);
                break;
            }
            case THREAD_EXIT:
            {
                log_info(logger_cpu, "PID: %u - Ejecutando: THREAD_EXIT", proceso->pid);
                enviar_thread_exit_a_kernel(socket_dispatch);
                enviar_contexto_a_memoria(proceso,conexion);
                break;
            }                
            case PROCESS_EXIT:
            {
                log_info(logger_cpu, "PID: %u - Ejecutando: PROCESS_EXIT", proceso->pid);
                enviar_process_exit_a_kernel(socket_dispatch);
                enviar_contexto_a_memoria(proceso,conexion);
                break;
            }            

            default:
                log_warning(logger_cpu, "Hubo un error: instrucción no encontrada");
        }

}

void check_interrupt(int conexion_kernel){
     printf("ENTRO EN CHECK INTERRUPT\n");    
  
    pthread_mutex_lock(&mutex_interrupcion_kernel);
    if(interrupcion_kernel){
        proceso_actual= NULL;   
        printf("ENTRO EN IF DEL  CHECK INTERRUPT\n");
        generar_interrupcion_a_kernel(conexion_kernel); 
        interrupcion_kernel = false;
    }
    pthread_mutex_unlock(&mutex_interrupcion_kernel);
    
}

void pedir_instruccion(t_proceso* proceso,int conexion){  
    
    t_paquete* paquete_pedido_instruccion;
    paquete_pedido_instruccion = crear_paquete(SOLICITUD_INSTRUCCION);
        
    agregar_a_paquete(paquete_pedido_instruccion,  &proceso->pid,  sizeof(uint32_t));
    agregar_a_paquete(paquete_pedido_instruccion, &proceso->tid,  sizeof(uint32_t));
    agregar_a_paquete(paquete_pedido_instruccion,  &proceso->registros_cpu.PC,  sizeof(uint32_t));  
        
    enviar_paquete(paquete_pedido_instruccion, conexion); 
    eliminar_paquete(paquete_pedido_instruccion);
}

//////////////////////////////////////// INSTRUCCIONES //////////////////////////////////////////
void set(char* registro, uint32_t valor, t_proceso* proceso){
    //printf("El valor del set es : %d ", valor);
    registros registro_elegido = identificarRegistro(registro);
    //pthread_mutex_lock(&mutex_proceso_actual);
    switch(registro_elegido){
        case PC:
        {
          proceso->registros_cpu.PC = valor;
            break;
        }
        case AX:
        {
          proceso->registros_cpu.AX = valor;
            break;
        }
        case BX:
        {
          proceso->registros_cpu.BX = valor;
            break;
        }
        case CX:
        {
          proceso->registros_cpu.CX = valor;
            break;
        }
        case DX:
        {
          proceso->registros_cpu.DX = valor;
            break;
        }
        case EX:
        {
          proceso->registros_cpu.EX = valor;
            break;
        }
        case FX:
        {
          proceso->registros_cpu.FX = valor;
            break;
        }
        case GX:
        {
          proceso->registros_cpu.GX = valor;
            break;
        }
        case HX:
        {
          proceso->registros_cpu.HX = valor;
            break;
        }
        
        default:
        log_info(logger_cpu, "El registro no existe");
    }
   // pthread_mutex_unlock(&mutex_proceso_actual);

    //proceso->pcb->registros_cpu.AX;
   // registro = valor;
}

void sum(char* registro_destino, char* registro_origen, t_proceso* proceso){
    registros id_registro_destino = identificarRegistro(registro_destino);
    registros id_registro_origen = identificarRegistro(registro_origen);

    uint32_t valor_reg_destino = obtenerValorActualRegistro(id_registro_destino,proceso);
    uint32_t valor_reg_origen = obtenerValorActualRegistro(id_registro_origen,proceso);
    pthread_mutex_lock(&mutex_proceso_actual);
    switch(id_registro_destino){
        case PC:
        {
           proceso->registros_cpu.PC = valor_reg_destino + valor_reg_origen;
            break;
        }
        case AX:
        {
           proceso->registros_cpu.AX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case BX:
        {
           proceso->registros_cpu.BX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case CX:
        {
           proceso->registros_cpu.CX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case DX:
        {
           proceso->registros_cpu.DX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case EX:
        {
           proceso->registros_cpu.EX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case FX:
        {
           proceso->registros_cpu.FX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case GX:
        {
           proceso->registros_cpu.GX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case HX:
        {
           proceso->registros_cpu.HX = valor_reg_destino + valor_reg_origen;
            break;
        }
        
        default:
        log_info(logger_cpu, "El registro no existe");
    }
    pthread_mutex_unlock(&mutex_proceso_actual);


    
}

void sub(char* registro_destino, char* registro_origen, t_proceso* proceso){
    registros id_registro_destino = identificarRegistro(registro_destino);
    registros id_registro_origen = identificarRegistro(registro_origen);

    uint32_t valor_reg_destino = obtenerValorActualRegistro(id_registro_destino,proceso);
    uint32_t valor_reg_origen = obtenerValorActualRegistro(id_registro_origen,proceso);
    pthread_mutex_lock(&mutex_proceso_actual);
    switch(id_registro_destino){
        case PC:
        {
           proceso->registros_cpu.PC = valor_reg_destino - valor_reg_origen;
            break;
        }
        case AX:
        {
           proceso->registros_cpu.AX = valor_reg_destino - valor_reg_origen;
            break;
        }
        case BX:
        {
           proceso->registros_cpu.BX = valor_reg_destino - valor_reg_origen;
            break;
        }
        case CX:
        {
           proceso->registros_cpu.CX = valor_reg_destino - valor_reg_origen;
            break;
        }
        case DX:
        {
           proceso->registros_cpu.DX = valor_reg_destino - valor_reg_origen;
            break;
        }
        case EX:
        {
           proceso->registros_cpu.EX = valor_reg_destino - valor_reg_origen;
            break;
        }
        case FX:
        {
           proceso->registros_cpu.FX = valor_reg_destino - valor_reg_origen;
            break;
        }
        case HX:
        {
           proceso->registros_cpu.HX = valor_reg_destino - valor_reg_origen;
            break;
        }
        case GX:
        {
           proceso->registros_cpu.GX = valor_reg_destino - valor_reg_origen;
            break;
        }        
        default:
        log_info(logger_cpu, "El registro no existe");
    }
    pthread_mutex_unlock(&mutex_proceso_actual);
  
}


void jnz(char* registro, uint32_t inst, t_proceso* proceso){
    registros id_registro = identificarRegistro(registro);
    uint32_t valor_registro = obtenerValorActualRegistro(id_registro,proceso);
    if(valor_registro != 0){
        pthread_mutex_lock(&mutex_proceso_actual);
        proceso->registros_cpu.PC = inst;
        pthread_mutex_unlock(&mutex_proceso_actual);
    }
}

void loguear(char* registro){
    registros id_registro = identificarRegistro(registro);
    uint32_t valor_reg = obtenerValorActualRegistro(id_registro,proceso_actual);
    log_info(logger_cpu, "valor registro %s: %d",registro, valor_reg);
}

void limpiarCadena(char* cadena) {
    char* token;
    char delimitadores[] = " \n\t"; // Espacio, salto de línea y tabulador
    char cadenaLimpia[100] = ""; // Asegúrate de que esta cadena sea lo suficientemente grande

    // Obtener el primer token
    token = strtok(cadena, delimitadores);
    
    // Iterar sobre los siguientes tokens
    while (token != NULL) {
        strcat(cadenaLimpia, token);
        token = strtok(NULL, delimitadores);
    }

    // Copiar la cadena limpia de vuelta a la cadena original
    strcpy(cadena, cadenaLimpia);
}


registros identificarRegistro(char* registro){
     printf("ENTRO A IDENTIFICAR_REGISTRO: %s\n",registro); 
     limpiarCadena(registro);
    if(strcmp(registro,"PC") == 0){
        
        return PC;
    }
    else if(strcmp(registro,"AX") == 0){
         printf("Entro AX \n"); 
        return AX;
    }
    else if(strcmp(registro,"BX") == 0){
        return BX;
    }
    else if(strcmp(registro,"CX") == 0){
        return CX;
    }
    else if(strcmp(registro,"DX") == 0){
        return DX;
    }
    else if(strcmp(registro,"EX") == 0){
        return EX;
    }
    else if(strcmp(registro,"FX") == 0){
        return FX;
    }
    else if(strcmp(registro,"GX") == 0){
        return GX;
    }
    else if(strcmp(registro,"HX") == 0){
        return HX;
    }
    else if(strcmp(registro,"base") == 0){
        return base;
    }
    else if(strcmp(registro,"limite") == 0){
        return limite;
    }
    else{
        return REG_NO_ENC;
    }
}

uint32_t obtenerValorActualRegistro(registros id_registro, t_proceso* proceso){
    switch(id_registro){
        case PC:
        {
           return proceso->registros_cpu.PC;
            break;
        }
        case AX:
        {
           return proceso->registros_cpu.AX;
            break;
        }
        case BX:
        {
           return proceso->registros_cpu.BX;
            break;
        }
        case CX:
        {
           return proceso->registros_cpu.CX;
            break;
        }
        case DX:
        {
           return proceso->registros_cpu.DX;
            break;
        }
        case EX:
        {
           return proceso->registros_cpu.EX;
            break;
        }
        case FX:
        {
           return proceso->registros_cpu.FX;
            break;
        }
        case GX:
        {
           return proceso->registros_cpu.GX;
            break;
        }
        case HX:
        {
           return proceso->registros_cpu.HX;
            break;
        }
        case base:
        {
           return proceso->registros_cpu.base;
            break;
        }
        case limite:
        {
           return proceso->registros_cpu.limite;
            break;
        }
      
        default:
        log_info(logger_cpu, "El registro no existe");
    }
}




uint32_t mmu(uint32_t direccion_logica, int conexion, int pid, int conexion_kernel_dispatch){
    uint32_t direccion_resultado;
    uint32_t desplazamiento = direccion_logica;
    obtener_base_particion(conexion,pid);
 
    
    sem_wait(&sem_valor_base_particion);
    //validacion de limites de particion
    if (1==1){
        direccion_resultado = base_particion+desplazamiento ;  
        log_info(logger_cpu, "PID: %u - ", proceso_actual->pid); //LOG OBLIGATORIO
        return direccion_resultado;
    }else{
        //segmentation fault
        enviar_contexto_a_memoria(proceso_actual, conexion);
        enviar_segfault_a_kernel(proceso_actual,conexion_kernel_dispatch);
    }            


}



void read_mem(char* registro_datos, char* registro_direccion, t_proceso* proceso, t_log* logger, int conexion){
    // Lee el valor de memoria correspondiente a la Dirección Lógica que se encuentra en el 
    //Registro Dirección y lo almacena en el Registro Datos
    registros id_registro_direccion = identificarRegistro(registro_direccion);
    
    //uint32_t valor_registro_direccion = malloc(sizeof(uint32_t));
    uint32_t valor_registro_direccion = obtenerValorActualRegistro(id_registro_direccion,proceso);

    uint32_t dir_fisica_result;
    dir_fisica_result = mmu(valor_registro_direccion,base,conexion, conexion_kernel_dispatch);

    registros id_registro_datos = identificarRegistro(registro_datos);

    uint32_t tamanio_a_leer = 4;

    pedir_valor_a_memoria(dir_fisica_result,proceso->pid,tamanio_a_leer,conexion);
    int valor_sem;
    sem_getvalue(&sem_valor_registro_recibido, &valor_sem);
    printf("valor sem_valor_registro_recibido:%d\n", valor_sem);
    sem_wait(&sem_valor_registro_recibido);
    printf("paso sem_valor_registro_recibido\n");

    log_info(logger, "PID: %u - Acción: LEER - Dirección Física: %u - Valor: %s", proceso_actual->pid,dir_fisica_result,valor_registro_obtenido); //LOG OBLIGATORIO
    
    char *endptr;
    printf("paso endptr\n");
    uint32_t valor_dir_fisica_uint32;
    uint8_t valor_dir_fisica_uint8t;
    if(tamanio_a_leer == sizeof(uint32_t)){
         valor_dir_fisica_uint32 = (uint32_t)strtoul(valor_registro_obtenido, &endptr, 10);// Convertir la cadena a uint32_t
        set(registro_datos,valor_dir_fisica_uint32,proceso);
    }
    else{
         valor_dir_fisica_uint8t = (uint8_t)strtoul(valor_registro_obtenido, &endptr, 10);// Convertir la cadena a uint8_t
        set(registro_datos,valor_dir_fisica_uint8t,proceso);
    }
    
    

}

void write_mem(char* registro_direccion, char* registro_datos, t_proceso* proceso, t_log* logger, int conexion){
    // Lee el valor del Registro Datos y lo escribe en la dirección física de
    // memoria obtenida a partir de la Dirección Lógica almacenada en el Registro Dirección.
    printf("registro: %s\n",registro_datos);
    registros id_registro_datos = identificarRegistro(registro_datos);
    printf("registro: %d\n",id_registro_datos);
    uint32_t valor_registro_datos = obtenerValorActualRegistro(id_registro_datos,proceso);
    printf("manda %d a guardar en mem\n",valor_registro_datos);
    registros id_registro_direccion = identificarRegistro(registro_direccion);
    uint32_t valor_registro_direccion = obtenerValorActualRegistro(id_registro_direccion,proceso);

    uint32_t dir_fisica_result = mmu(valor_registro_direccion,base,conexion, conexion_kernel_dispatch);
    //TODO: Si el tamanio de valor_registro_datos(es un int de 32 siempre?) es mayor a tamanio_pagina hay
    //que dividir ambos y tomar el floor para obtener cant de paginas, con eso dividir datos a enviar en *cant de paginas*, y
    //por cada pedacito de intfo llamar a mmu y agregar dir fisca obtenida en lista 


// Calcular el tamaño necesario para el valor_str
    int len = snprintf(NULL, 0, "%u", valor_registro_datos);
    char* valor_str = malloc(len + 1);
    snprintf(valor_str, len + 1, "%u", valor_registro_datos);

    
    log_info(logger, "PID: %u - Acción: ESCRIBIR - Dirección Física: %u - Valor: %u", proceso_actual->pid,dir_fisica_result,valor_registro_datos); //LOG OBLIGATORIO

}



void pedir_valor_a_memoria(uint32_t dir_fisica, uint32_t pid, uint32_t tamanio, int conexion){
        printf("entro a pedir_valor_a_memoria\n");
        t_paquete* paquete_pedido_valor_memoria;
        paquete_pedido_valor_memoria = (HANDSHAKE); // TODO: Crear codigo de operacion

        agregar_a_paquete(paquete_pedido_valor_memoria,  &pid,  sizeof(uint32_t));     
        agregar_a_paquete(paquete_pedido_valor_memoria,  &dir_fisica,  sizeof(uint32_t)); 
        agregar_a_paquete(paquete_pedido_valor_memoria,  &tamanio,  sizeof(uint32_t));        
            
        enviar_paquete(paquete_pedido_valor_memoria, conexion); 
        eliminar_paquete(paquete_pedido_valor_memoria);

}


//////////////////////////////////////// SYSCALLS //////////////////////////////////////////


void enviar_dump_memory_a_kernel(int socket_dispatch){
    printf("entro a enviar_dump_memory_a_kernel\n");
    t_paquete* paquete_dump_memory;   
    paquete_dump_memory = crear_paquete(DUMP_MEMORY);     
    
    agregar_a_paquete(paquete_dump_memory, &proceso_actual->pid,  sizeof(uint32_t));
    agregar_a_paquete(paquete_dump_memory, &proceso_actual->tid,  sizeof(uint32_t));
    enviar_paquete(paquete_dump_memory, socket_dispatch); 
    eliminar_paquete(paquete_dump_memory);    
}


void enviar_io_a_kernel(char* tiempo ,int socket_dispatch){
    printf("entro a enviar_dump_memory_a_kernel\n");
    t_paquete* paquete_io;
    char *endptr;   
    paquete_io = crear_paquete(DUMP_MEMORY);     
    uint32_t tiempo_num = (uint32_t)strtoul(tiempo, &endptr, 10);// Convertir la cadena a uint32_t
    
    agregar_a_paquete(paquete_io, &proceso_actual->pid,  sizeof(uint32_t));
    agregar_a_paquete(paquete_io, &proceso_actual->tid,  sizeof(uint32_t));
    agregar_a_paquete(paquete_io, &tiempo_num,  sizeof(uint32_t));
    enviar_paquete(paquete_io, socket_dispatch); 
    eliminar_paquete(paquete_io);       
}

void enviar_process_create_a_kernel(char* nombre_pseudocodigo, char* tamanio_proceso, char* prioridad_hilo, int socket_dispatch){
    printf("entro a enviar_process_create_a_kernel\n");
    t_paquete* paquete_create_process;
    char *endptr;
    paquete_create_process = crear_paquete(PROCESO_CREAR); //AGREGAR LA OPERACION CORESPONDENTIE
    int tamanio_nombre_pseudocodigo = string_length(nombre_pseudocodigo)+1;
  
    //agregar_a_paquete(paquete_create_process, &tamanio_nombre_pseudocodigo,  sizeof(uint32_t));
    agregar_a_paquete(paquete_create_process, nombre_pseudocodigo, tamanio_nombre_pseudocodigo);
    uint32_t tamanio_proceso_num = (uint32_t)strtoul(tamanio_proceso, &endptr, 10);// Convertir la cadena a uint32_t
    agregar_a_paquete(paquete_create_process, &tamanio_proceso_num,  sizeof(uint32_t));
    uint32_t prioridad_hilo_num = (uint32_t)strtoul(prioridad_hilo, &endptr, 10);// Convertir la cadena a uint32_t
    agregar_a_paquete(paquete_create_process, &prioridad_hilo_num,  sizeof(uint32_t));
    enviar_paquete(paquete_create_process, socket_dispatch); 
    eliminar_paquete(paquete_create_process);

}

void enviar_thread_create_a_kernel(char* nombre_pseudocodigo, char* prioridad_hilo, int socket_dispatch){
    printf("entro a enviar_thread_create_a_kernel\n");
    t_paquete* paquete_create_thread;
    char *endptr;
    paquete_create_thread = crear_paquete(HILO_CREAR); //AGREGAR LA OPERACION CORESPONDENTIE
    int tamanio_nombre_pseudocodigo = string_length(nombre_pseudocodigo)+1;
  
    agregar_a_paquete(paquete_create_thread, &tamanio_nombre_pseudocodigo,  sizeof(uint32_t));
    agregar_a_paquete(paquete_create_thread, nombre_pseudocodigo, tamanio_nombre_pseudocodigo);     
    uint32_t prioridad_hilo_num = (uint32_t)strtoul(prioridad_hilo, &endptr, 10);// Convertir la cadena a uint32_t
    agregar_a_paquete(paquete_create_thread, &prioridad_hilo_num,  sizeof(uint32_t));
    enviar_paquete(paquete_create_thread, socket_dispatch); 
    eliminar_paquete(paquete_create_thread);
}


void enviar_thread_join_a_kernel(char* tid ,int socket_dispatch){
    printf("entro a enviar_thread_join_a_kernel\n");
    t_paquete* paquete_thread_join;
    char *endptr;
    paquete_thread_join = crear_paquete(HILO_CREAR); 
    uint32_t tid_numero = (uint32_t)strtoul(tid, &endptr, 10);
       
    agregar_a_paquete(paquete_thread_join, &tid_numero,  sizeof(uint32_t));
    enviar_paquete(paquete_thread_join, socket_dispatch); 
    eliminar_paquete(paquete_thread_join);    
}


void enviar_thread_cancel_a_kernel(char* tid ,int socket_dispatch){
    printf("entro a enviar_thread_cancel_a_kernel\n");
    t_paquete* paquete_thread_cancel;
    char *endptr;
    paquete_thread_cancel = crear_paquete(HILO_CANCELAR); 
    uint32_t tid_numero = (uint32_t)strtoul(tid, &endptr, 10);
       
    agregar_a_paquete(paquete_thread_cancel, &tid_numero,  sizeof(uint32_t));
    enviar_paquete(paquete_thread_cancel, socket_dispatch); 
    eliminar_paquete(paquete_thread_cancel);      
}


void enviar_mutex_create_a_kernel(char* recurso, int conexion_kernel){
    printf("entro a enviar mutex create a kernell\n");        
    t_paquete* paquete_mutex_create_kernel;   
    paquete_mutex_create_kernel = crear_paquete(MUTEX_CREAR); 
    int tamanio_recurso = strlen(recurso)+1;

    agregar_a_paquete(paquete_mutex_create_kernel,  &proceso_actual->pid,  sizeof(uint32_t));  
    agregar_a_paquete(paquete_mutex_create_kernel,  &tamanio_recurso,  sizeof(uint32_t));       
    agregar_a_paquete(paquete_mutex_create_kernel,  recurso,  tamanio_recurso);            
    enviar_paquete(paquete_mutex_create_kernel, conexion_kernel); 
    eliminar_paquete(paquete_mutex_create_kernel);

}


void enviar_mutex_lock_a_kernel(char* recurso, int conexion_kernel){
    printf("entro a enviar_mutex_lock_a_kernel\n");        
    t_paquete* paquete_lock_kernel;   
    paquete_lock_kernel = crear_paquete(MUTEX_BLOQUEAR); 
    int tamanio_recurso = strlen(recurso)+1;

    agregar_a_paquete(paquete_lock_kernel,  &proceso_actual->pid,  sizeof(uint32_t));  
    agregar_a_paquete(paquete_lock_kernel,  &tamanio_recurso,  sizeof(uint32_t));       
    agregar_a_paquete(paquete_lock_kernel,  recurso,  tamanio_recurso);            
    enviar_paquete(paquete_lock_kernel, conexion_kernel); 
    eliminar_paquete(paquete_lock_kernel);

}

void enviar_mutex_unlock_a_kernel(char* recurso, int conexion_kernel){
    printf("entro a enviar_mutex_unlock_a_kernel\n");
    t_paquete* paquete_unlock_kernel;   
    paquete_unlock_kernel = crear_paquete(MUTEX_DESBLOQUEAR); 
    int tamanio_recurso = strlen(recurso)+1;

    agregar_a_paquete(paquete_unlock_kernel,  &proceso_actual->pid,  sizeof(uint32_t));   
    agregar_a_paquete(paquete_unlock_kernel,  &tamanio_recurso,  sizeof(uint32_t));       
    agregar_a_paquete(paquete_unlock_kernel,  recurso,  tamanio_recurso);      
    enviar_paquete(paquete_unlock_kernel, conexion_kernel); 
    eliminar_paquete(paquete_unlock_kernel);
}


void enviar_process_exit_a_kernel(int conexion_kernel){
    t_paquete* paquete_process_exit_kernel;   
    paquete_process_exit_kernel = crear_paquete(PROCESO_SALIR); 
    
    agregar_a_paquete(paquete_process_exit_kernel,  &proceso_actual->pid,  sizeof(uint32_t));   
    enviar_paquete(paquete_process_exit_kernel, conexion_kernel); 
    eliminar_paquete(paquete_process_exit_kernel);    
    proceso_actual = NULL;
}


void enviar_thread_exit_a_kernel(int conexion_kernel){
    t_paquete* paquete_thread_exit_kernel;   
    paquete_thread_exit_kernel = crear_paquete(HILO_SALIR); 
    
    agregar_a_paquete(paquete_thread_exit_kernel,  &proceso_actual->pid,  sizeof(uint32_t)); 
    agregar_a_paquete(paquete_thread_exit_kernel,  &proceso_actual->tid,  sizeof(uint32_t));     
    enviar_paquete(paquete_thread_exit_kernel, conexion_kernel); 
    eliminar_paquete(paquete_thread_exit_kernel);  
    proceso_actual = NULL;    
}


////////////////////////////////  UTILS //////////////////////////////////////////

void imprimir_contenido_paquete(t_paquete* paquete);
void imprimir_contenido_paquete(t_paquete* paquete) {
    printf("Codigo de operacion: %d\n", paquete->codigo_operacion);
    printf("Tamaño del buffer: %d\n", paquete->buffer->size);
    printf("Contenido del buffer:\n");

    uint8_t* stream = (uint8_t*) paquete->buffer->stream;
    for (int i = 0; i < paquete->buffer->size; i++) {
        printf("%02X ", stream[i]);
    }
    printf("\n");
}

void obtener_base_particion(int conexion, int pid){ 
    printf("entro a obtener base particion\n");
    t_paquete* paquete_pedido_tamanio_base;
    paquete_pedido_tamanio_base = crear_paquete(BASE_PARTICION); 
    agregar_a_paquete(paquete_pedido_tamanio_base, pid,  sizeof(uint32_t));
    enviar_paquete(paquete_pedido_tamanio_base, conexion); 
    eliminar_paquete(paquete_pedido_tamanio_base);

}


 

void ciclo_de_instrucciones(int *conexion_mer, t_proceso *proceso, int *socket_dispatch, int *socket_interrupt)
{   log_info(logger_cpu, "Entro al ciclo");
    int conexion_mem = *conexion_mer;
    int dispatch = *socket_dispatch;
    int interrupt = *socket_interrupt;
   
    //free(conexion_mer);
    //free(socket_dispatch);
    //free(socket_interrupt);
    
    log_info(logger_cpu, "Entro al ciclo");

    instr_t *inst = malloc(sizeof(instr_t));
    log_info(logger_cpu, "Voy a entrar a fetch");
    inst = fetch(conexion_mem,proceso); 
    tipo_instruccion tipo_inst;
    log_info(logger_cpu, "Voy a entrar a decode");
    tipo_inst = decode(inst);
    log_info(logger_cpu, "Voy a entrar a execute");
    execute(inst, tipo_inst, proceso, conexion_mem, dispatch, interrupt);
    if (tipo_inst != PROCESS_EXIT && tipo_inst != THREAD_EXIT ) 
    {
        proceso_actual->registros_cpu.PC += 1;
    }
    log_info(logger_cpu, "Voy a entrar a check_interrupt");
    check_interrupt(socket_dispatch);
    log_info(logger_cpu, "Sale de check_interrupt");
    log_info(logger_cpu, "Termino ciclo de instrucciones");

    // interrupcion_kernel = false;

    free(inst->param1);
    free(inst->param2);
    free(inst->param3);
    free(inst->param4);
    free(inst->param5);
    free(inst);
}

tipo_instruccion str_to_tipo_instruccion(const char *str) {

    printf("Entro al funcion a testear \n");
    tipo_instruccion instruccion_a_devolver = -1;
    if (strcmp(str, "SET") == 0) instruccion_a_devolver = SET;
    else if (strcmp(str, "READ_MEM") == 0) instruccion_a_devolver = READ_MEM;
    else if (strcmp(str, "WRITE_MEM") == 0) instruccion_a_devolver = WRITE_MEM;
    else if (strcmp(str, "SUM") == 0) instruccion_a_devolver = SUM;
    else if (strcmp(str, "SUB") == 0) instruccion_a_devolver = SUB;
    else if (strcmp(str, "JNZ") == 0) instruccion_a_devolver = JNZ;
    else if (strcmp(str, "LOG") == 0) instruccion_a_devolver = LOG;
    else if (strcmp(str, "DUMP_MEMORY") == 0) instruccion_a_devolver = DUMP_MEMORY;
    else if (strcmp(str, "IO") == 0) instruccion_a_devolver = IO;
    else if (strcmp(str, "PROCESS_CREATE") == 0) instruccion_a_devolver = PROCESS_CREATE;
    else if (strcmp(str, "THREAD_JOIN") == 0) instruccion_a_devolver = THREAD_JOIN;
    else if (strcmp(str, "THREAD_CANCEL") == 0) instruccion_a_devolver = THREAD_CANCEL;
    else if (strcmp(str, "MUTEX_CREATE") == 0) instruccion_a_devolver = MUTEX_CREATE;
    else if (strcmp(str, "MUTEX_LOCK") == 0) instruccion_a_devolver = MUTEX_LOCK;
    else if (strcmp(str, "MUTEX_UNLOCK") == 0) instruccion_a_devolver = MUTEX_UNLOCK;
    else if (strcmp(str, "THREAD_EXIT") == 0) instruccion_a_devolver = THREAD_EXIT;
    else if (strcmp(str, "PROCESS_EXIT") == 0) instruccion_a_devolver = PROCESS_EXIT;
    else printf("Entro en el default de str_to_tipo_instruccion \n");

    printf("imprimo instruccion_a_devolver: %d \n", instruccion_a_devolver);
    printf("Saliendo de la funcion a testear \n");
    return instruccion_a_devolver;
}

void generar_interrupcion_a_kernel(int conexion){
    log_info(logger_cpu,"entro a generar_interrupcion_a_kernel\n");
    t_paquete* paquete_interrupcion_kernel;    
   
    paquete_interrupcion_kernel = crear_paquete(HANDSHAKE); //TODO: crear codigo de operacion
    enviar_paquete(paquete_interrupcion_kernel, conexion);   
    eliminar_paquete(paquete_interrupcion_kernel);
    log_info(logger_cpu,"Interrupcion kernel enviada a %d", conexion);
 }



void enviar_contexto_a_memoria(t_proceso* proceso, int conexion){
    printf("entro a DEVOLUCION_CONTEXTO\n");
    t_paquete* paquete_devolucion_contexto;

    paquete_devolucion_contexto = crear_paquete(DEVOLUCION_CONTEXTO); 
    agregar_a_paquete(paquete_devolucion_contexto, &proceso->pid,  sizeof(uint32_t));         
    agregar_a_paquete(paquete_devolucion_contexto, &proceso->tid,  sizeof(uint32_t));        
    agregar_a_paquete(paquete_devolucion_contexto, &proceso->registros_cpu.PC, sizeof(uint32_t));
    agregar_a_paquete(paquete_devolucion_contexto, &proceso->registros_cpu.AX, sizeof(uint32_t)); 
    agregar_a_paquete(paquete_devolucion_contexto, &proceso->registros_cpu.BX, sizeof(uint32_t)); 
    agregar_a_paquete(paquete_devolucion_contexto, &proceso->registros_cpu.CX, sizeof(uint32_t)); 
    agregar_a_paquete(paquete_devolucion_contexto, &proceso->registros_cpu.DX, sizeof(uint32_t)); 
    agregar_a_paquete(paquete_devolucion_contexto, &proceso->registros_cpu.EX, sizeof(uint32_t));
    agregar_a_paquete(paquete_devolucion_contexto, &proceso->registros_cpu.FX, sizeof(uint32_t));
    agregar_a_paquete(paquete_devolucion_contexto, &proceso->registros_cpu.HX, sizeof(uint32_t));
    agregar_a_paquete(paquete_devolucion_contexto, &proceso->registros_cpu.GX, sizeof(uint32_t));
    agregar_a_paquete(paquete_devolucion_contexto, &proceso->registros_cpu.base, sizeof(uint32_t));
    agregar_a_paquete(paquete_devolucion_contexto, &proceso->registros_cpu.limite, sizeof(uint32_t));
    enviar_paquete(paquete_devolucion_contexto, conexion); 
    eliminar_paquete(paquete_devolucion_contexto);

 }

 void solicitar_contexto_(t_proceso* proceso, int conexion){
    printf("entro a paquete_solicitud_contexto\n");
    t_paquete* paquete_solicitud_contexto;

    paquete_solicitud_contexto = crear_paquete(SOLICITUD_CONTEXTO); 
    agregar_a_paquete(paquete_solicitud_contexto, &proceso->pid,  sizeof(uint32_t));         
    agregar_a_paquete(paquete_solicitud_contexto, &proceso->tid,  sizeof(uint32_t));
    enviar_paquete(paquete_solicitud_contexto, conexion); 
    eliminar_paquete(paquete_solicitud_contexto);
 }

void deserializar_contexto_(t_proceso* proceso, t_list* lista_contexto){    
   //0 pid
   //1 tid
    proceso->registros_cpu.PC = *(uint32_t*)list_get(lista_contexto, 2);
    proceso->registros_cpu.AX = *(uint32_t*)list_get(lista_contexto, 3);
    proceso->registros_cpu.BX = *(uint32_t*)list_get(lista_contexto, 4);
    proceso->registros_cpu.CX = *(uint32_t*)list_get(lista_contexto, 5);
    proceso->registros_cpu.DX = *(uint32_t*)list_get(lista_contexto, 6);
    proceso->registros_cpu.EX = *(uint32_t*)list_get(lista_contexto, 7);
    proceso->registros_cpu.FX = *(uint32_t*)list_get(lista_contexto, 8);
    proceso->registros_cpu.GX = *(uint32_t*)list_get(lista_contexto, 9);
    proceso->registros_cpu.HX = *(uint32_t*)list_get(lista_contexto, 10);
    proceso->registros_cpu.base = *(uint32_t*)list_get(lista_contexto, 11);
    proceso->registros_cpu.limite = *(uint32_t*)list_get(lista_contexto, 12);

}   

void enviar_segfault_a_kernel(t_proceso* proceso,int conexion_kernel_dispatch){
    printf("entro a paquete_solicitud_contexto\n");
    t_paquete* paquete_segfault;

    paquete_segfault = crear_paquete(SEGMENTATION_FAULT); 
    agregar_a_paquete(paquete_segfault, &proceso->pid,  sizeof(uint32_t));         
    agregar_a_paquete(paquete_segfault, &proceso->tid,  sizeof(uint32_t));
    enviar_paquete(paquete_segfault, conexion_kernel_dispatch); 
    eliminar_paquete(paquete_segfault);
}  