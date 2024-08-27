#include "../include/main.h"

int main(int argc, char* argv[]) {

    char* path_config = argv[1];              //para correr por consola
    //char* path_config = "./memoria.config";   //para correr por vsc(terminal)
    
    //-------------------Configuraciones---------------------------
    if (!init(path_config) || !cargar_configuracion(path_config)) {

        cerrar_programa();
        printf("No se pudo inicializar configuracion\n");
        return EXIT_FAILURE;
    }
    //inicializar_configuraion(path_config);
    printf("Se iniciaron correctamente las configuraciones\n");

    //-------------------Variables---------------------------
    if (!inicializar_memoria()) {
        cerrar_programa();
        printf("No se pudo inicializar Memoria\n");
        return EXIT_FAILURE;
    }
    
    log_info(logger_memoria, "Se inicio correctamente la Memoria");

    //-------------------Servidores------------------------
    //iniciar_servidores();
    //log_info(logger_memoria, "Se inicio correctamente los servidores");

    //saludar("memoria");
    //void cerrar_programa();
    return 0;
}
