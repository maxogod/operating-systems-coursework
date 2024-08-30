# fisop-fs

Para nuestro File System decidimos implementar una estructura basada en bloques implementado estructuras de bloques y del filesystem.

## Estructura del File System

Nuestro file system consta de 2 structs packed (explicaremos el porque estan packed en la parte de **persistencia**)

```c
typedef struct __attribute__((__packed__)) block {
	blk_status_t status;      // Status of block (FREE || OCCUPIED)
	blk_type_t type;          // Type of block (FS_DIR || FS_FILE)
	char path[MAX_PATHNAME];  // Pathname of file
	char name[MAX_NAME];      // Name of file
	char data[MAX_CONTENT];   // Content of file
	mode_t mode;              // Protection (Read, Written or Executed)
	nlink_t nlink;            // Number of hard links
	uid_t id_user;            // ID of user
	off_t size;               // Total size, in bytes
	time_t access_time;       // Time of last access
	time_t modify_time;       // Time of last modification
} block_t;
```
Los bloques donde cada bloque representa una seccion de informacion donde se puede guardar archivos ó directorios
Cada bloque guarda una cantidad maxima de 4096 bytes (4kB) de informacion.
```c
typedef struct __attribute__((__packed__)) filesystem {
	block_t blocks[MAX_BLOCKS];
} filesystem_t;
```
Creamos una estructura `filesystem` que toma el rol de superbloque, y actua como contenedor para la totalidad de los bloques del sistema.

Nuestro superbloque consta de un array de bloques que representa el file system en su totalidad, en cada posicion del array se encuentra un bloque donde se puede colocar informacion. Al colocar data en un bloque, su status cambia de ser FREE (libre) a OCCUPIED (ocupado)

Para nuestro file system seleccionamos contener una cantidad maxima de 1024 bloques para poder guardar un total de 4MB

## Como el File System busca un archivo específico dado un path
Esto se logra a traves de la funcion: 
```c
block_t * get_block(filesystem_t *fs, const char *pathname)
```
Que consta en buscar un bloque en el sistema de archivos que tenga una ruta específica
(pathname) mediante comparaciones de cadenas y devuelve un puntero a ese bloque. Si no encuentra el bloque, devuelve NULL.

## Persistencia en nuestro File System

En esta seccion se aborda la carga y exportacion del filesystem desde y hacia disco. Los structs que utilizamos son packed ya que de esa manera esta funcionalidad de persistencia no dependera del sistema en el que este corriendo.

En el metodo 
```c 
int save_filesystem(filesystem_t *fs)
``` 
Esta funcion consiste en guardar el estado actual de un sistema de archivos en un archivo. Devuelve EXIT_SUCCESS si la operación tiene éxito, o EXIT_FAILURE si ocurre algún error.

Para guardar la informacion del filesystem, se le pasa a la funcion `fwrite` un puntero al `struct filesystem` y se exporta en un archivo de formato ```fisopfs```. Al contener un struct este archivo contiene informacion binaria.

En el metodo
```c 
void init_filesystem(filesystem_t *fs, char *save_file);
```
Se carga en memoria el file system desde disco, para esto se le pasa el path relativo a el `save file` del cual se quiere cargar. Si ningun archivo es dado se usara el archivo por default `save.fisopfs` si este no existe se inicializa el filesystem desde cero. El archivo que se use para inicializar sera el mismo que se usara al guardar.

## Tests

Es recomendado usar docker. De no usarlo, funcionan las pruebas y sus funciones igualmente, pero existen unos warnings debido al sistema.

Para correr los test, estando en el directorio `/fisopfs`, escribe en terminal:

```bash
make docker-run # opcional
make test
```
- El mismo ejecutable hace 'make' previamente antes de comenzar, el output del make se encuentra ignorado para no estorbar.
- En caso de que no pueda ser montado el filesystem, se cancelan las pruebas.
- Los tests si bien testean cada una de las funciones, para que pasen todas es progresivo y tienen que funcionar una por una, ya que utilizan la misma información a medida que se va creando.
- Si todas las pruebas pasan, solo se muestra PASSED, pero si alguna falla, se imprime su salida de la ejecución y luego lo esperado.
- Si una o más pruebas fallan, el codigo de salida de `test.sh` es 1, de lo contrario, si todas pasan es 0.

![image](https://github.com/fiubatps/sisop_2024a_g13/assets/105026197/d55ef833-5ad6-48c8-8b8c-e77849e48854)

![image](https://github.com/fiubatps/sisop_2024a_g13/assets/105026197/bb67d9a3-59eb-4da8-af39-78f2f7e64854)

### Tests automaticos: Docker

Para correr los test automaticamente con docker, debera hacerse dentro de la carpeta `/fisopfs`, primero:

```bash
docker build -f Dockerfile.test -t filesystem-test:lastest .
```

Y luego

```bash
docker run \
	--device /dev/fuse \
	--cap-add SYS_ADMIN \
	--security-opt apparmor:unconfined \
	filesystem-test:lastest
```

*(necesario para github action)*
