## Primera Parte:

1. ¿Qué política de planificación utiliza xv6-riscv para elegir el próximo
   proceso a ejecutarse?

Utiliza RoundRobin, definido en proc.c

2. ¿Cuánto dura un quantum en xv6-riscv?

1000000 ciclos (pulsos de reloj), lo que es alrededor de 100 milisegundos en
qemu, es lo que se define en start.c como el tiempo antes de generar una
interrupcion de temporizador. Es decir, cada proceso se ejecuta durante ese
periodo tiempo antes de que se de un cambio de contexto

3. ¿Cuánto dura un cambio de contexto en xv6-riscv?

El cambio de contexto en xv6-riscv esta dado por la funcion swtch definida en
lenguaje ensamblador, en el archivo swtch.S, y se encarga de cargar y almacenar
los registros (del 0 al 11) utilizados por los procesos, asi como tambien la
direccion de retorno y el stack pointer; por lo que podemos decir que el cambio
de contexto dura, en determinada maquina, lo que tardan en ejecutarse
principalmente las 14 instrucciones de almacenamiento y las 14 instrucciones de
carga del swtch.S

4. ¿El cambio de contexto consume tiempo de un quantum?

Sí. Cuando el sistema operativo realiza un cambio de contexto, está cambiando de
la ejecución de un proceso a otro, lo que implica guardar el estado del proceso
actual y cargar el estado del nuevo proceso. Este proceso de guardar y cargar el
estado del proceso consume recursos de CPU y tiempo.

5. ¿Hay alguna forma de que a un proceso se le asigne menos tiempo?

Si observamos la implementacion de la politica de planificación, destacamos tres
funciones:

'scheduler' ejecuta el planificador en cada CPU

'sched' realiza un cambio de contexto entre el planificador y el proceso en
ejecucion, es decir, devuelve el control del CPU al planificador

La función ‘yield’ cambia el estado del proceso a listo y llama a sched. yield
es llamada en el archivo trap.c con el proposito de generar una interrupcion
cada vez que se cumplen los ciclos del reloj, devolviendo en esos casos el
control al planificador

En principio, no es posible asignarle menos tiempo de CPU a un proceso, por
ejemplo, el 50% del tiempo de Quantum. Si es posible generar un IO que llama a
yield() cediendo el resto del intervalo que le quedaba, pero no es posible
asignar, como tal, una fraccion del Quantum que le corresponde

6. ¿Cúales son los estados en los que un proceso pueden permanecer en xv6- riscv
   y que los hace cambiar de estado?

Hay seis estados posibles en xv6-riscv: UNUSED, USED, SLEEPING, RUNNABLE,
RUNNING y ZOMBIE Son diferentes funciones de proc.c que hace que los procesos
cambien de estado:

UNUSED: En principio, cuando se inicia un nuevo proceso, se encuentran en estado
UNUSED. (ver procinit)

USED: Cuando se crea el proceso, se busca en la tabla de procesos uno que se
encuentre en estado,UNUSED, y si se encuentra, se marca como USED. Luego se le
asigna un PID y un área de memoria para el trapframe, y se crea una tabla de
páginas para el proceso y establece un nuevo contexto para su ejecución (ver
allocproc)

SLEEPING: Un proceso cambia al estado SLEEPING cuando se quiere que el proceso
se detenga, y reanude luego que ocurra algun evento o condicion deseada (ver
sleep)

RUNNABLE: a diferencia de los estados mencionados hasta ahora, hay varias
funciones que asignan al proceso el estado RUNNABLE. Queda claro, que esto se
hace cuando se quiere que el planificador considere al proceso y le asigne
tiempo de ejecucion. A continuacion describimos las funciones que hacen a un
proceso RUNNABLE:

La función userinit se encarga de configurar el primer proceso de usuario,
tambien llamado init.

fork: queremos que el proceso hijo se ejecute

yield: cuando un proceso renuncia al CPU, queremos que siga siendo considerado
por el planificador

wakeup: despertamos a todos los procesos en SLEEPING, reanudando su ejecucion

kill: cuando queremos finalizar la ejecucion de un proceso, y este se encuentra
en estado\
SLEEPING, cambiamos a RUNNABLE ya que esto permite que el proceso complete
tareas de limpieza y liberación de recursos antes de su terminación

