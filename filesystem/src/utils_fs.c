#include"../include/utils_fs.h"


t_dumped* dumped_deserializar(t_list* lista) {
    t_dumped* dumped = malloc(sizeof(t_dumped));


}

void liberar_t_dumped(t_dumped* dumped) {
    if (dumped == NULL) {
        return; 
    }
   
    if (dumped->nombre_archivo != NULL) {
        free(dumped->nombre_archivo);
    }
    if (dumped->contenido != NULL) {
        free(dumped->contenido);
    }
    free(dumped);
}