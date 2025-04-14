# TP1-SO

## Ejecucion en Docker

### Descargar la imagen
`docker pull agodio/itba-so-multi-platform:3.0`

### Ejecución del contenedor
`docker run -v "${PWD}:/root" --privileged -ti agodio/itba-so-multi-platform:3.0`

### Copiar el contenido en el container
`docker cp TP1-SO <nombre_o_id_del_contenedor>:/root`

### Ir al directorio de guardado:
`root@id: cd root/TP1-SO`

### Ejecutar:
`root@id: ./nombre_ejecutable`