RUNNING: un proceso cambia a RUNNING en la funcion scheduler justo antes de
realizar un cambio de contexto entre el planificador y ese proceso, justamente
para darle tiempo de ejecucion

ZOMBIE: Cuando finalizamos la ejecucion de un programa, la funcion exit cambia
el estado del proceso a ZOMBIE, lo que indica que ha finalizado su ejecución,
pero su estado aún no se ha recogido por su proceso

# Segunda Parte

# Mediciones

HARDWARE Intel i5-3330 (4) 3.200GHz, NVIDIA GeForce GT 630 SOFTWARE QEMU version
6.2.0

## POLÍTICA SCHEDULER ROUND ROBIN

### QUANTUM 1000000

| Métrica                           | iobench | cpubench | cpubench & iobench | cpubench & cpubench | cpubench & iobench & cpubench |
| --------------------------------- | ------- | -------- | ------------------ | ------------------- | ----------------------------- |
| PROMEDIO MFLOPS/100T 1ER CPUBENCH |         | 516,235  | 511,938            | 444,133             | 651,688                       |
| PROMEDIO MFLOPS/100T 2DO CPUBENCH |         |          |                    | 657,125             | 645,312                       |
| PROMEDIO OPW/100T                 | 3917,47 |          | 35,5               |                     | 18,6                          |
| PROMEDIO OPR/100T                 | 3917,47 |          | 35,5               |                     | 18,6                          |
| CANT. SELECT IO                   | 246353  |          | 2237               |                     | 1236                          |
| CANT. SELECT 1ER CPU              |         | 2132     | 2107               | 2237                | 1051                          |
| CANT. SELECT 2DO CPU              |         |          |                    | 2153                | 1065                          |

### QUANTUM 100000

| Métrica                           | iobench | cpubench | cpubench & iobench | cpubench & cpubench | cpubench & iobench & cpubench |
| --------------------------------- | ------- | -------- | ------------------ | ------------------- | ----------------------------- |
| PROMEDIO MFLOPS/100T 1ER CPUBENCH |         | 4927.5   | 4635.67            | 6442.56             | 3563.31                       |
| PROMEDIO MFLOPS/100T 2DO CPUBENCH |         |          |                    | 6396.88             | 3378.83                       |
| PROMEDIO OPW/100T                 | 3952.42 |          | 334.588            |                     | 168,118                       |
| PROMEDIO OPR/100T                 | 3952.42 |          | 334.588            |                     | 168,118                       |
| CANT. SELECT IO                   | 261342  |          | 20961              |                     | 10507                         |
| CANT. SELECT 1ER CPU              |         | 20991    | 21254              | 10599               | 10575                         |
| CANT. SELECT 2DO CPU              |         |          |                    | 10518               | 10534                         |

## POLÍTICA SCHEDULER MLFQ

### QUANTUM 1000000

| Métrica                           | iobench | cpubench | cpubench & iobench | cpubench & cpubench | cpubench & iobench & cpubench |
| --------------------------------- | ------- | -------- | ------------------ | ------------------- | ----------------------------- |
| PROMEDIO MFLOPS/100T 1ER CPUBENCH |         | 516.71   | 511.938            | 491.571             | 646.75                        |
| PROMEDIO MFLOPS/100T 2DO CPUBENCH |         |          |                    | 493.071             | 644.375                       |
| PROMEDIO OPW/100T                 | 3712.79 |          | 35.5               |                     | 36.1                          |
| PROMEDIO OPR/100T                 | 3712.79 |          | 35.5               |                     | 36.1                          |
| CANT. SELECT IO                   | 233385  |          | 2237               | 2237                | 2219                          |
| CANT. SELECT 1ER CPU              |         | 4236     | 2109               | 2109                | 1062                          |
| CANT. SELECT 2DO CPU              |         |          |                    |                     | 1066                          |

### QUANTUM 100000

