#include "../include/main.h"

char *path_config;

int main(char argc, char *argv[]) {

    path_config = argv[1];

    printf("iniciando...\n");

    if (!init(path_config) || !cargar_configuracion(path_config)) {
        cerrar_programa();
        printf("No se pudo inicializar entrada salida");
        return EXIT_FAILURE;
    }


  



    
    



    cerrar_programa();
    return 0;
}