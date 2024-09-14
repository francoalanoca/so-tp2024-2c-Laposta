#include "../include/main.h"
int main(int argc, char* argv[]) {
    char* path_config = argv[1];
    iniciar_modulo(path_config);
    generar_conexiones_a_cpu();
    int tamanio=atoi(argv[3]);
    process_create(argv[2],tamanio,HILO_MAIN);//PROCESO KERNEL INICIAL
    sleep(50000);
    process_create("archivo2.txt",10,2);
    //  mostrar_pcb(list_get(lista_new,0),logger_kernel);
    //  mostrar_pcb(list_get(lista_new,1),logger_kernel);    
    
    return 0;

}
