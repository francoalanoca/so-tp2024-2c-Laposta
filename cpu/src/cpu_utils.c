#include"../include/cpu_utils.h"

int tamanioParams;
int tamanioInterfaces;

instr_t* fetch(int conexion, t_pcb* proceso){
    log_info(logger_cpu, "Voy a entrar a pedir_instruccion");
    log_info(logger_cpu, "PID: %u- FETCH- Program Counter: %u", proceso->pid,proceso->program_counter); //LOG OBLIGATORIO
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


void execute(instr_t* inst,tipo_instruccion tipo_inst, t_pcb* proceso, int conexion,t_list* tlb,  int socket_dispatch, int socket_interrupt){
    
    switch(tipo_inst){
        case SET:
        {   log_info(logger_cpu, "ENTRO EN SET ");
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

        case JNZ:
        {
            log_info(logger_cpu, "PID: %u - Ejecutando: JNZ - %s %s", proceso->pid,inst->param1,inst->param2); //LOG OBLIGATORIO
            jnz(inst->param1, inst->param2,proceso);
            break;
        }        
        
        case READ_MEM :
        {
            log_info(logger_cpu, "PID: %u - Ejecutando: READ_MEM - %s %s %s %s %s", proceso->pid,inst->param1,inst->param2,inst->param3,inst->param4,inst->param5); //LOG OBLIGATORIO
            io_fs_write(inst->param1,inst->param2,inst->param3,inst->param4,inst->param5,proceso,logger_cpu,conexion,tlb,socket_dispatch);
            break;
        }   
       
        case WRITE_MEM  :
        {
            log_info(logger_cpu, "PID: %u - Ejecutando: WRITE_MEM  - %s %s %s %s %s", proceso->pid,inst->param1,inst->param2,inst->param3,inst->param4,inst->param5); //LOG OBLIGATORIO
            io_fs_write(inst->param1,inst->param2,inst->param3,inst->param4,inst->param5,proceso,logger_cpu,conexion,tlb,socket_dispatch);
            break;
        }   


        case LOG:
        {
            log_info(logger_cpu, "PID: %u - Ejecutando: LOG", proceso->pid); //LOG OBLIGATORIO
            exit_inst(socket_dispatch);
            break;
        }



        default:
        	log_warning(logger_cpu, "Huvo un error: instruccion no encontrada");
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

void pedir_instruccion(t_pcb* proceso,int conexion){
  
    
    t_paquete* paquete_pedido_instruccion;
    paquete_pedido_instruccion = crear_paquete(PROXIMA_INSTRUCCION); 
        
    agregar_a_paquete(paquete_pedido_instruccion,  &proceso->pid,  sizeof(uint32_t)); 
    agregar_a_paquete(paquete_pedido_instruccion,  &proceso->program_counter,  sizeof(uint32_t));  
        
    enviar_paquete(paquete_pedido_instruccion, conexion); 
    eliminar_paquete(paquete_pedido_instruccion);
}

void set(char* registro, uint32_t valor, t_pcb* proceso){
    //printf("El valor del set es : %d ", valor);
    registros registro_elegido = identificarRegistro(registro);
    //pthread_mutex_lock(&mutex_proceso_actual);
    switch(registro_elegido){
        case PC:
        {
          registros_cpu->PC = valor;
            break;
        }
        case AX:
        {
          registros_cpu->AX = valor;
            break;
        }
        case BX:
        {
          registros_cpu->BX = valor;
            break;
        }
        case CX:
        {
          registros_cpu->CX = valor;
            break;
        }
        case DX:
        {
          registros_cpu->DX = valor;
            break;
        }
        case EX:
        {
          registros_cpu->EX = valor;
            break;
        }
        case FX:
        {
          registros_cpu->FX = valor;
            break;
        }
        case GX:
        {
          registros_cpu->GX = valor;
            break;
        }
        case HX:
        {
          registros_cpu->HX = valor;
            break;
        }
        
        default:
        log_info(logger_cpu, "El registro no existe");
    }
   // pthread_mutex_unlock(&mutex_proceso_actual);

    //proceso->pcb->registros_cpu.AX;
   // registro = valor;
}

void sum(char* registro_destino, char* registro_origen, t_pcb* proceso){
    registros id_registro_destino = identificarRegistro(registro_destino);
    registros id_registro_origen = identificarRegistro(registro_origen);

    uint32_t valor_reg_destino = obtenerValorActualRegistro(id_registro_destino,proceso);
    uint32_t valor_reg_origen = obtenerValorActualRegistro(id_registro_origen,proceso);
    pthread_mutex_lock(&mutex_proceso_actual);
    switch(id_registro_destino){
        case PC:
        {
           registros_cpu->PC = valor_reg_destino + valor_reg_origen;
            break;
        }
        case AX:
        {
           registros_cpu->AX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case BX:
        {
           registros_cpu->BX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case CX:
        {
           registros_cpu->CX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case DX:
        {
           registros_cpu->DX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case EX:
        {
           registros_cpu->EX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case FX:
        {
           registros_cpu->FX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case GX:
        {
           registros_cpu->GX = valor_reg_destino + valor_reg_origen;
            break;
        }
        case HX:
        {
           registros_cpu->HX = valor_reg_destino + valor_reg_origen;
            break;
        }
        
        default:
        log_info(logger_cpu, "El registro no existe");
    }
    pthread_mutex_unlock(&mutex_proceso_actual);


    //registro_destino = registro_destino + registro_origen;
}


void jnz(char* registro, uint32_t inst, t_pcb* proceso){
    registros id_registro = identificarRegistro(registro);
    uint32_t valor_registro = obtenerValorActualRegistro(id_registro,proceso);
    if(valor_registro != 0){
        pthread_mutex_lock(&mutex_proceso_actual);
        proceso->program_counter = inst;
        pthread_mutex_unlock(&mutex_proceso_actual);
    }
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
    else if(strcmp(registro,"SI") == 0){
        return HX;
    }
    else{
        return REG_NO_ENC;
    }
}

uint32_t obtenerValorActualRegistro(registros id_registro, t_pcb* proceso){
    switch(id_registro){
        case PC:
        {
           return registros_cpu->PC;
            break;
        }
        case AX:
        {
           return registros_cpu->AX;
            break;
        }
        case BX:
        {
           return registros_cpu->BX;
            break;
        }
        case CX:
        {
           return registros_cpu->CX;
            break;
        }
        case DX:
        {
           return registros_cpu->DX;
            break;
        }
        case EX:
        {
           return registros_cpu->EX;
            break;
        }
        case FX:
        {
           return registros_cpu->FX;
            break;
        }
        case GX:
        {
           return registros_cpu->GX;
            break;
        }
        case HX:
        {
           return registros_cpu->HX;
            break;
        }
      
        default:
        log_info(logger_cpu, "El registro no existe");
    }
}






uint32_t mmu(uint32_t direccion_logica, uint32_t tamanio_pag, int conexion, t_log* logger,t_list* tlb){
    uint32_t direccion_resultado;// = malloc(sizeof(uint32_t));
    bool encontro_en_tlb = false;
    uint32_t indice_encontrado; //= malloc(sizeof(uint32_t));
   // char* valor_direccion_logica = concatenar_cadenas(uint32_to_string(direccion_logica->nro_pag),uint32_to_string(direccion_logica->nro_pag));
    uint32_t nro_pagina;// = malloc(sizeof(uint32_t));
    uint32_t desplazamiento;// = malloc(sizeof(uint32_t));
 //CALCULAR NRO DE PAGINA Y DESPLAZAMIENTO
    nro_pagina =  floor(direccion_logica / tamanio_pag);
    desplazamiento = direccion_logica - nro_pagina * tamanio_pag;

 log_info(logger, "PID: %u - TLB HIT- Pagina: %u", proceso_actual->pid,nro_pagina); //LOG OBLIGATORIO
            
	 

       
    //ver el caso en que me piden un tamaño que no entra en la pagina
    return direccion_resultado;
}






void pedir_marco_a_memoria(uint32_t pid, uint32_t nro_pagina, int conexion){
    printf("entro a pedir_marco_a_memoria\n");
    t_paquete* paquete_pedido_marco;
    paquete_pedido_marco = crear_paquete(PEDIDO_MARCO_A_MEMORIA); 
        
    agregar_a_paquete(paquete_pedido_marco,  &pid,  sizeof(uint32_t)); 
    agregar_a_paquete(paquete_pedido_marco,  &nro_pagina,  sizeof(uint32_t));  
        
    enviar_paquete(paquete_pedido_marco, conexion); 
    eliminar_paquete(paquete_pedido_marco);
}


void read_mem(char* registro_datos, char* registro_direccion, t_pcb* proceso, t_log* logger, int conexion, t_list* tlb){
    // Lee el valor de memoria correspondiente a la Dirección Lógica que se encuentra en el 
    //Registro Dirección y lo almacena en el Registro Datos
    registros id_registro_direccion = identificarRegistro(registro_direccion);
    
    //uint32_t valor_registro_direccion = malloc(sizeof(uint32_t));
    uint32_t valor_registro_direccion = obtenerValorActualRegistro(id_registro_direccion,proceso);

    uint32_t dir_fisica_result = malloc(sizeof(uint32_t));
    dir_fisica_result = mmu(valor_registro_direccion,tamanio_pagina,conexion,logger,tlb);

    registros id_registro_datos = identificarRegistro(registro_datos);

    uint32_t tamanio_a_leer = obtenerTamanioRegistro(id_registro_datos);

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

void write_mem(char* registro_direccion, char* registro_datos, t_pcb* proceso, t_log* logger, int conexion, t_list* tlb){
    // Lee el valor del Registro Datos y lo escribe en la dirección física de
    // memoria obtenida a partir de la Dirección Lógica almacenada en el Registro Dirección.
    printf("registro: %s\n",registro_datos);
    registros id_registro_datos = identificarRegistro(registro_datos);
    printf("registro: %d\n",id_registro_datos);
    uint32_t valor_registro_datos = obtenerValorActualRegistro(id_registro_datos,proceso);
    printf("manda %d a guardar en mem\n",valor_registro_datos);
    registros id_registro_direccion = identificarRegistro(registro_direccion);
    uint32_t valor_registro_direccion = obtenerValorActualRegistro(id_registro_direccion,proceso);

    uint32_t dir_fisica_result = mmu(valor_registro_direccion,tamanio_pagina,conexion,logger,tlb);
    //TODO: Si el tamanio de valor_registro_datos(es un int de 32 siempre?) es mayor a tamanio_pagina hay
    //que dividir ambos y tomar el floor para obtener cant de paginas, con eso dividir datos a enviar en *cant de paginas*, y
    //por cada pedacito de intfo llamar a mmu y agregar dir fisca obtenida en lista 


// Calcular el tamaño necesario para el valor_str
    int len = snprintf(NULL, 0, "%u", valor_registro_datos);
    char* valor_str = malloc(len + 1);
    snprintf(valor_str, len + 1, "%u", valor_registro_datos);

    guardar_en_direccion_fisica(dir_fisica_result, len + 1, valor_str, proceso->pid, conexion);
    log_info(logger, "PID: %u - Acción: ESCRIBIR - Dirección Física: %u - Valor: %u", proceso_actual->pid,dir_fisica_result,valor_registro_datos); //LOG OBLIGATORIO

}



void wait_inst(char* recurso, int conexion_kernel){
    // Esta instrucción solicita al Kernel que se asigne una instancia del recurso
    //indicado por parámetro.

    solicitar_wait_kernel(proceso_actual,(strlen(recurso) + 1) * sizeof(char),recurso, conexion_kernel); 
}

void signal_inst(char* recurso, int conexion_kernel){
    //Esta instrucción solicita al Kernel que se libere una instancia del recurso
    //indicado por parámetro
    solicitar_signal_kernel(proceso_actual,(strlen(recurso) + 1) * sizeof(char) ,recurso, conexion_kernel);
}


void exit_inst( int conexion_kernel){
    // Esta instrucción representa la syscall de finalización del proceso. Se deberá devolver el
    //Contexto de Ejecución actualizado al Kernel para su finalización.
    log_info(logger_cpu, "Entro a exit_inst pid :%d", proceso_actual->pid); 
    
    pthread_mutex_lock(&mutex_proceso_interrumpido_actual);
/*ANTERIOR
    //proceso_interrumpido_actual->pcb->pid = proceso_actual->pid;
    t_proceso_interrumpido* proceso_interrumpido_a_enviar = malloc(sizeof(t_proceso_interrumpido));
    proceso_interrumpido_a_enviar->pcb=malloc(sizeof(t_pcb));
    proceso_interrumpido_a_enviar->pcb->path = malloc(proceso_actual->path_length);

    proceso_interrumpido_a_enviar->pcb = proceso_actual;
    log_info(logger_cpu, "Path del proc actual :%s", proceso_actual->path); 
    strcpy(proceso_interrumpido_a_enviar->pcb->path, proceso_actual->path);
    //proceso_interrumpido_actual->pcb = proceso_actual;
    proceso_interrumpido_a_enviar->motivo_interrupcion = INSTRUCCION_EXIT;*/

    t_proceso_interrumpido *proceso_interrumpido_actual = malloc(sizeof(t_proceso_interrumpido));
    proceso_interrumpido_actual->pcb = proceso_actual;
    proceso_interrumpido_actual->pcb->path = malloc(proceso_actual->path_length);   
    proceso_interrumpido_actual->interfaz = "none";
    strcpy(proceso_interrumpido_actual->pcb->path, proceso_actual->path);

    log_info(logger_cpu, "Pid asignado en proceo de interrupcion pid :%d", proceso_interrumpido_actual->pcb->pid ); 
    log_info(logger_cpu, "Pid asignado en proceo de interrupcion path :%s", proceso_interrumpido_actual->pcb->path );
    proceso_interrumpido_actual->motivo_interrupcion = INSTRUCCION_EXIT;
    pthread_mutex_unlock(&mutex_proceso_interrumpido_actual);
    solicitar_exit_a_kernel(proceso_interrumpido_actual, conexion_kernel);
   //solicitar_exit_a_kernel(proceso_interrumpido_a_enviar);
    pthread_mutex_lock(&mutex_proceso_actual);
    proceso_actual = NULL;
    pthread_mutex_unlock(&mutex_proceso_actual);
    pthread_mutex_lock(&mutex_proceso_interrumpido_actual);
    proceso_interrumpido_actual = NULL;
    pthread_mutex_unlock(&mutex_proceso_interrumpido_actual);
}

void pedir_valor_a_memoria(uint32_t dir_fisica, uint32_t pid, uint32_t tamanio, int conexion){
        printf("entro a pedir_valor_a_memoria\n");
        t_paquete* paquete_pedido_valor_memoria;
        paquete_pedido_valor_memoria = crear_paquete(PETICION_VALOR_MEMORIA); 

        agregar_a_paquete(paquete_pedido_valor_memoria,  &pid,  sizeof(uint32_t));     
        agregar_a_paquete(paquete_pedido_valor_memoria,  &dir_fisica,  sizeof(uint32_t)); 
        agregar_a_paquete(paquete_pedido_valor_memoria,  &tamanio,  sizeof(uint32_t));
         
            
        enviar_paquete(paquete_pedido_valor_memoria, conexion); 
        free(paquete_pedido_valor_memoria->buffer->stream);
        free(paquete_pedido_valor_memoria->buffer);
        free(paquete_pedido_valor_memoria);

}




void envia_error_de_memoria_a_kernel(t_proceso_interrumpido* proceso, int conexion_kernel){
        printf("entro a envia_error_de_memoria_a_kernel\n");
        t_paquete* paquete_error_memoria;
   
    paquete_error_memoria = crear_paquete(INTERRUPCION_CPU); 
    
    agregar_a_paquete(paquete_error_memoria,  &proceso->pcb->pid,  sizeof(uint32_t));         
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->program_counter, sizeof(uint32_t));  
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->path_length, sizeof(uint32_t)); 
    agregar_a_paquete(paquete_error_memoria, proceso->pcb->path, proceso->pcb->path_length);
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->registros_cpu.PC, sizeof(uint32_t));
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->registros_cpu.AX, sizeof(uint32_t)); //VER TAMANIO
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->registros_cpu.BX, sizeof(uint32_t)); //VER TAMANIO
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->registros_cpu.CX, sizeof(uint32_t)); //VER TAMANIO
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->registros_cpu.DX, sizeof(uint32_t)); //VER TAMANIO
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->registros_cpu.EAX, sizeof(uint32_t));
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->registros_cpu.EBX, sizeof(uint32_t));
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->registros_cpu.ECX, sizeof(uint32_t));
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->registros_cpu.EDX, sizeof(uint32_t));
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->registros_cpu.SI, sizeof(uint32_t));
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->registros_cpu.DI, sizeof(uint32_t));
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->estado, sizeof(uint32_t));
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->tiempo_ejecucion, sizeof(uint32_t));
    agregar_a_paquete(paquete_error_memoria, &proceso->pcb->quantum, sizeof(uint32_t));
    agregar_a_paquete(paquete_error_memoria, &proceso->motivo_interrupcion, sizeof(uint32_t));
       
    enviar_paquete(paquete_error_memoria, conexion_kernel); 
    free(paquete_error_memoria->buffer->stream);
    free(paquete_error_memoria->buffer);
    free(paquete_error_memoria);
   
}




