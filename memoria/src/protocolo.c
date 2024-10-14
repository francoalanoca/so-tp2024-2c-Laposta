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
			log_info(logger_memoria, "Recibí SOLICITUD_CONTEXTO \n");
			valores = recibir_paquete(socket_cpu);
			uint32_t pid = *(uint32_t*)list_get(valores, 0);
			uint32_t tid = *(uint32_t*)list_get(valores, 1);
			t_m_contexto* contexto_encontrado = buscar_contexto_en_lista(pid,tid);
			usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
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
			log_info(logger_memoria, "Recibí SOLICITUD_INSTRUCCION \n");
			valores = recibir_paquete(socket_cpu);
			t_proceso_memoria* solicitud_instruccion = deserializar_solicitud_instruccion(valores);         
            char* instruccion = buscar_instruccion(solicitud_instruccion->pid, solicitud_instruccion->tid, solicitud_instruccion->program_counter);
			usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
			enviar_respuesta_instruccion(instruccion, socket_cpu);    
			log_info(logger_memoria, "enviada respuesta de SOLICITUD_INSTRUCCION_RTA \n");
			break;

		case READ_MEMORIA:
			log_info(logger_memoria, "Recibí READ_MEMORIA \n");
			valores = recibir_paquete(socket_cpu);
			t_escribir_leer* peticion_leer = deserializar_read_memoria(valores);     
            char* respuesta_leer = malloc(4);
			memset(respuesta_leer, 0, sizeof(respuesta_leer));
			//INICIO MUTEX
			if(read_mem(peticion_leer->direccion_fisica,respuesta_leer)){
				usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
				enviar_respuesta_read_memoria(peticion_leer->pid,respuesta_leer, socket_cpu,READ_MEMORIA_RTA_OK);
				log_info(logger_memoria, "“## <Escritura> - (PID:TID) - (%d:%d) - Dir. Física: %d - Tamaño: %d” \n",peticion_leer->pid,peticion_leer->tid,peticion_leer->direccion_fisica,peticion_leer->tamanio); 
				
			}
			else{
				usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
				enviar_respuesta_read_memoria(peticion_leer->pid,respuesta_leer, socket_cpu,READ_MEMORIA_RTA_ERROR);
			}
			//FIN MUTEX
			//usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
			//enviar_respuesta_read_memoria(respuesta_leer, socket_cpu);
			log_info(logger_memoria, "enviada respuesta de READ_MEMORIA_RTA \n");
			break;

		case WRITE_MEMORIA:
			log_info(logger_memoria, "Recibí WRITE_MEMORIA \n");
			valores = recibir_paquete(socket_cpu);
			t_escribir_leer* peticion_escribir = deserializar_write_memoria(valores);  
			//INICIO MUTEX 
			if(write_mem(peticion_escribir->direccion_fisica, peticion_escribir->valor)){
				usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
				enviar_respuesta_write_memoria(peticion_escribir->pid, socket_cpu,WRITE_MEMORIA_RTA_OK);
				log_info(logger_memoria, "“## <Escritura> - (PID:TID) - (%d:%d) - Dir. Física: %d - Tamaño: %d” \n",peticion_escribir->pid,peticion_escribir->tid,peticion_escribir->direccion_fisica,peticion_escribir->tamanio);
			}
			else{
				usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
				enviar_respuesta_write_memoria(peticion_escribir->pid, socket_cpu,WRITE_MEMORIA_RTA_ERROR);
			}
			//FIN MUTEX
            //char* respuesta_escribir = escribir_memoria(peticion_escribir->pid, peticion_escribir->direccion_fisica, peticion_escribir->valor, peticion_escribir->tamanio);
			//usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
			//enviar_respuesta_write_memoria(respuesta_escribir, socket_cpu);
			log_info(logger_memoria, "enviada respuesta de WRITE_MEMORIA_RTA \n");
			break;
		
		case DEVOLUCION_CONTEXTO:
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
			usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
			log_info(logger_memoria, "enviada respuesta de DEVOLUCION_CONTEXTO_RTA \n");
			break;

		case -1:
			log_error(logger_memoria, "CPU se desconecto. Terminando servidor.\n");
			return EXIT_FAILURE;
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
		sleep(5);
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
			if(existe_proceso_en_memoria(iniciar_proceso->pid)){
				//Enviar rta ERROR:Ya existe
				enviar_respuesta_iniciar_proceso(iniciar_proceso, fd_kernel,INICIAR_PROCESO_RTA_ERROR_YA_EXISTE);
			}
			else{
				//INICIO MUTEX
				int rta_crear_proceso = crear_proceso(iniciar_proceso->pid,iniciar_proceso->tamanio_proceso);
				//FIN MUTEX
				if(rta_crear_proceso == INICIAR_PROCESO_RTA_OK){
				//inicializar_proceso(iniciar_proceso->pid, iniciar_proceso->tamanio_proceso);
				//enviar rta OK
				usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
				enviar_respuesta_iniciar_proceso(iniciar_proceso, fd_kernel,INICIAR_PROCESO_RTA_OK);
				log_info(logger_memoria, "## Proceso Creado- PID: %d Tamaño: %d\n",iniciar_proceso->pid,iniciar_proceso->tamanio_proceso);
				log_info(logger_memoria, "enviada respuesta OK hay espacio \n");
				}
				else{
					//enviar rta con error:no hay espacio en memoria
					usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
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
				usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
				enviar_respuesta_finalizar_proceso(pid_proceso_a_finalizar, fd_kernel,FINALIZAR_PROCESO_RTA_OK);
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
				usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
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
				usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
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
			char* nombre_archivo = generar_nombre_archivo(pid, tid);
			uint32_t tamanio_nombre_archivo = (strlen(nombre_archivo)+1) * sizeof(char);

			t_miniPCB* proceso_a_leer = busco_proceso_por_PID(pid);
			//obtener base y tamanio del proceso asociado al pid
			uint32_t base_proceso = proceso_a_leer->base;
			uint32_t tamanio_proceso = proceso_a_leer->limite;

			//crear variable que diga cuantas leídas debo hacer en base al tamanio del proceso
			uint32_t leidas_a_hacer = tamanio_proceso / 4; //siempre se lee de a 4 bytes
			
			//hacer un for con la variable anterior y por cada pasada hacer un read e ir concatenandolo en una variable
			char* contenido = malloc(tamanio_proceso);
			char* leido_actual = malloc(4);
			for(int i=0;i<leidas_a_hacer;i++){
        	if(read_mem(base_proceso,leido_actual)){
				//concateno lo leido con lo que ya habia leido antes
				strcat(contenido, leido_actual);
			}
			else{
				printf("Ocurrio un error al leer el contenido del proceso\n");
			}
        	
    		}

			//ver el tamanio real de lo leído
			uint32_t tamanio_contenido = strlen(contenido) + 1; //no hace falta multiplicar por sizeof(char) ya que este siempre vale 1 byte

			usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
			
			//crear conexion con fs y enviar peticion
			int socket_filesystem = crear_conexion(logger_memoria, "File System", cfg_memoria->IP_FILESYSTEM, cfg_memoria->PUERTO_FILESYSTEM);
            //enviar_solicitud_espacio_a_memoria(un_pcb,socket_filesystem);
			enviar_creacion_memory_dump(tamanio_nombre_archivo,nombre_archivo,tamanio_contenido, contenido,socket_filesystem);
          
            //int respuesta=recibir_resp_de_memoria_a_solicitud(socket_memoria);
			//int respuesta=recibir_resp_de_fs_memory_dump(socket_filesystem);
			int respuesta = recibir_operacion(socket_filesystem);
        
			//close(socket_memoria);
            close(socket_filesystem);

			if(respuesta==PEDIDO_MEMORY_DUMP_RTA_OK){
                log_info(logger_memoria,"recibi OK rta memory dump de fs\n");
				//Enviar a kernel OK
				enviar_confirmacion_memory_dump_a_kernel(PEDIDO_MEMORY_DUMP_RTA_OK,fd_kernel);
            }else{
               log_info(logger_memoria,"hubo ERROR en rta memory dump de fs\n");
			   //Enviar a kernel ERROR
			   enviar_confirmacion_memory_dump_a_kernel(PEDIDO_MEMORY_DUMP_RTA_ERROR,fd_kernel);
            }

			//enviar_creacion_memory_dump(tamanio_nombre_archivo,nombre_archivo,tamanio_contenido, contenido,socket_filesystem); //TODO: ver como se consigue socket_filesystem
			//log_info(logger_memoria, "enviada respuesta de PEDIDO_MEMORY_DUMP_RTA \n");
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




