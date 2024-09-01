#include "../include/main.h"
int main(int argc, char* argv[]) {
    char* path_config = argv[1];
    iniciar_modulo(path_config);
     generar_conexiones_a_cpu();
     conectar_a_memoria();

    return 0;
}