void solicitar_wait_kernel(t_pcb* pcb,uint32_t recurso_tamanio ,char* recurso, int conexion_kernel){
        printf("entro a solicitar_wait_kernel\n");
        
        t_paquete* paquete_wait_kernel;
   
        paquete_wait_kernel = crear_paquete(ENVIO_WAIT_A_KERNEL); 
        agregar_a_paquete(paquete_wait_kernel,  &pcb->pid,  sizeof(uint32_t));         
        agregar_a_paquete(paquete_wait_kernel, &pcb->program_counter, sizeof(uint32_t));  
        agregar_a_paquete(paquete_wait_kernel, &pcb->path_length, sizeof(uint32_t)); 
        agregar_a_paquete(paquete_wait_kernel, pcb->path, pcb->path_length);
        agregar_a_paquete(paquete_wait_kernel, &pcb->registros_cpu.PC, sizeof(uint32_t));
        agregar_a_paquete(paquete_wait_kernel, &pcb->registros_cpu.AX, sizeof(uint32_t)); //VER TAMANIO
        agregar_a_paquete(paquete_wait_kernel, &pcb->registros_cpu.BX, sizeof(uint32_t)); //VER TAMANIO
        agregar_a_paquete(paquete_wait_kernel, &pcb->registros_cpu.CX, sizeof(uint32_t)); //VER TAMANIO
        agregar_a_paquete(paquete_wait_kernel, &pcb->registros_cpu.DX, sizeof(uint32_t)); //VER TAMANIO
        agregar_a_paquete(paquete_wait_kernel, &pcb->registros_cpu.EAX, sizeof(uint32_t));
        agregar_a_paquete(paquete_wait_kernel, &pcb->registros_cpu.EBX, sizeof(uint32_t));
        agregar_a_paquete(paquete_wait_kernel, &pcb->registros_cpu.ECX, sizeof(uint32_t));
        agregar_a_paquete(paquete_wait_kernel, &pcb->registros_cpu.EDX, sizeof(uint32_t));
        agregar_a_paquete(paquete_wait_kernel, &pcb->registros_cpu.SI, sizeof(uint32_t));
        agregar_a_paquete(paquete_wait_kernel, &pcb->registros_cpu.DI, sizeof(uint32_t));
        agregar_a_paquete(paquete_wait_kernel, &pcb->estado, sizeof(uint32_t));
        agregar_a_paquete(paquete_wait_kernel, &pcb->tiempo_ejecucion, sizeof(uint32_t));
        agregar_a_paquete(paquete_wait_kernel, &pcb->quantum, sizeof(uint32_t));        
        agregar_a_paquete(paquete_wait_kernel, &recurso_tamanio, sizeof(uint32_t));
        agregar_a_paquete(paquete_wait_kernel, recurso, recurso_tamanio);
        
        enviar_paquete(paquete_wait_kernel, conexion_kernel); 
        free(paquete_wait_kernel->buffer->stream);
        free(paquete_wait_kernel->buffer);
        free(paquete_wait_kernel);

}