| Métrica                           | iobench   | cpubench | cpubench & iobench | cpubench & cpubench | cpubench & iobench & cpubench |
| --------------------------------- | --------- | -------- | ------------------ | ------------------- | ----------------------------- |
| PROMEDIO MFLOPS/100T 1ER CPUBENCH |           | 5054     | 4591.33            | 6437.06             | 6090.06                       |
| PROMEDIO MFLOPS/100T 2DO CPUBENCH |           |          |                    | 6443.69             | 6133.26                       |
| PROMEDIO OPW/100T                 | 3737.894  |          | 334.118            |                     | 176.70                        |
| PROMEDIO OPR/100T                 | 3737.894  |          | 334.118            |                     | 176.70                        |
| CANT. SELECT IO                   | 250463    |          | 20960              |                     | 10913                         |
| CANT. SELECT 1ER CPU              |           | 21314    | 21021              | 10496               | 10513                         |
| CANT. SELECT 2DO CPU              |           |          |                    | 10504               | 10597                         |

# RR

## QUANTUM 1M

#### Caso 1 IOBENCH:

Podemos ver la utilización de el proceso con PID = 3.

Realiza 149504 trabajos tanto de lectura como de escritura.

El tiempo de ejecucion es de 2000.

El proceso fue elejido por el scheduler un total de 246353 veces, realizando
0.606 trabajos por switch y su ultimo tiempo de ejecucion fue en el tick 2119.

#### Caso 2 CPUBENCH:

Podemos ver la utilización que el proceso con PID = 3.

Realiza 1677721600 trabajos de multiplicacion de matrices.

El proceso fue elejido por el scheduler un total de 2132 veces realizando
786.923 trabajos por switch y su ultimo tiempo de ejecucion fue en el tick 2157.

#### Caso 3 IOBENCH + CPUBENCH:

Podemos ver la utilización que los procesos con PIDs = 5 y 3, 3 para el CPU y 5
para el IO, realizando tareas de lectura y escritura + multiplicaciones de
matrices en simultaneo.

Realiza 1073741824 trabajos el CPU de Multiplicacion de matrices y 1408 trabajos
el IO tanto de lectura como de escritura.

El proceso de CPUBENCH fue elejido por el scheduler un total de 2107 veces
realizando 509.606 trabajos por switch y su ultimo tiempo de ejecucion fue en el
tick 2237.

El proceso de IOBENCH fue elejido por el scheduler un total de 2237 veces
realizando 0,65 trabajos por switch y su ultimo tiempo de ejecucion fue en el
tick 2153.

#### Caso 4 CPUBENCH + CPUBENCH:

Podemos ver la utilizacion que los procesos con PIDs = 5 y 3, 3 para el
CPUBENCH1 y 5 para el CPUBENCH2, realizando multiplicación de matrices en
simultaneo.

Realiza 4093640704 trabajos el CPUBENCH1 de Multiplicación de matrices y
268435456 trabajos el CPUBENCH2 de Multiplicación de matrices.

El proceso de CPUBENCH1 fue elejido por el scheduler un total de 1063 veces
realizando 3.851.026 trabajos por switch y su ultimo tiempo de ejecucion fue en
el tick 2168.

El proceso de CPUBENCH2 fue elejido por el scheduler un total de 1073 veces
realizando 250.172 trabajos por switch y su ultimo tiempo de ejecucion fue en el
tick 2174.

#### Caso 5 CPUBENCH &; CPUBENCH &; IOBENCH

Veremos el accionar de los procesos con PID's = 7 y 5 para el primer cpubench y
el segundo, respectivamete, y PID = 3 para el iobench.

El primer cpubench realiza 3892314112 operaciones, es elegido 1051 veces y su
último tiempo de ejecución fue en el tick 2107. Además, realizo 3.703.438,726
trabajos por switch.

El segundo cpubench realiza la misma cantidad de operaciones operaciones que el
primero, es elegido 1065 veces y su último tiempo de ejecución fue en el
tick 2117. Además, realizo 3.654.755,035 trabajos por switch.

El iobench realiza 768 operaciones de lectura/escritura, es elegido 1236 veces y
su última ejecución fue en el tiempo 2119. Además, realiza 0,62135 trabajos por
switch.

##### Comentarios:

