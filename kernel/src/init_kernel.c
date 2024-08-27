#include "../include/init_kernel.h"
t_config_kernel* config_kernel;
t_log* logger_kernel;

void iniciar_modulo( char *ruta_config){
    logger_kernel=log_create("logs_kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
    cargar_config_kernel(ruta_config);

}
void cargar_config_kernel(char* ruta_config){
    t_config* config=iniciar_config(ruta_config,logger_kernel);
    config_kernel=malloc(sizeof(t_config_kernel));
    config_kernel->algoritmo_planif=config_get_string_value(config,"ALGORITMO_PLANIFICACION");
    config_kernel->ip_cpu=config_get_string_value(config,"IP_CPU");
    config_kernel->ip_memoria=config_get_string_value(config,"IP_MEMORIA");
    config_kernel->puerto_memoria=config_get_string_value(config,"PUERTO_MEMORIA");
    config_kernel->log_level=config_get_string_value(config,"LOG_LEVEL");
    config_kernel->puerto_dispatch=config_get_string_value(config,"PUERTO_CPU_DISPATCH");
    config_kernel->puerto_interrupt=config_get_string_value(config,"PUERTO_CPU_INTERRUPT");
    config_kernel->quantum=config_get_int_value(config,"QUANTUM");
    
    log_info(logger_kernel,"configuracion cargada");

}

void conectar_a_memoria(){
    pthread_t conexion_memoria_hilo;
    int conexion_memoria = crear_conexion(
        logger_kernel
        ,"MEMORIA"
        ,config_kernel->ip_memoria
        ,config_kernel->puerto_memoria);
	pthread_create(&conexion_memoria_hilo,NULL,(void*) procesar_conexion,(void *)&conexion_memoria);
	pthread_detach(conexion_memoria_hilo);
    log_info(logger_kernel,"conexion a MEMORIA\n");
}
void generar_conexiones_a_cpu() {

	pthread_t conexion_cpu_dispatch_hilo;
	pthread_t conexion_cpu_interrupt_hilo;
    //conecta a por dispatch a cpu
	config_kernel->conexion_cpu_dispatch = crear_conexion(
        logger_kernel
        ,"CPU"
        ,config_kernel->ip_cpu
        ,config_kernel->puerto_dispatch
    );
	log_info(logger_kernel,"conexion generado correctamente\n");
	pthread_create(&conexion_cpu_dispatch_hilo,NULL,(void*) procesar_conexion,(void *)&config_kernel->conexion_cpu_dispatch);
	pthread_detach(conexion_cpu_dispatch_hilo);

    //conecta a por interrupt a cpu
	config_kernel->conexion_cpu_interrupt= crear_conexion(
        logger_kernel
        ,"CPU"
        ,config_kernel->ip_cpu
        ,config_kernel->puerto_interrupt);
	pthread_create(&conexion_cpu_interrupt_hilo, NULL, (void*) procesar_conexion, (void *)&config_kernel->conexion_cpu_interrupt);
	pthread_detach(conexion_cpu_interrupt_hilo);
	log_info(logger_kernel,"conexion generado correctamente\n");

}
void procesar_conexion(void* socket){
    //TODO: operaciones a ejecutar
}

