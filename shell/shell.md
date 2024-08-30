# shell

### Búsqueda en $PATH

#### ¿cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?
La mayor diferencia que aparta `execve(2)` de los demas wrappers de la libreria estandar de C es que 
`execve(2)` es una llamada al sistema, toma los argumentos:

`pathname`: Especifica la ruta al archivo ejecutable que deseas ejecutar. Puede ser una ruta absoluta (comenzando desde el directorio raíz) o una ruta relativa (relativa al directorio de trabajo actual).

`argv[]`: Este parámetro es un array de cadenas que representan los argumentos de línea de comandos pasados al nuevo programa. Convencionalmente, se termina con un puntero NULL. El primer elemento (argv[0]) generalmente representa el nombre del programa que se está ejecutando. Los elementos subsiguientes (argv[1], argv[2], etc.) 

`envp[]`: Este parámetro es un array de cadenas que representan las variables de entorno pasadas al nuevo programa. Al igual que argv, se termina con un puntero NULL. Cada cadena en este array tiene el formato "variable=valor", donde variable es el nombre de la variable de entorno y valor es su valor.

`int execve(const char *pathname, char *const argv[], char *const envp[]);`

`execve(2)` es más bajo nivel y flexible, pero requiere más trabajo manual para configurar 
todos los argumentos y el entorno correctamente. Las funciones de la familia `exec(3)` 
proporcionan una interfaz más fácil de usar y manejan algunos detalles, como 
la búsqueda del programa ejecutable en el `PATH`, pero son menos flexibles en 
términos de cómo se pasan los argumentos y el entorno.

#### ¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?
La llamada `exec(3)` puede fallar por varias razones, como la incapacidad de encontrar el archivo 
ejecutable especificado, permisos insuficientes para ejecutar el archivo, o errores 
relacionados con la carga y ejecución del programa. Cuando una llamada a `exec(3)` 
falla, la función devuelve `-1` y establece la variable errno para indicar el tipo de error que ocurrió.

---

### Procesos en segundo plano

#### Detallar cuál es el mecanismo utilizado para implementar procesos en segundo plano.

Para ejecutar procesos en segundo plano, se emplea la llamada al sistema fork() seguida de exec() en el proceso hijo, tal como en cualquier comando de la shell. 
Sin embargo, los procesos en segundo plano se distinguen por no ser bloqueantes, es decir, el usuario no tiene que esperar a que finalicen los comandos para continuar utilizando la shell. Esto permite que el proceso hijo se ejecute en segundo plano mientras el proceso padre sigue en primer plano.

La estructura backcmd cuenta con un atributo adicional en comparación con otras estructuras de cmd: un struct de cmd adicional.
Este atributo de cmd facilita la ejecución del proceso hijo en segundo plano, mientras que el proceso padre continúa en primer plano mediante funciones recursivas, pasando como parámetro el atributo cmd de backcmd.

---

### Flujo estándar

#### Investigar el significado de 2>&1, explicar cómo funciona su forma general
La expresión `2>&1` se utiliza en la `shell` para redirigir la salida de error estándar `(stderr)` 
a la misma ubicación que la salida estándar `(stdout)`.

La sintaxis `2>&1` en la línea de comandos se interpreta de la siguiente manera:
* `2>`: Indica la redirección del descriptor de archivo 2 `(stderr)`.
* `&1`: Indica que se redirigirá a donde está redirigido el descriptor de archivo 1 `(stdout)`.

#### Mostrar qué sucede con la salida de cat out.txt en el ejemplo.

```shell
$ cat out.txt
/home:
<username>
        Program: [cat out.txt] exited, status: 0
```

#### Luego repetirlo, invirtiendo el orden de las redirecciones (es decir, 2>&1 >out.txt). ¿Cambió algo? Compararlo con el comportamiento en bash(1).

En el comando `2>&1 >out.txt`, primero se redirige la salida de error estándar `(stderr)` 
al mismo lugar que la salida estándar `(stdout)`. Luego, se redirige la salida estándar al archivo `out.txt`.

Tanto en bash como en nusetra shell, hace lo mismo pero la unica diferencia es que en vez de dar un 
`exit status de 0`, nos devuelve un exit status de `-11` y que el proceso fue `'killed'`

---

### Tuberías múltiples

#### Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe
Cuando se ejecuta un pipe en la shell, el código de salida que se reporta generalmente es el de la última tarea
en la secuencia del pipe. En un pipe, cada comando dentro del pipe se ejecuta como un proceso separado, y el 
resultado de cada comando se pasa al siguiente mediante la tubería. Si un comando falla dentro del pipe, la shell constesta
el error de dicho comando.

