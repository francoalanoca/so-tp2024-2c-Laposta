#include "../include/protocolo.h"




/*---------------------------- CPU-------------------------*/

void memoria_atender_cpu(){
    
	log_info(logger_memoria, "EMPIEZO A ATENDER CPU(%d)",socket_cpu);


	while (1) {

        //Se queda esperando a que Cpu le envie algo y extrae el cod de operacion
		int cod_op = recibir_operacion(socket_cpu);
		op_code response;
		t_list* valores =  malloc(sizeof(t_list));


		switch (cod_op) {
		case HANDSHAKE:
			log_info(logger_memoria, "Handshake realizado con cliente cpu(%d)",socket_cpu);
            response = HANDSHAKE_OK;
            if (send(socket_cpu, &response, sizeof(uint32_t), MSG_WAITALL) != sizeof(uint32_t)) {
                log_error(logger_memoria, "Error al enviar respuesta de handshake a cliente");
                   
                break;
            }
            break;
		
		case SOLICITUD_CONTEXTO:
			usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
			log_info(logger_memoria, "Recibí SOLICITUD_CONTEXTO \n");
			valores = recibir_paquete(socket_cpu);
			uint32_t pid = *(uint32_t*)list_get(valores, 0);
			uint32_t tid = *(uint32_t*)list_get(valores, 1);
			t_m_contexto* contexto_encontrado = buscar_contexto_en_lista(pid,tid);
			contexto_encontrado->pid = pid;
			contexto_encontrado->tid = tid;
			log_warning(logger_memoria,"contexto encontrado: registros");
			log_warning(logger_memoria,"PC=%d",contexto_encontrado->registros.PC);
			log_warning(logger_memoria,"AX=%d",contexto_encontrado->registros.AX);
			log_warning(logger_memoria,"BX=%d",contexto_encontrado->registros.BX);
			enviar_respuesta_contexto(contexto_encontrado,socket_cpu);
			log_info(logger_memoria, "## Contexto Solicitado - (PID:TID) - (%d:%d)\n",pid,tid);
			free(contexto_encontrado);
			break;

		case SOLICITUD_INSTRUCCION:
			//sleep(8);
			usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
			log_info(logger_memoria, "Recibí SOLICITUD_INSTRUCCION \n");
			valores = recibir_paquete(socket_cpu);
			t_proceso_memoria* solicitud_instruccion = deserializar_solicitud_instruccion(valores);         
            char* instruccion = buscar_instruccion(solicitud_instruccion->pid, solicitud_instruccion->tid, solicitud_instruccion->program_counter);
			log_info(logger_memoria, "## Obtener instrucción - (PID:TID) - (%d:%d) - Instrucción: %s\n",solicitud_instruccion->pid,solicitud_instruccion->tid,instruccion); 
			
			enviar_respuesta_instruccion(instruccion, socket_cpu);    
			log_info(logger_memoria, "enviada respuesta de SOLICITUD_INSTRUCCION_RTA \n");
			break;

		case READ_MEMORIA:
			usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
			log_info(logger_memoria, "Recibí READ_MEMORIA \n");
			valores = recibir_paquete(socket_cpu);
			t_escribir_leer* peticion_leer = deserializar_read_memoria(valores); 
			uint32_t tamanio_rta = 4;    
			log_info(logger_memoria, "ANTES DE MEMSET \n");
            uint32_t respuesta_leer = -1;

			log_info(logger_memoria, "ANTES DE FUNCION READ_MEM \n");
			//INICIO MUTEX
			if(read_mem(peticion_leer->direccion_fisica,&respuesta_leer)){
				log_info(logger_memoria, "RESPUESTA READ: %d",respuesta_leer);
				enviar_respuesta_read_memoria(peticion_leer->pid,respuesta_leer, socket_cpu,READ_MEMORIA_RTA_OK);
				log_info(logger_memoria, "## Lectura - (PID:TID) - (%d:%d) - Dir. Física: %d - Tamaño: %d \n",peticion_leer->pid,peticion_leer->tid,peticion_leer->direccion_fisica,peticion_leer->tamanio); 
				
			}
			else{

				enviar_respuesta_read_memoria(peticion_leer->pid,respuesta_leer, socket_cpu,READ_MEMORIA_RTA_ERROR);
			}
			//FIN MUTEX
			//usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
			//enviar_respuesta_read_memoria(respuesta_leer, socket_cpu);
			log_info(logger_memoria, "enviada respuesta de READ_MEMORIA_RTA \n");
			break;

		case WRITE_MEMORIA:
			usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
			log_info(logger_memoria, "Recibí WRITE_MEMORIA \n");
			valores = recibir_paquete(socket_cpu);
			t_escribir_leer* peticion_escribir = deserializar_write_memoria(valores);  
			//INICIO MUTEX 
			if(write_mem(peticion_escribir->direccion_fisica, peticion_escribir->valor)){
				
				enviar_respuesta_write_memoria(peticion_escribir->pid, socket_cpu,WRITE_MEMORIA_RTA_OK);
				log_info(logger_memoria, "## Escritura - (PID:TID) - (%d:%d) - Dir. Física: %d - Tamaño: %d\n",peticion_escribir->pid,peticion_escribir->tid,peticion_escribir->direccion_fisica,4);
			}
			else{
				enviar_respuesta_write_memoria(peticion_escribir->pid, socket_cpu,WRITE_MEMORIA_RTA_ERROR);
			}
			//FIN MUTEX
            //char* respuesta_escribir = escribir_memoria(peticion_escribir->pid, peticion_escribir->direccion_fisica, peticion_escribir->valor, peticion_escribir->tamanio);
			//usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
			//enviar_respuesta_write_memoria(respuesta_escribir, socket_cpu);
			log_info(logger_memoria, "enviada respuesta de WRITE_MEMORIA_RTA \n");
			break;
		
		case DEVOLUCION_CONTEXTO:
			usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
			log_info(logger_memoria, "Recibí DEVOLUCION_CONTEXTO \n");
			valores = recibir_paquete(socket_cpu);
			t_m_contexto* contexto_actualizado = malloc(sizeof(t_m_contexto));
			//t_m_contexto* contexto_actualizado = deserializar_contexto(valores);
			deserializar_contexto(contexto_actualizado,valores);
			printf("Entrare a actualizar_contexto\n");
			printf("Pid contexto_actualizado:%d\n",contexto_actualizado->pid);
			if(actualizar_contexto(contexto_actualizado)){
				enviar_respuesta_actualizar_contexto(contexto_actualizado,socket_cpu,DEVOLUCION_CONTEXTO_RTA_OK);
				log_info(logger_memoria, "## Contexto Actualizado - (PID:TID) - (%d:%d)\n",contexto_actualizado->pid,contexto_actualizado->tid);
			}
			else{
				printf("No se encontro el pid/tid\n");
				enviar_respuesta_actualizar_contexto(contexto_actualizado,socket_cpu,DEVOLUCION_CONTEXTO_RTA_ERROR);
			}
			//usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
			log_info(logger_memoria, "enviada respuesta de DEVOLUCION_CONTEXTO_RTA \n");
			break;

		case -1:
			log_error(logger_memoria, "CPU se desconecto. Terminando servidor.\n");
			//return EXIT_FAILURE;
			break;

		default:
			log_warning(logger_memoria,"Operacion desconocida: %d",cod_op);
			break;
		}
    }
}




