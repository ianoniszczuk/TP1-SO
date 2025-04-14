# 🐍 Chomp Champs - Sistemas Operativos TP1

Un juego de tipo Snake multijugador implementado con mecanismos IPC.

## 📋 Contenido

- [Descripción](#descripción)
- [Instalación](#instalación)
- [Ejecución](#ejecución)
- [Análisis de Código](#análisis-de-código)
- [Arquitectura](#arquitectura)

## 📝 Descripción

Este proyecto implementa una versión multiplayer reversionada del clásico juego Snake usando conceptos de sistemas operativos como:
- Memoria compartida
- Procesos y comunicación entre procesos
- Semáforos y sincronización

## 🚀 Instalación

### Compilación local

```bash
# Clonar el repositorio
git clone https://github.com/username/TP1-SO.git
cd TP1-SO

# Compilar el proyecto
make clean && make

# Ejecutar el proyecto
./compile_and_run.sh
```

### Ejecución en Docker

#### 1. Descargar la imagen

```bash
docker pull agodio/itba-so-multi-platform:3.0
```

#### 2. Ejecución del contenedor

```bash
docker run -v "${PWD}:/root" --privileged -ti agodio/itba-so-multi-platform:3.0
```

#### 3. Copiar el contenido en el container (alternativa)

```bash
docker cp TP1-SO <nombre_o_id_del_contenedor>:/root
```

#### 4. Navegar al directorio

```bash
cd /root/TP1-SO
```

## 🎮 Ejecución

### Ejecutar el juego

```bash
./executables/master -d 400 -t 20 -p ./executables/player ./executables/player ./executables/player ./executables/player -v ./executables/view
```

| Parámetro | Descripción                           |
|-----------|---------------------------------------|
| -d        | Delay en milisegundos                 |
| -t        | Timeout en segundos                   |
| -p        | Flag que indica los jugadores         |
| -v        | Path del visualizador (opcional)      |
| -w        | Ancho del tablero (por defecto: 10)   |
| -h        | Alto del tablero (por defecto: 10)    |
| -s        | Semilla para los números aleatorios   |

## 🔍 Análisis de Código

Para realizar un análisis estático del código, se puede utilizar PVS-Studio:

### Preparar los archivos

```bash
# Agregar comentarios requeridos para PVS-Studio
find . -name "*.c" | while read line; do 
    sed -i '1s/^\(.*\)$/\/\/ This is a personal academic project. Dear PVS-Studio, please check it.\n\1/' "$line"
done

find . -name "*.c" | while read line; do 
    sed -i '2s/^\(.*\)$/\/\/ PVS-Studio Static Code Analyzer for C, C++ and C#: http:\/\/www.viva64.com\n\1/' "$line"
done
```

### Ejecutar el análisis

```bash
pvs-studio-analyzer trace -- make
pvs-studio-analyzer analyze
plog-converter -a '64:1,2,3;GA:1,2,3;OP:1,2,3' -t tasklist -o report.tasks PVS-Studio.log
```
Ahora, el archivo report.tasks contiene el reporte con los warnings y/o errores.

## 🏗️ Arquitectura

El sistema está compuesto por los siguientes componentes:

- **Master**: Coordina el juego y la comunicación entre componentes
- **Player**: Procesos que controlan a cada jugador
- **View**: Visualización del estado del juego
- **Libraries**: Módulos compartidos para gestión de memoria y sincronización

## 👥 Autores

- Juan Ignacio Cantarella (@jicanta)
- Ian Cruz Oniszczuk (@ianoniszczuk)
- Federico José Magri (@fedemagri23)

---