Primero vemos que el **iobench** cuando corre solo es elegido muchas veces por
el scheduler. Luego al correr junto con **cpubench** decrementa
considerablemente las veces que es elegido, esto no ocurre con el **cpubench**
ya que se mantiene en valores parecidos corriendo solo o junto con otros
procesos. Lo mismo ocurre con las OPW y OPR en comparacion con los MFLOPS, se ve
mucho mas afectado **iobench**.

# RR

## QUANTUM 100K

#### Caso 1 IOBENCH

Podemos ver la utilización de el proceso con PID = 3.

Realiza 150912 trabajos tanto de lectura como de escritura.

El proceso fue elejido por el scheduler un total de 261342 veces, realizando
0.577 trabajos por switch y su ultimo tiempo de ejecucion fue en el tick 21161.

#### Caso 2 CPUBENCH

Podemos ver la utilización de el proceso con PID = 3.

Realiza 1073741824 trabajos.

El proceso fue elejido por el scheduler un total de 20991 veces realizando
51152,48 trabajos por switch y su ultimo tiempo de ejecucion fue en el
tick 21140.

#### Caso 3 IOBENCH + CPUBENCH:

Veremos la utilización de los procesos con PID's = 3 y 5 para el cpubench y el
iobench respectivamente.

El cpubench realiza 469762048 trabajos y el iobench 13056 trabajos de W/R.

El proceso de CPUBENCH fue elejido por el scheduler un total de 21254 veces
realizando 22102,28 trabajos por switch y su ultimo tiempo de ejecucion fue en
el tick 21818.

El proceso de IOBENCH fue elejido por el scheduler un total de 20961 veces
realizando 0,62 trabajos por switch y su ultimo tiempo de ejecucion fue en el
tick 21524.

#### Caso 4 CPUBENCH + CPUBENCH:

Podemos ver la utilizacion que los procesos con PIDs = 5 y 3, 3 para el
CPUBENCH1 y 5 para el CPUBENCH2, realizando multiplicacion de matrices en
simultaneo.

Realiza 3892314112 trabajos el CPUBENCH1 y 3892314112 trabajos el CPUBENCH2.

El proceso de CPUBENCH1 fue elejido por el scheduler un total de 10518 veces
realizando 370.062 trabajos por switch y su ultimo tiempo de ejecucion fue en el
tick 21557.

El proceso de CPUBENCH2 fue elejido por el scheduler un total de 10599 veces
realizando 367.234 trabajos por switch y su ultimo tiempo de ejecucion fue en el
tick 21644.

#### Caso 5 CPUBENCH + CPUBENCH + IOBENCH:

Podemos ver la utilizacion que los procesos con PIDs = 7,5,3 , 7 para el
CPUBENCH1, 5 para el CPUBENCH2 y 3 para el IO, con prioridad 0 para los 3,
realizando tareas de lectura y escritura + multiplicaciones de matrices en
simultaneo.

Realiza 1543503872 trabajos el CPUBENCH1 y 2147483648 trabajos el CPUBENCH2 y
6656 trabajos el IO tanto de lectura como de escritura.

El proceso de CPUBENCH1 fue elejido por el scheduler un total de 10534 veces
realizando 146.525 trabajos por switch y su ultimo tiempo de ejecucion fue en el
tick 21179.

El proceso de CPUBENCH2 fue elejido por el scheduler un total de 10575 veces
realizando 203.071 trabajos por switch y su ultimo tiempo de ejecucion fue en el
tick 21224.

El proceso de IOBENCH fue elejido por el scheduler un total de 10507 veces
realizando 0,63 trabajos por switch y su ultimo tiempo de ejecucion fue en el
tick 21122.

##### Comentarios:

Similar al quantum de 1 millon, con la diferencia de que los procesos son
elegidos muchas mas veces al ser un quantum mas chico. Al parecer aca se ve mas
afectado el **cpubench** al correr junto con **iobench**.

# MLFQ

## QUANTUM 1M

#### Caso 1 IOBENCH:

Podemos ver la utilizacion que el proceso con PID = 3 Y prioridad 2 realiza
141312 trabajos tanto de lectura como de escritura. El tiempo de ejecucion es
de 2000. El proceso fue elejido por el scheduler un total de 233385 veces,
realizando 0,60 trabajos por switch y su ultimo tiempo de ejecucion fue en el
tick 2125.

