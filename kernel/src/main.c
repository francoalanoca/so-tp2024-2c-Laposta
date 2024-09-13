#include "../include/main.h"
int main(int argc, char* argv[]) {
    char* path_config = argv[1];
    iniciar_modulo(path_config);
    // generar_conexiones_a_cpu();
        // int conexion=conectar_a_memoria();
        // int soli_hand=HANDSHAKE;
        // send(conexion,&soli_hand, sizeof(uint32_t), MSG_WAITALL);
        // log_info(logger_kernel,"ENIVADO DESDE MAIN");
        // sleep(50000);
     process_create("archivo1.txt",12,5);
     sleep(50000);
     process_create("archivo2.txt",10,2);
    //  mostrar_pcb(list_get(lista_new,0),logger_kernel);
    //  mostrar_pcb(list_get(lista_new,1),logger_kernel);    
    
    return 0;

}
