# TP1-SO

## Ejecucion en Docker

- Iniciar el container de la materia:
`docker cp mi_programa <nombre_o_id_del_contenedor>:/root/mi_programa`
`docker start <CONTAINER ID>`

- Ejecutar container:
`docker exec -it <CONTIANER ID> bash`

- Moverse al directorio de guardado:
`root@id: cd root/mi_programa`

- Ejecutar:
`root@id: ./nombre_ejecutable`