#### Caso 2 CPUBENCH:

Podemos ver la utilizacion que el proceso con PID = 4 y prioridad 0 (La mas
alta).

Realiza 1677721600 trabajos de Multiplicacion de matrices.

El proceso fue elejido por el scheduler un total de 4236 veces realizando
396.062 trabajos por switch y su ultimo tiempo de ejecucion fue en el tick 2400.

#### Caso 3 IOBENCH + CPUBENCH:

Podemos ver la utilizacion que los procesos con PIDs = 4 y 6 , 4 para el CPU y 6
para el IO, con prioridad 0 y 2 respectivamente, realizando tareas de lectura y
escritura + multiplicaciones de matrices en simultaneo.

Realiza 1073741824 trabajos el CPU de Multiplicacion de matrices y 1408 trabajos
el IO tanto de lectura como de escritura.

El proceso de CPUBENCH fue elejido por el scheduler un total de 2109 veces
realizando 509.123 trabajos por switch y su ultimo tiempo de ejecucion fue en el
tick 2168.

El proceso de IOBENCH fue elejido por el scheduler un total de 2237 veces
realizando 0,62 trabajos por switch y su ultimo tiempo de ejecucion fue en el
tick 2169.

#### Caso 4 CPUBENCH + CPUBENCH:

Podemos ver la utilizacion que los procesos con PIDs = 3 y , 5 para el CPUBENCH1
3 y 5 para el CPUBENCH2,con prioridades 0 los 2 procesos, realizando
multiplicacion de matrices en simultaneo.

Realiza 268435456 trabajos el CPUBENCH1 de Multiplicacion de matrices y
268435456 trabajos el CPUBENCH2 de Multiplicacion de matrices.

El proceso de CPUBENCH1 fue elejido por el scheduler un total de 1061 veces
realizando 253.002 trabajos por switch y su ultimo tiempo de ejecucion fue en el
tick 2173.

El proceso de CPUBENCH2 fue elejido por el scheduler un total de 1069 veces
realizando 251.108 trabajos por switch y su ultimo tiempo de ejecucion fue en el
tick 2182.

#### Caso 5 CPUBENCH + CPUBENCH + IOBENCH:

Podemos ver la utilizacion que los procesos con PIDs = 7,5,4. 7 para el
CPUBENCH1,5 para el CPUBENCH2 y 4 para el IO, con prioridad 0 para los cpubench
y 1 IOBENCH, realizando tareas de lectura y escritura + multiplicaciones de
matrices en simultaneo.

Realiza 3892314112 trabajos el CPUBENCH1 y 3892314112 trabajos el CPUBENCH2 de
Multiplicacion de matrices y 1408 trabajos el IO tanto de lectura como de
escritura.

El proceso de CPUBENCH1 fue elejido por el scheduler un total de 1062 veces
realizando 3.665.079 trabajos por switch y su ultimo tiempo de ejecucion fue en
el tick 2124.

El proceso de CPUBENCH2 fue elejido por el scheduler un total de 1066 veces
realizando 3.651.326 trabajos por switch y su ultimo tiempo de ejecucion fue en
el tick 2128.

El proceso de IOBENCH fue elejido por el scheduler un total de 2219 veces
realizando 0,63 trabajos por switch y su ultimo tiempo de ejecucion fue en el
tick 2130.

##### Comentarios:

Con MLFQ se puede notar que **iobench** cuando corre junto con **cpubench** sale
mas favorecido que con RR, ya que mantiene una prioridad mas alta. Especialmente
se nota con el ultimo caso en el cual es elegido el doble de veces que los otros
procesos, y genera casi el doble de operaciones OPW y OPR que en RR. En los
casos en los que corren solos se puede ver que son bastante similares.

# MLFQ

## QUANTUM 100K

#### Caso 1 IOBENCH:

Podemos ver la utilizacion que el proceso con PID = 3 Y prioridad 0 realiza
143104 trabajos tanto de lectura como de escritura. El tiempo de ejecucion es
de 2000. El proceso fue elejido por el scheduler un total de 250463 veces,
realizando 0,57 trabajos por switch y su ultimo tiempo de ejecucion fue en el
tick 21331.

#### Caso 2 CPUBENCH:

Podemos ver la utilizacion que el proceso con PID = 3 y prioridad 0 (La mas
alta).

Realiza 1073741824 trabajos de Multiplicacion de matrices.

El proceso fue elejido por el scheduler un total de 21314 veces realizando
50.377 trabajos por switch y su ultimo tiempo de ejecucion fue en el tick 21460.

#### Caso 3 IOBENCH + CPUBENCH:

Podemos ver la utilizacion que los procesos con PIDs = 3 y 5 , 3 para el CPU y 5
para el IO, con prioridad 0 y 1 respectivamente, realizando tareas de lectura y
escritura + multiplicaciones de matrices en simultaneo.

Realiza 469762048 trabajos el CPU de Multiplicacion de matrices y 13184 trabajos
el IO tanto de lectura como de escritura.

El proceso de CPUBENCH fue elejido por el scheduler un total de 21021 veces
realizando 22.347 trabajos por switch y su ultimo tiempo de ejecucion fue en el
tick 21607.

El proceso de IOBENCH fue elejido por el scheduler un total de 20960 veces
realizando 0,62 trabajos por switch y su ultimo tiempo de ejecucion fue en el
tick 21545.

#### Caso 4 CPUBENCH + CPUBENCH:

Podemos ver la utilizacion que los procesos con PIDs = 3 y 5, 5 para el
CPUBENCH1 3 y 5 para el CPUBENCH2,con prioridades 0 los 2 procesos, realizando
multiplicacion de matrices en simultaneo.

Realiza 4093640704 trabajos el CPUBENCH1 de Multiplicacion de matrices y
4093640704 trabajos el CPUBENCH2 de Multiplicacion de matrices.

El proceso de CPUBENCH1 fue elejido por el scheduler un total de 10496 veces
realizando 390.019 trabajos por switch y su ultimo tiempo de ejecucion fue en el
tick 21691.

El proceso de CPUBENCH2 fue elejido por el scheduler un total de 10504 veces
realizando 389.722 trabajos por switch y su ultimo tiempo de ejecucion fue en el
tick 21700.

#### Caso 5 CPUBENCH + CPUBENCH + IOBENCH:

Podemos ver la utilizacion que los procesos con PIDs = 7,5,3 , 7 para el
CPUBENCH1, 5 para el CPUBENCH2 y 3 para el IO, con prioridad 0 para los 3,
realizando tareas de lectura y escritura + multiplicaciones de matrices en
simultaneo.

Realiza 3489660928 trabajos el CPUBENCH1 y 3087007744 trabajos el CPUBENCH2 de
Multiplicacion de matrices y 6912 trabajos el IO tanto de lectura como de
escritura.

El proceso de CPUBENCH1 fue elejido por el scheduler un total de 10513 veces
realizando 331.937 trabajos por switch y su ultimo tiempo de ejecucion fue en el
tick 21165.

El proceso de CPUBENCH2 fue elejido por el scheduler un total de 10597 veces
realizando 291.309 trabajos por switch y su ultimo tiempo de ejecucion fue en el
tick 21250.

El proceso de IOBENCH fue elejido por el scheduler un total de 10913 veces
realizando 0,63 trabajos por switch y su ultimo tiempo de ejecucion fue en el
tick 21156.

##### Comentarios:

Podemos ver una tabla muy similar a RR con el mismo quantum. Esto se debe a que
la prioridad de los **iobench** baja, ya que con un quantum mas chico estos no
alcanzan a terminar antes de que se llame a yield.

# Se puede producir starvation en el nuevo planificador?

La respuesta es que sí. Esto pasa en el contexto que hay muchos procesos
interactivos: como estos procesos renuncian a la CPU antes de terminar su
intervalo de ejecucion, ascienden de prioridad, mientras que los procesos que
consumen todo su intervalo de CPU, se mentienen en las prioridades mas bajas.
Esto causa que, al haber muchos procesos interactivos, se repartan la CPU entre
ellos, sin dar tiempo de CPU al proceso largo.

Esto se soluciona con otra regla mas, que dice que cada un determinado tiempo,
todos los procesos vuelven a la prioridad mas alta, y de esa manera consiguen
mas tiempo de ejecucion