/*---------------------------- KERNEL-------------------------*/


void memoria_atender_kernel(void* socket){
    int fd_kernel=*((int*)socket);
	// while (1) {
        //Se queda esperando a que KErnel le envie algo y extrae el cod de operacion
		int cod_op = recibir_operacion(fd_kernel);
		op_code response;
		t_list* valores;
		//pthread_t
		//sleep(5);
		switch (cod_op) {
		case HANDSHAKE:
			//pthread_create(&hilo_kernel, NULL, (void*) memoria_atender_kernel, NULL));
			log_info(logger_memoria, "Handshake realizado con cliente(%d)",fd_kernel);
            response = HANDSHAKE_OK;
            if (send(fd_kernel, &response, sizeof(uint32_t), MSG_WAITALL) != sizeof(uint32_t)) {
                log_error(logger_memoria, "Error al enviar respuesta de handshake a cliente");
                   
                break;
            }
            break;
		
		case INICIAR_PROCESO:
			log_info(logger_memoria, "Recibí INICIAR_PROCESO \n");
			valores = recibir_paquete(fd_kernel);

			t_m_crear_proceso* iniciar_proceso = deserializar_iniciar_proceso(valores);
			printf("SALIO DE DESEREALIZACION:%d\n",iniciar_proceso->pid);
			if(existe_proceso_en_memoria(iniciar_proceso->pid)){
				printf("ENTRA EXISTE\n"); 
				//Enviar rta ERROR:Ya existe
				enviar_respuesta_iniciar_proceso(iniciar_proceso, fd_kernel,INICIAR_PROCESO_RTA_ERROR_YA_EXISTE);
			}
			else{
				printf("ENTRA EXISTE FALSE\n"); 
				//INICIO MUTEX
				int rta_crear_proceso = crear_proceso(iniciar_proceso->pid,iniciar_proceso->tamanio_proceso);
				printf("SALE CREAR PROCESO\n"); 
				//FIN MUTEX
				if(rta_crear_proceso == INICIAR_PROCESO_RTA_OK){
				//inicializar_proceso(iniciar_proceso->pid, iniciar_proceso->tamanio_proceso);
				//enviar rta OK
				//usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
				enviar_respuesta_iniciar_proceso(iniciar_proceso, fd_kernel,INICIAR_PROCESO_RTA_OK);
				log_info(logger_memoria, "## Proceso Creado- PID: %d Tamaño: %d\n",iniciar_proceso->pid,iniciar_proceso->tamanio_proceso);
				log_info(logger_memoria, "enviada respuesta OK hay espacio \n");
				}
				else{
					//enviar rta con error:no hay espacio en memoria
					//usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
					enviar_respuesta_iniciar_proceso(iniciar_proceso, fd_kernel,rta_crear_proceso);
						log_info(logger_memoria, "enviada respuesta no hay espacio \n");
				}
			}
			
			
			//usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
			//enviar_respuesta_iniciar_proceso(iniciar_proceso, fd_kernel);
			log_info(logger_memoria, "enviada respuesta de INICIAR_PROCESO_RTA \n");
			break;

		case FINALIZAR_PROCESO:
			log_info(logger_memoria, "Recibí FINALIZAR_PROCESO \n");
			valores = recibir_paquete(fd_kernel);
			uint32_t pid_proceso_a_finalizar = deserializar_finalizar_proceso(valores);
            //finalizar_proceso(pid_proceso_a_finalizar);
			if(!existe_proceso_en_memoria(pid_proceso_a_finalizar)){
				//Enviar rta ERROR:No existe
				enviar_respuesta_finalizar_proceso(pid_proceso_a_finalizar, fd_kernel,FINALIZAR_PROCESO_RTA_ERROR_NO_EXISTE);
			}
			else{
				//INICIO MUTEX
				finalizar_proceso(pid_proceso_a_finalizar);
				//FIN MUTEX
				//Elimino de lista miniPBCs
				//usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
				enviar_respuesta_finalizar_proceso(pid_proceso_a_finalizar, fd_kernel,FINALIZAR_HILO_RTA_OK);
				uint32_t tamanio_proceso = buscar_tamanio_proceso_por_pid(pid_proceso_a_finalizar);
				log_info(logger_memoria, "## Proceso Destruido- PID: %d Tamaño: %d\n",pid_proceso_a_finalizar,tamanio_proceso);
				
			}
			
			//usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
			//enviar_respuesta_finalizar_proceso(pid_proceso_a_finalizar, fd_kernel);
			log_info(logger_memoria, "enviada respuesta de FINALIZAR_PROCESO_RTA \n");
			break;

		case INICIAR_HILO:
			log_info(logger_memoria, "Recibí INICIAR_HILO \n");
			valores = recibir_paquete(fd_kernel);
			t_m_crear_hilo* iniciar_hilo = deserializar_iniciar_hilo(valores);
			// if(existe_hilo_en_memoria(iniciar_hilo->pid,iniciar_hilo->tid)){
			// 	//Enviar rta ERROR:Ya existe
			// 	enviar_respuesta_iniciar_hilo(iniciar_hilo, fd_kernel,INICIAR_HILO_RTA_ERROR_YA_EXISTE);
			// }
			// else{
				//leer_instrucciones(iniciar_hilo->archivo_pseudocodigo, iniciar_hilo->pid, iniciar_hilo->tid);
				//INICIO MUTEX
				inicializar_hilo(iniciar_hilo->pid, iniciar_hilo->tid, iniciar_hilo->archivo_pseudocodigo);
				//FIN MUTEX
				//usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
				enviar_respuesta_iniciar_hilo(iniciar_hilo, fd_kernel,INICIAR_HILO_RTA_OK);
				log_info(logger_memoria, "## Hilo Creado- (PID:TID)- (%d:%d)\n",iniciar_hilo->pid,iniciar_hilo->tid);
			// }
			
			log_info(logger_memoria, "enviada respuesta de INICIAR_HILO_RTA \n");
			break;

		case FINALIZAR_HILO:
			log_info(logger_memoria, "Recibí FINALIZAR_HILO \n");
			valores = recibir_paquete(fd_kernel);
			uint32_t pid_hilo = *(uint32_t*)list_get(valores, 0);
			uint32_t tid_hilo = *(uint32_t*)list_get(valores, 1);

			if(!existe_hilo_en_memoria(pid_hilo,tid_hilo)){
				//Enviar rta ERROR:No existe
				enviar_respuesta_finalizar_hilo(pid_hilo,tid_hilo, fd_kernel,FINALIZAR_HILO_RTA_ERROR_NO_EXISTE);
			}
			else{
				//INICIO MUTEX
				eliminar_hilo_de_lista(lista_miniPCBs,pid_hilo,tid_hilo);
				//FIN MUTEX
				//usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
				enviar_respuesta_finalizar_hilo(pid_hilo,tid_hilo, fd_kernel,FINALIZAR_HILO_RTA_OK);
				log_info(logger_memoria, "## Hilo Destruido- (PID:TID)- (%d:%d)\n",pid_hilo,tid_hilo);
			}

			
			log_info(logger_memoria, "enviada respuesta de FINALIZAR_HILO_RTA \n");
			break;

		case PEDIDO_MEMORY_DUMP:
			log_info(logger_memoria, "Recibí PEDIDO_MEMORY_DUMP \n");
			valores = recibir_paquete(fd_kernel);
			uint32_t pid = *(uint32_t*)list_get(valores, 0);
			uint32_t tid = *(uint32_t*)list_get(valores, 1);

			log_info(logger_memoria, "## Memory Dump solicitado - (PID:TID) - (%d:%d)\n",pid,tid); 

			char* nombre_archivo = generar_nombre_archivo(pid, tid);
			uint32_t tamanio_nombre_archivo = (strlen(nombre_archivo)+1) * sizeof(char);

			t_miniPCB* proceso_a_leer = busco_proceso_por_PID(pid);
			//obtener base y tamanio del proceso asociado al pid
			uint32_t base_proceso = proceso_a_leer->base;
			uint32_t tamanio_proceso = proceso_a_leer->limite;

			
			
			//hacer un for con la variable anterior y por cada pasada hacer un read e ir concatenandolo en una variable
			char* contenido_leido = malloc(tamanio_proceso+1);	
			 for (int i = 0; i < tamanio_proceso; i++) {
        		memcpy(contenido_leido+i, memoria_usuario+base_proceso+i, 1); 
				//printf("%s ", contenido_leido[i]);
    		};
			contenido_leido[tamanio_proceso]='\0'; 
				                        
            //log_error(logger_memoria, "Error al leer la memoria en la dirección: %d", direccion_lectura);
            //free(contenido_leido);
           
      
    	 	log_info(logger_memoria, "Contenido para dump: %d", contenido_leido);
		 	log_info(logger_memoria, "Contenido para dump: %s", contenido_leido);
			log_info(logger_memoria, "Contenido para dump: %x", contenido_leido);	
			// Calcular el tamaño real de lo leído (siempre será igual a tamanio_proceso)
    		uint32_t tamanio_contenido = tamanio_proceso;
			
			
			//preparo la informacion para pasarsela al nuevo hilo
			t_peticion_dump_fs* peticion_fs = malloc(sizeof(t_peticion_dump_fs));
			peticion_fs->tamanio_nombre_archivo = tamanio_nombre_archivo;
			peticion_fs->nombre_archivo = nombre_archivo;
			peticion_fs->tamanio_contenido = tamanio_contenido;
			peticion_fs->contenido= malloc(tamanio_proceso+1);
			strcpy(peticion_fs->contenido,contenido_leido); ////////
			log_info(logger_memoria, "Contenido para dump: %s", peticion_fs->contenido);
			peticion_fs->fd_kernel = fd_kernel;

			//creamos hilo para creacion de nueva conexion con fs
			pthread_t hilo_dump_fs; 
    		pthread_create(&hilo_dump_fs, NULL, (void *)atender_dump_memory_fs, peticion_fs);
    		pthread_detach(hilo_dump_fs); //va el & o no? si lo pongo me tira un warning
			
			break;

		case -1:
		
			log_error(logger_memoria, "Kernel se desconecto. Terminando servidor.\n");
			//return EXIT_FAILURE;
			break;

		default:
			log_warning(logger_memoria,"Operacion desconocida: %d",cod_op);
			break;
		}

		free(socket);
    //  }
}

