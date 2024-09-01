#include "../include/main.h"
int main(int argc, char* argv[]) {
    iniciar_modulo("/home/utnso/GitHub/tp-2024-2c-Laposta/kernel/src/config_kernel.config");
     generar_conexiones_a_cpu();
     conectar_a_memoria();

    return 0;
}
