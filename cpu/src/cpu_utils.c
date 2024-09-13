#include"../include/cpu_utils.h"

int tamanioParams;
int tamanioInterfaces;
t_pcb* proceso_actual;
t_registros_CPU* registros_cpu;
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
            
            break;
        }   
       
        case WRITE_MEM  :
        {
            log_info(logger_cpu, "PID: %u - Ejecutando: WRITE_MEM  - %s %s %s %s %s", proceso->pid,inst->param1,inst->param2,inst->param3,inst->param4,inst->param5); //LOG OBLIGATORIO
            
            break;
        }   


        case LOG:
        {
            log_info(logger_cpu, "PID: %u - Ejecutando: LOG", proceso->pid); //LOG OBLIGATORIO
           
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
    paquete_pedido_instruccion = crear_paquete(HANDSHAKE); // TODO: Crear codigo de operacion
        
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






uint32_t mmu(uint32_t direccion_logica, uint32_t base_particion, int conexion){
    uint32_t direccion_resultado;// = malloc(sizeof(uint32_t));
    bool encontro_en_tlb = false;
    uint32_t indice_encontrado; //= malloc(sizeof(uint32_t));
   // char* valor_direccion_logica = concatenar_cadenas(uint32_to_string(direccion_logica->nro_pag),uint32_to_string(direccion_logica->nro_pag));
 
    uint32_t desplazamiento = direccion_logica;// = malloc(sizeof(uint32_t));
 
    direccion_resultado = base_particion+desplazamiento ;

 log_info(logger_cpu, "PID: %u - ", proceso_actual->pid); //LOG OBLIGATORIO
            
	 

       
    //ver el caso en que me piden un tamaño que no entra en la pagina
    return direccion_resultado;
}



void read_mem(char* registro_datos, char* registro_direccion, t_pcb* proceso, t_log* logger, int conexion){
    // Lee el valor de memoria correspondiente a la Dirección Lógica que se encuentra en el 
    //Registro Dirección y lo almacena en el Registro Datos
    registros id_registro_direccion = identificarRegistro(registro_direccion);
    
    //uint32_t valor_registro_direccion = malloc(sizeof(uint32_t));
    uint32_t valor_registro_direccion = obtenerValorActualRegistro(id_registro_direccion,proceso);

    uint32_t dir_fisica_result = malloc(sizeof(uint32_t));
    dir_fisica_result = mmu(valor_registro_direccion,base,conexion);

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

void write_mem(char* registro_direccion, char* registro_datos, t_pcb* proceso, t_log* logger, int conexion){
    // Lee el valor del Registro Datos y lo escribe en la dirección física de
    // memoria obtenida a partir de la Dirección Lógica almacenada en el Registro Dirección.
    printf("registro: %s\n",registro_datos);
    registros id_registro_datos = identificarRegistro(registro_datos);
    printf("registro: %d\n",id_registro_datos);
    uint32_t valor_registro_datos = obtenerValorActualRegistro(id_registro_datos,proceso);
    printf("manda %d a guardar en mem\n",valor_registro_datos);
    registros id_registro_direccion = identificarRegistro(registro_direccion);
    uint32_t valor_registro_direccion = obtenerValorActualRegistro(id_registro_direccion,proceso);

    uint32_t dir_fisica_result = mmu(valor_registro_direccion,base,conexion);
    //TODO: Si el tamanio de valor_registro_datos(es un int de 32 siempre?) es mayor a tamanio_pagina hay
    //que dividir ambos y tomar el floor para obtener cant de paginas, con eso dividir datos a enviar en *cant de paginas*, y
    //por cada pedacito de intfo llamar a mmu y agregar dir fisca obtenida en lista 


// Calcular el tamaño necesario para el valor_str
    int len = snprintf(NULL, 0, "%u", valor_registro_datos);
    char* valor_str = malloc(len + 1);
    snprintf(valor_str, len + 1, "%u", valor_registro_datos);

    
    log_info(logger, "PID: %u - Acción: ESCRIBIR - Dirección Física: %u - Valor: %u", proceso_actual->pid,dir_fisica_result,valor_registro_datos); //LOG OBLIGATORIO

}



void mutex_lock_inst(char* recurso, int conexion_kernel){
    // Esta instrucción solicita al Kernel que se asigne una instancia del recurso
    //indicado por parámetro.

    solicitar_mutex_lock_kernel(proceso_actual,(strlen(recurso) + 1) * sizeof(char),recurso, conexion_kernel); 
}

void mutex_unlock_inst(char* recurso, int conexion_kernel){
    //Esta instrucción solicita al Kernel que se libere una instancia del recurso
    //indicado por parámetro
    solicitar_mutex_unlock_kernel(proceso_actual,(strlen(recurso) + 1) * sizeof(char) ,recurso, conexion_kernel);
}




void pedir_valor_a_memoria(uint32_t dir_fisica, uint32_t pid, uint32_t tamanio, int conexion){
        printf("entro a pedir_valor_a_memoria\n");
        t_paquete* paquete_pedido_valor_memoria;
        paquete_pedido_valor_memoria = (HANDSHAKE); // TODO: Crear codigo de operacion

        agregar_a_paquete(paquete_pedido_valor_memoria,  &pid,  sizeof(uint32_t));     
        agregar_a_paquete(paquete_pedido_valor_memoria,  &dir_fisica,  sizeof(uint32_t)); 
        agregar_a_paquete(paquete_pedido_valor_memoria,  &tamanio,  sizeof(uint32_t));
         
            
        enviar_paquete(paquete_pedido_valor_memoria, conexion); 
        free(paquete_pedido_valor_memoria->buffer->stream);
        free(paquete_pedido_valor_memoria->buffer);
        free(paquete_pedido_valor_memoria);

}






void solicitar_mutex_lock_kernel(t_pcb* pcb,uint32_t recurso_tamanio ,char* recurso, int conexion_kernel){
        printf("entro a solicitar_wait_kernel\n");
        
        t_paquete* paquete_wait_kernel;
   
        paquete_wait_kernel = crear_paquete(HANDSHAKE); // TODO: Crear codigo de operacion
        agregar_a_paquete(paquete_wait_kernel,  &pcb->pid,  sizeof(uint32_t));         
        
        enviar_paquete(paquete_wait_kernel, conexion_kernel); 
        eliminar_paquete(paquete_wait_kernel);

}

void solicitar_mutex_unlock_kernel(t_pcb* pcb,uint32_t recurso_tamanio,char* recurso, int conexion_kernel){
        printf("entro a solicitar_wait_kernel\n");
        t_paquete* paquete_signal_kernel;
   
        paquete_signal_kernel = crear_paquete(HANDSHAKE); // TODO: Crear codigo de operacion
        
        agregar_a_paquete(paquete_signal_kernel,  &pcb->pid,  sizeof(uint32_t));      
        
        enviar_paquete(paquete_signal_kernel, conexion_kernel); 

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

void obtener_base_particion(int conexion, t_pcb* pcb){ // PCB O PROCESS ID?
    printf("entro a obtener base particion\n");
    t_paquete* paquete_pedido_tamanio_pag;
    paquete_pedido_tamanio_pag = crear_paquete(BASE_PARTICION); // TODO: Crear codigo de operacion
    //AGREGAR A PAQUETE
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
    if (tipo_inst != HANDSHAKE) //TODO: Crear tipo de instruccion
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

tipo_instruccion str_to_tipo_instruccion(const char *str) {

    printf("Entro al funcion a testear \n");
    tipo_instruccion instruccion_a_devolver = -1;
    if (strcmp(str, "SET") == 0) instruccion_a_devolver = SET;
    else if (strcmp(str, "SUM") == 0) instruccion_a_devolver = SUM;
    else if (strcmp(str, "SUB") == 0) instruccion_a_devolver = SUB;
    else if (strcmp(str, "JNZ") == 0) instruccion_a_devolver = JNZ;
    // agregar instrucciones faltantese
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