void atender_dump_memory_fs(t_peticion_dump_fs* peticion_fs){
	//crear conexion con fs y enviar peticion
	int socket_filesystem = crear_conexion(logger_memoria, "File System", cfg_memoria->IP_FILESYSTEM, cfg_memoria->PUERTO_FILESYSTEM);
    //enviar_solicitud_espacio_a_memoria(un_pcb,socket_filesystem);
	enviar_creacion_memory_dump(peticion_fs->tamanio_nombre_archivo,peticion_fs->nombre_archivo,peticion_fs->tamanio_contenido, peticion_fs->contenido,socket_filesystem);
     
    //int respuesta=recibir_resp_de_memoria_a_solicitud(socket_memoria);
	//int respuesta=recibir_resp_de_fs_memory_dump(socket_filesystem);
	int respuesta = recibir_operacion(socket_filesystem);
        
	//close(socket_memoria);
    close(socket_filesystem);

	if(respuesta==PEDIDO_MEMORY_DUMP_RTA_OK){
        log_info(logger_memoria,"recibi OK rta memory dump de fs\n");
		//Enviar a kernel OK
		enviar_confirmacion_memory_dump_a_kernel(PEDIDO_MEMORY_DUMP_RTA_OK,peticion_fs->fd_kernel);
    }else{
        log_info(logger_memoria,"hubo ERROR en rta memory dump de fs\n");
	   //Enviar a kernel ERROR
	   enviar_confirmacion_memory_dump_a_kernel(PEDIDO_MEMORY_DUMP_RTA_ERROR,peticion_fs->fd_kernel);
    }
}