void solicitar_signal_kernel(t_pcb* pcb,uint32_t recurso_tamanio,char* recurso, int conexion_kernel){
        printf("entro a solicitar_wait_kernel\n");
        t_paquete* paquete_signal_kernel;
   
        paquete_signal_kernel = crear_paquete(ENVIO_SIGNAL_A_KERNEL); 
        
        agregar_a_paquete(paquete_signal_kernel,  &pcb->pid,  sizeof(uint32_t));         
        agregar_a_paquete(paquete_signal_kernel, &pcb->program_counter, sizeof(uint32_t));  
        agregar_a_paquete(paquete_signal_kernel, &pcb->path_length, sizeof(uint32_t)); 
        agregar_a_paquete(paquete_signal_kernel, pcb->path, pcb->path_length);
        agregar_a_paquete(paquete_signal_kernel, &pcb->registros_cpu.PC, sizeof(uint32_t));
        agregar_a_paquete(paquete_signal_kernel, &pcb->registros_cpu.AX, sizeof(uint32_t)); //VER TAMANIO
        agregar_a_paquete(paquete_signal_kernel, &pcb->registros_cpu.BX, sizeof(uint32_t)); //VER TAMANIO
        agregar_a_paquete(paquete_signal_kernel, &pcb->registros_cpu.CX, sizeof(uint32_t)); //VER TAMANIO
        agregar_a_paquete(paquete_signal_kernel, &pcb->registros_cpu.DX, sizeof(uint32_t)); //VER TAMANIO
        agregar_a_paquete(paquete_signal_kernel, &pcb->registros_cpu.EAX, sizeof(uint32_t));
        agregar_a_paquete(paquete_signal_kernel, &pcb->registros_cpu.EBX, sizeof(uint32_t));
        agregar_a_paquete(paquete_signal_kernel, &pcb->registros_cpu.ECX, sizeof(uint32_t));
        agregar_a_paquete(paquete_signal_kernel, &pcb->registros_cpu.EDX, sizeof(uint32_t));
        agregar_a_paquete(paquete_signal_kernel, &pcb->registros_cpu.SI, sizeof(uint32_t));
        agregar_a_paquete(paquete_signal_kernel, &pcb->registros_cpu.DI, sizeof(uint32_t));
        agregar_a_paquete(paquete_signal_kernel, &pcb->estado, sizeof(uint32_t));
        agregar_a_paquete(paquete_signal_kernel, &pcb->tiempo_ejecucion, sizeof(uint32_t));
        agregar_a_paquete(paquete_signal_kernel, &pcb->quantum, sizeof(uint32_t));        
        agregar_a_paquete(paquete_signal_kernel, &recurso_tamanio, sizeof(uint32_t));
        agregar_a_paquete(paquete_signal_kernel, recurso, recurso_tamanio);
        
        enviar_paquete(paquete_signal_kernel, conexion_kernel); 
         sem_wait(&sem_interrupcion_kernel);
       eliminar_paquete(paquete_signal_kernel);
}




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
void solicitar_exit_a_kernel(t_proceso_interrumpido* proceso, int conexion_kernel){
        printf("entro a solicitar_exit_a_kernel\n");
        t_paquete* paquete_exit_kernel = crear_paquete(INTERRUPCION_CPU); 
        printf("CREO EL PAQUETE\n");
       // proceso->pcb->path = malloc(proceso->pcb->path_length);
       // strcpy(proceso->pcb->path,"PATHPRUEBA");
        printf("PID: %u,PATH:%s MOTIVO:%u\n",proceso->pcb->pid,proceso->pcb->path,proceso->motivo_interrupcion);
        printf("MOTIVO:%u\n",proceso->motivo_interrupcion);
        agregar_a_paquete(paquete_exit_kernel,  &proceso->pcb->pid,  sizeof(uint32_t));         
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->program_counter, sizeof(uint32_t));  
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->path_length, sizeof(uint32_t)); 
        agregar_a_paquete(paquete_exit_kernel, proceso->pcb->path, proceso->pcb->path_length);
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->registros_cpu.PC, sizeof(uint32_t));
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->registros_cpu.AX, sizeof(uint32_t)); //VER TAMANIO
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->registros_cpu.BX, sizeof(uint32_t)); //VER TAMANIO
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->registros_cpu.CX, sizeof(uint32_t)); //VER TAMANIO
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->registros_cpu.DX, sizeof(uint32_t)); //VER TAMANIO
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->registros_cpu.EAX, sizeof(uint32_t));
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->registros_cpu.EBX, sizeof(uint32_t));
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->registros_cpu.ECX, sizeof(uint32_t));
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->registros_cpu.EDX, sizeof(uint32_t));
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->registros_cpu.SI, sizeof(uint32_t));
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->registros_cpu.DI, sizeof(uint32_t));
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->estado, sizeof(uint32_t));
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->tiempo_ejecucion, sizeof(uint32_t));
        agregar_a_paquete(paquete_exit_kernel, &proceso->pcb->quantum, sizeof(uint32_t));
        agregar_a_paquete(paquete_exit_kernel, &proceso->motivo_interrupcion, sizeof(uint32_t));
        imprimir_contenido_paquete(paquete_exit_kernel);
        enviar_paquete(paquete_exit_kernel, conexion_kernel); 
        eliminar_paquete(paquete_exit_kernel);
    
}


