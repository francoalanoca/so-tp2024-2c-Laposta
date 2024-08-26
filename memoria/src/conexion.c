#include "../include/conexion.h"



/*
//Funcion que inicia los servidores y sockets
void iniciar_servidores(){

    iniciar_conexiones();
    escuchar_modulos();
}



//Funcion que inicia los socket de memoria y los demas modulos
void iniciar_conexiones(){
    //Iniciar server de Memoria
    socket_memoria = iniciar_servidor(logger_memoria, "Server Memoria", "127.0.0.0", cfg_memoria->PUERTO_ESCUCHA);
    log_info(logger_memoria,"Inicio de server Kernel exitosamente");

    
    //Esperar al cliente Cpu
    log_info(logger_memoria, "Conexion con Cpu exitosa");
    socket_cpu = esperar_cliente(logger_memoria, "Cpu", socket_memoria);

    //Esperar al cliente Kernel
    log_info(logger_memoria, "Conexion con Kernel exitosa");
    socket_kernel = esperar_cliente(logger_memoria, "Kernel", socket_memoria);

    //Esperar al cliente EntradaSalida
    log_info(logger_memoria, "Conexion con Entrada Salida exitosa");
    socket_filesystem = esperar_cliente(logger_memoria, "File System", socket_memoria);
}



void escuchar_modulos(){

	//Atender a Kernel
    pthread_t hilo_kernel;
    //se crea un nuevo hilo que atiende al cliente
    pthread_create(&hilo_kernel, NULL, (void*) memoria_atender_kernel, NULL);
    //se desacopla del hilo principal para no interferir
    pthread_detach(hilo_kernel);

    //Atender a Cpu
    pthread_t hilo_cpu;
    //se crea un nuevo hilo que atiende al cliente
    pthread_create(&hilo_cpu, NULL, (void*) memoria_atender_cpu, NULL);
    //se desacopla del hilo principal para no interferir
    pthread_detach(hilo_cpu);

    //Atender a EntradaSalida
    pthread_t hilo_entradasalida;
    //se crea un nuevo hilo que atiende al cliente
    pthread_create(&hilo_entradasalida, NULL, (void*)memoria_atender_filesystem, NULL);
    //caundo el hilo principal llega a join este se detiene hasta que se termine el hilo actual en lugar de deacoplarse
    pthread_join(hilo_entradasalida, NULL);
}

*/