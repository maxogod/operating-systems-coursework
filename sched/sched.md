# sched

Lugar para respuestas en prosa, seguimientos con GDB y documentación del TP.

Enunciado: https://fisop.github.io/website/tps/sched/#parte-2-scheduler-round-robin

## Parte 1: Cambio de contexto

Información de los registros antes de ejecutar iret en el context switch:

![before_iret](https://github.com/fiubatps/sisop_2024a_g13/assets/144845243/59abda60-8b4a-4390-8e87-946829793099)

Información de los registros despues de ejecutar iret:

![after_iret](https://github.com/fiubatps/sisop_2024a_g13/assets/144845243/7031b65e-61f4-4013-b438-08bfbee994f6)

Contenido del stack antes de comenzar el context swtich:

![before_first_inst](https://github.com/fiubatps/sisop_2024a_g13/assets/144845243/cb67b576-aef0-4470-bd95-57bc6780d71d)


Contenido del stack en la primer instrucción del context swtich:

![first_inst](https://github.com/fiubatps/sisop_2024a_g13/assets/144845243/22bebe31-b64a-4a84-b27e-0de21882b6d1)


Contenido del stack en la segunda instrucción del context swtich:

![second_inst](https://github.com/fiubatps/sisop_2024a_g13/assets/144845243/030203d4-b0f8-48b8-9ab7-e9099ece8201)


Contenido del stack en la tercera instrucción del context swtich:

![third_inst](https://github.com/fiubatps/sisop_2024a_g13/assets/144845243/35ce9897-417d-4e0c-a16e-29c013c97ad4)


Contenido del stack en la cuarta instrucción del context swtich:

![fourth_inst](https://github.com/fiubatps/sisop_2024a_g13/assets/144845243/1865e831-179c-469a-9e7f-cf075a549fdb)


Contenido del stack en la quinta instrucción del context swtich:

![fifth_inst](https://github.com/fiubatps/sisop_2024a_g13/assets/144845243/ffde6166-8b62-438e-9722-74929c8f153b)



## Parte 2: Scheduler round robin

En esta parte se implementó una política de scheduler de round robin, que itera todos los procesos de manera circular y a medida que se encuentran procesos en estado RUNNABLE, los ejecuta. Si no llega a encontrar ninguno en ese estado continúa ejecutándose el que estaba corriendo. En nuestro proyecto todas las pruebas para Round Robin pasan correctamente.

## Parte 3: Scheduler con prioridades

Para nuestra implementación de política de scheduling elegimos una variante del sistema MLFQ (Multi Level Feedback Queue), donde no se implementaron mediante colas, sino por distintos niveles de prioridad por números, yendo desde la mas alta ``0`` a la mas baja ``4``. Si un proceso se ejecuta, se incrementa un contador propio del proceso de veces ejecutadas, y en el caso de superar un máximo de veces ``SCHED_ALLOTMENT``, al proceso se le disminuye la prioridad a un numero mas alto si es que no supera a la mas baja y se le setea la cantidad de veces ejecutadas a cero.
Cada llamada a yield aumenta un contador, y para evitar que todos queden en prioridad baja, cuando el valor del contador llega a una cantidad determinada de veces ``CYCLES UNTIL BOOST``, la prioridad de todos los procesos se setean a 0 y sus veces ejecutadas también a 0.
Se consideró que si dos procesos comparten la misma prioridad y está en estado ``RUNNABLE``, se ejecuta el método de Round Robin para elegir qué proceso se va a ejecutar.
También determinamos que cuando se hace fork, el proceso hijo obtiene la misma prioridad que la del padre. Se implementó como se pidió en el enunciado, una syscall para obtener prioridades, y otra para modificar prioridades.

Reglas de MLFQ:
* Regla 1: Si Prioridad(A) > Prioridad(B), A se ejecuta (B no).
* Regla 2: Si Prioridad(A) = Prioridad(B), A & B se ejecutan en Round Robin.
* Regla 3: Cuando se crea un proceso, se le asigna la mayor prioridad.
* Regla 4: Cuando un proceso usa todo su `allotment` se le baja en 1 la prioridad.
* Regla 5: Despues de un periodo S, se boostean todos los procesos a la maxima prioridad.

Se crearon dos ejemplos para certificar que el usuario es capaz mediante syscalls disminuye la prioridad de un proceso, y que además no le es posible aumentarla. Los ejemplos son: ``testsyscall`` (se puede cambiar la prioridad) y ``testsyscallerr`` (no debería poder aumentar la prioridad).

``testsyscall``:


![imagen](https://github.com/fiubatps/sisop_2024a_g13/assets/144845243/4b6e4c75-3a7d-491f-9600-6af4b774282b)


``testsyscallerr``:


![testerr](https://github.com/fiubatps/sisop_2024a_g13/assets/144845243/c14ba213-3ff7-402f-8265-fc0d54ab03cc)

Referencias:
Operating Systems: Three Easy Pieces, capítulo 8: Scheduling: The Multi-Level Feedback Queue.
