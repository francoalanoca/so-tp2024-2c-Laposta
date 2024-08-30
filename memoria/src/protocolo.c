#include "../include/protocolo.h"




/*---------------------------- CPU-------------------------*/

void memoria_atender_cpu(){
    


	while (1) {

        //Se queda esperando a que Cpu le envie algo y extrae el cod de operacion
		int cod_op = recibir_operacion(socket_cpu);
		op_code response;
		t_list* valores =  malloc(sizeof(t_list));


		switch (cod_op) {
		case HANDSHAKE:
			log_info(logger_memoria, "Handshake realizado con cliente(%d)",socket_cpu);
            response = HANDSHAKE_OK;
            if (send(socket_cpu, &response, sizeof(uint32_t), MSG_WAITALL) != sizeof(uint32_t)) {
                log_error(logger_memoria, "Error al enviar respuesta de handshake a cliente");
                   
                break;
            }
            break;
		
		case SOLICITUD_CONTEXTO:
			log_info(logger_memoria, "Recibí SOLICITUD_CONTEXTO \n");
			valores = recibir_paquete(socket_cpu);
			usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
			log_info(logger_memoria, "enviada respuesta de SOLICITUD_CONTEXTO_RTA \n");
			break;

		case SOLICITUD_INSTRUCCION:
			log_info(logger_memoria, "Recibí SOLICITUD_INSTRUCCION \n");
			valores = recibir_paquete(socket_cpu);
			//t_proceso_memoria* solicitud_instruccion = deserializar_solicitud_instruccion(valores);         
            //char* instruccion = buscar_instruccion(solicitud_instruccion->pid, solicitud_instruccion->program_counter);
			usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
			//enviar_respuesta_instruccion(instruccion, socket_cpu);    
			log_info(logger_memoria, "enviada respuesta de SOLICITUD_INSTRUCCION_RTA \n");
			break;

		case READ_MEMORIA:
			log_info(logger_memoria, "Recibí READ_MEMORIA \n");
			valores = recibir_paquete(socket_cpu);
			//t_escribir_leer* peticion_leer = deserializar_read_memoria(valores);     
            //char* respuesta_leer = leer_memoria(peticion_leer->pid, peticion_leer->direccion_fisica, peticion_leer->tamanio);
			usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
			//enviar_respuesta_read_memoria(respuesta_leer, socket_cpu);
			log_info(logger_memoria, "enviada respuesta de READ_MEMORIA_RTA \n");
			break;

		case WRITE_MEMORIA:
			log_info(logger_memoria, "Recibí WRITE_MEMORIA \n");
			valores = recibir_paquete(socket_cpu);
			//t_escribir_leer* peticion_escribir = deserializar_write_memoria(valores);   
            //char* respuesta_escribir = escribir_memoria(peticion_escribir->pid, peticion_escribir->direccion_fisica, peticion_escribir->valor, peticion_escribir->tamanio);
			usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
			//enviar_respuesta_write_memoria(respuesta_escribir, socket_cpu);
			log_info(logger_memoria, "enviada respuesta de WRITE_MEMORIA_RTA \n");
			break;
		
		case DEVOLUCION_CONTEXTO:
			log_info(logger_memoria, "Recibí DEVOLUCION_CONTEXTO \n");
			valores = recibir_paquete(socket_cpu);
			usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
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

void memoria_atender_kernel(){
    


	while (1) {
        //Se queda esperando a que KErnel le envie algo y extrae el cod de operacion
		int cod_op = recibir_operacion(socket_kernel);
		op_code response;
		t_list* valores =  malloc(sizeof(t_list));


		switch (cod_op) {
		case HANDSHAKE:
			log_info(logger_memoria, "Handshake realizado con cliente(%d)",socket_kernel);
            response = HANDSHAKE_OK;
            if (send(socket_kernel, &response, sizeof(uint32_t), MSG_WAITALL) != sizeof(uint32_t)) {
                log_error(logger_memoria, "Error al enviar respuesta de handshake a cliente");
                   
                break;
            }
            break;
		
		case INICIAR_PROCESO:
			log_info(logger_memoria, "Recibí INICIAR_PROCESO \n");
			valores = recibir_paquete(socket_kernel);
			//t_m_crear_proceso iniciar_proceso = deserializar_iniciar_proceso(valores);
			//leer_instrucciones(iniciar_proceso->archivo_pseudocodigo, iniciar_proceso->pid);
			//crear_proceso(iniciar_proceso->pid);
			usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
			//enviar_respuesta_iniciar_proceso(iniciar_proceso, socket_kernel);
			log_info(logger_memoria, "enviada respuesta de INICIAR_PROCESO_RTA \n");
			break;

		case FINALIZAR_PROCESO:
			log_info(logger_memoria, "Recibí FINALIZAR_PROCESO \n");
			valores = recibir_paquete(socket_kernel);
			//uint32_t pid_proceso_a_finalizar = deserializar_finalizar_proceso(valores);
            //finalizar_proceso(pid_proceso_a_finalizar);
			usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
			//enviar_respuesta_finalizar_proceso(pid_proceso_a_finalizar, socket_kernel);
			log_info(logger_memoria, "enviada respuesta de FINALIZAR_PROCESO_RTA \n");
			break;

		case INICIAR_HILO:
			log_info(logger_memoria, "Recibí INICIAR_HILO \n");
			valores = recibir_paquete(socket_kernel);
			usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
			log_info(logger_memoria, "enviada respuesta de INICIAR_HILO_RTA \n");
			break;

		case FINALIZAR_HILO:
			log_info(logger_memoria, "Recibí FINALIZAR_HILO \n");
			valores = recibir_paquete(socket_kernel);
			usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
			log_info(logger_memoria, "enviada respuesta de FINALIZAR_HILO_RTA \n");
			break;

		case PEDIDO_MEMORY_DUMP:
			log_info(logger_memoria, "Recibí PEDIDO_MEMORY_DUMP \n");
			valores = recibir_paquete(socket_kernel);
			usleep(cfg_memoria->RETARDO_RESPUESTA * 1000);
			log_info(logger_memoria, "enviada respuesta de PEDIDO_MEMORY_DUMP_RTA \n");
			break;

		case -1:
			log_error(logger_memoria, "Kernel se desconecto. Terminando servidor.\n");
			//return EXIT_FAILURE;
			break;

		default:
			log_warning(logger_memoria,"Operacion desconocida: %d",cod_op);
			break;
		}
    }
}




