#include "../include/main.h"
int main(int argc, char* argv[]) {
    char* path_config = argv[1];
    iniciar_modulo(path_config);
    // generar_conexiones_a_cpu();
     //conectar_a_memoria();
     process_create("proceso_codigo.txt",12345,5);
     process_create("proc_codigo2.txt",10,2);
     mostrar_pcb(list_get(lista_new,0),logger_kernel);
     mostrar_pcb(list_get(lista_new,1),logger_kernel);    
    return 0;
}