En nuestra implementacion de shell, el esqueleto incluye una linea de codigo en `printstatus.c` que indica lo siguiente:

`if (strlen(cmd->scmd) == 0 || cmd->type == PIPE)
return;`

Que indica que al pipear los comandos, el exit status de cada comando no se imprime.

#### ¿Cambia en algo?
Al comentar las lineas que mencionamos previamente, el exit status del ultimo comando del pipe se imprime en la terminal.


#### ¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este comportamiento usando bash. Comparar con su implementación.
Por ejemplo si corro el siguiente comando en bash:

    cat text.txt | wc -l 

produce el siguiente error: [ asumiendo que no esta text.txt en el directorio]

    cat: text.txt: No such file or directory
    0

esto ocurre porque el comando `cat text.txt` falla y el comando `wc -l` recibe un input vacio, esto tiene sentido ya que comienza
ejecutando el primer comando y si este falla, el siguiente comando recibe un input vacio y por lo tanto da 0.

La salida mostrada, anteriormente, ocurre tanto en la implmentacion de bash como en nuestra shell.

Si corremos `echo $?`, la varible magica correspondiente al codigo de salida, el resultado en ambas shells es:

    0

---

### Variables de entorno temporarias

####  ¿Por qué es necesario hacerlo luego de la llamada a fork(2)?

Es necesario llamarlo luego de hacer la llamada fork, ya que si se hiciera antes se modifican las variables de entorno del padre y se busca modificar las del hijo, es decir solo el mismo proceso que se está llamando en la etapa de exec command. Por eso son variables temporarias, tienen el tiempo de vida en lo que dura el proceso hijo.

#### En algunos de los wrappers de la familia de funciones de `(exec(3))` (las que finalizan con la letra e), les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar `(setenv(3))` por cada una de las variables, se guardan en un arreglo y se lo coloca en el tercer argumento de una de las funciones de `(exec(3))`. ¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué. Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.

El comportamiento es distinto, ya que si se le pasa un arreglo de variables a una función de la familia que terminan en `e` de `(exec(3))`, NO podría heredar las variables que tiene el padre al momento de ser llamado el proceso, por lo tanto solo tendría disponibles en el momento que se lo llama, las variables que se les fueron brindadas por argumento.

Para conseguir que el comportamiento sea el mismo, habría que crear un array con todas las variables de entorno del padre (la shell), agregar al array todas las nuevas que se quieren crear, y simplemente pasarselo al wrapper de `(exec())` como tercer parámetro.

---

### Pseudo-variables

#### Investigar al menos otras tres variables mágicas estándar, y describir su propósito.
Incluir un ejemplo de su uso en bash (u otra terminal similar).

`$#` : devuelve el numero de argumentos pasados al programa

`$*` : devuelve todos los argumentos pasados al programa como un arreglo

`$@` : devuelve todos los argumentos pasados al programa en distintos arreglos

`$$` : devuelve el id de la shell actualmente corriendo

`$!` : devuelve el id del último proceso corriendo en segundo plano

un ejemplo de un script para usar `$#`:

>        function login(){
>
>            if [[ ${#} < 4 ]]; then
>
>                echo "Bad Syntax. Usage: -user [username] -pass [password] required"
>
>            return
>
>            fi
>
>        }


---

### Comandos built-in

#### ¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como true y false)

El comando `pwd` podría implementarse sin necesariamente ser un built-in, ya que al existir una variable de entorno 'PWD' que es donde se almacena el valor de la ubicacion actual, luego se puede preguntar por ella y realizar la misma consulta.
Hay varias ventajas en hacerlo built-in, una podría ser en que es mas óptimo en incluirlo con el resto de los built-in ya que simplemente es hacer un chequeo del ingreso en conjunto con el resto de los posibles comandos y hacer uso de la información del estado interno de la shell sin la necesidad de usar de variables de entorno o llamadas al sistema adicionales. Además de quitar la necesidad de crear un proceso hijo para consultarlo, lo hace más eficiente. A su vez no tendria sentido que `cd` sea un binario no built-in, ni tampoco funcionaría, ya que el cambiar entre directorios es algo personal de cada proceso.

---

### Segundo plano avanzado

#### ¿Por qué es necesario el uso de señales?

Se usan las señales con el fin de manejar el flujo del programa con distintos manejos de señales (handlers) al comunicarse entre procesos. Los procesos hijos al terminar su proceso liberan una señal del tipo SIGCHILD el cual nosotros nos encargamos de atraparla y realizar una tarea específica sabiendo que tipo de proceso terminó, solamente se le hace un tratamiento especial a los procesos background (los identificamos definiendo que siempre sus pid son iguales a su pgid).


### Historial

Sin Hacer 

---
