# Operating-systems
## Лабораторная #1
#### Основная цель лабораторной работы 
— знакомство с системными инструментами анализа производительности и поведения программ. В данной лабораторной работе Вам будет предложено произвести нагрузочное тестирование Вашей операционной системы при помощи инструмента stress-ng.

В качестве тестируемых подсистем использовать: cpu, cache, io, memory, network, pipe, scheduler.

Для работы со счетчиками ядра использовать все утилиты, которые были рассмотренны на лекции (раздел 1.9, кроме kdb)

Ниже приведены списки параметров для различных подсистем (Вам будет выдано 2 значения для каждой подсистемы согласно варианту в журнале). Подбирая числовые значения для выданных параметров, и используя средства мониторинга, добиться **максимальной** производительности системы (BOGOPS, FLOPS, Read/Write Speed, Network Speed).

###### Параметры для cpu:

all ackermann apery bitops callfunc cdouble cfloat clongdouble collatz correlate cpuid crc16 decimal32 decimal64 decimal128 dither div16 div32 div64 double euler explog factorial fibonacci fft fletcher16 float float32 float64 float80 float128 floatconversion gamma gcd gray hamming hanoi hyperbolic idct int128 int64 int32 int16 int8 int128float int128double int128longdouble int128decimal32 int128decimal64 int128decimal128 int64float int64double int64longdouble int32float int32double int32longdouble intconversion ipv4checksum jmp lfsr32 ln2 logmap longdouble loop matrixprod nsqrt omega parity phi pi prime psi queens rand rand48 rgb sieve stats sqrt trig union zeta

###### Параметры для cache:

cache-fence, cache-level, cache-prefetch, cache-ways, l1cache, l1cache-line-size, l1cache-sets, l1cache-ssize, l1cache-ways, prefetch-l3-size, stream-l3-size

###### Параметры для io:

iomix, ioport, ioprio, io-uring

###### Параметры для memory:

fork-vm, lockbus, madvise, mcontend, memfd, memfd-fds, memrate, memthras, misaligned-method, mmaphuge-mmaps, prefetch, shm, zlib-mem-level

###### Параметры для network:

dccp, netdev, netlink-proc, netlink-task, sockdiag

###### Параметры для pipe:

oom-pipe, pipe-ops, pipe-data-size, pipe-size, pipeherd, pipeherd-yield, sigpipe

###### Параметры для sched:

sched-prio, sched-period, sched-runtime, sched-deadline, resched, schedpolicy, yield

#### Построить графики (подходящие по заданию.):
- Потребления программой CPU;
- Нагрузки, генерируемой программой на подсистему ввода-вывода;
- Нагрузки, генерируемой программой на сетевую подсистему;
- Другие графики, необходимые для демонстрации работы.

#### Содержание отчета:
1. Описание шагов выполненных для сбора информации (включая исходные тексты всех использованных скриптов и вспомогательных программ);
2. Полученные графики, результаты выполнения скриптов и вспомогательных программ;
3. Выводы по работе.

#### Темы для подготовки к защите лабораторной работы:
1. Структура процесса;
2. Виртуальная память;
3. Системные утилиты сбора статистики ядра;
4. Основы ввода-вывода (блочный и последовательный ввод-вывод);
5. Файловая система procfs;
6. Использование утилиты strace, ltrace, bpftrace;
7. Профилирование и построение flamegraph'а и stap;
