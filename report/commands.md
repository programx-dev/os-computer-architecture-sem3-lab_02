# Команды для лаб 2

## Сборка

```bash
cd ~/dev/mai/os-computer-architecture-sem3/lab_02
```

```bash
cmake -S . -B build
```

```bash
cmake --build build
```

Следующие команды выполняются из build:

```bash
cd build
```

## Запуск

```bash
./parent < ../data/demo_basic.txt
```

## Базовый strace

```bash
strace -f -o ../report/strace/strace_f.txt ./parent < ../data/demo_basic.txt
```

-f: следить за двумя дочерними процессами.
-o: сохранить трассу в файл. Без -o strace выводит её в stderr.
В каталоге build находятся ./parent, ./child1 и ./child2. Знак < берёт ввод из файла.

## Короткий strace

```bash
strace -f -e trace=pipe2,clone,dup2,execve,read,write,close,wait4,exit_group -o ../report/strace/strace_short.txt ./parent < ../data/demo_basic.txt
```

-e trace=... оставляет только перечисленные системные вызовы:
pipe2 — создание каналов; clone — создание детей; dup2 — перенаправление stdin и stdout; execve — запуск дочерних программ; read/write — обмен данными; close — закрытие концов каналов; wait4 — ожидание детей; exit_group — завершение процесса.
В трассе также видны чтения libc и закрытия файлов загрузчиком.

## Показать PID через ps
В первом терминале, находясь в build, запустить:

```bash
./parent
```

Оставить программу ждать ввода. Во втором терминале выполнить:

```bash
ps -C parent,child1,child2 -o pid,ppid,stat,comm --forest
```

ps показывает текущие процессы. -C выбирает названия программ. -o выбирает столбцы PID, PPID, STAT и COMM. --forest показывает дерево.

## Коды завершения

```bash
./parent < ../data/demo_basic.txt
```

```bash
echo $?
```

При успехе ожидается 0. Команду echo $? нужно вводить сразу после ./parent: она печатает код последней команды.

Для ошибки вывода:

```bash
./parent < ../data/demo_basic.txt > /dev/full
```

```bash
echo $?
```

/dev/full всегда сообщает об ошибке записи. Для этого случая у parent предусмотрен код 3. Сообщение идёт в stderr.

Для ошибки запуска детей из неправильной рабочей папки:

```bash
cd ..
```

```bash
./build/parent < /dev/null
```

```bash
echo $?
```

```bash
cd build
```

./child1 и ./child2 ищутся относительно текущей папки.

## Перенаправление вывода
Только stderr на экране:

```bash
./parent < ../data/demo_basic.txt > /dev/null
```

stdout отброшен. При успешном запуске экран будет пустым, потому что ошибки не было. Для проверки с ошибкой можно использовать приведённую выше команду с /dev/full: её perror останется на экране.

Только stdout на экране:

```bash
./parent < ../data/demo_basic.txt 2> /dev/null
```

stderr отброшен.

stdout и stderr в разные файлы:

```bash
./parent < ../data/demo_basic.txt > ../report/out/stdout.txt 2> ../report/out/stderr.txt
```

stdout и stderr вместе в один файл:

```bash
./parent < ../data/demo_basic.txt > ../report/out/all.txt 2>&1
```

> перенаправляет stdout (дескриптор 1), 2> перенаправляет stderr (дескриптор 2), < задаёт stdin (дескриптор 0). /dev/null отбрасывает данные. 2>&1 копирует текущее направление stdout в stderr; порядок перенаправлений имеет значение.