void obtenerTamanioPagina(int conexion){
    printf("entro a obtenerTamanioPagina\n");
    t_paquete* paquete_pedido_tamanio_pag;
    paquete_pedido_tamanio_pag = crear_paquete(SOLICITUD_TAMANIO_PAGINA); 
    enviar_paquete(paquete_pedido_tamanio_pag, conexion); 
    eliminar_paquete(paquete_pedido_tamanio_pag);

}


 

void ciclo_de_instrucciones(int *conexion_mer, t_pcb *proceso, t_list *tlb, int *socket_dispatch, int*socket_dispatch_interrupciones ,int *socket_interrupt)
{   log_info(logger_cpu, "Entro al ciclo");
    int conexion_mem = *conexion_mer;
    int dispatch = *socket_dispatch;
    int interrupt = *socket_interrupt;
    int dispatch_interrupt = *socket_dispatch_interrupciones;
    //free(conexion_mer);
    //free(socket_dispatch);
    //free(socket_interrupt);
    
    log_info(logger_cpu, "Entro al ciclo");
    log_info(logger_cpu, "TLB size ciclo: %d\n", list_size(tlb));
    instr_t *inst = malloc(sizeof(instr_t));
    log_info(logger_cpu, "Voy a entrar a fetch");
    inst = fetch(conexion_mem,proceso);
    tipo_instruccion tipo_inst;
    log_info(logger_cpu, "Voy a entrar a decode");
    tipo_inst = decode(inst);
    log_info(logger_cpu, "Voy a entrar a execute");
    execute(inst, tipo_inst, proceso, conexion_mem, tlb, dispatch, interrupt);
    if (tipo_inst != EXIT)
    {
        proceso_actual->program_counter += 1;
    }
    log_info(logger_cpu, "Voy a entrar a check_interrupt");
    check_interrupt(dispatch_interrupt);
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
