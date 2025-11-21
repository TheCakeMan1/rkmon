# Установка rkmon

Инструкция по сборке и установке модуля ядра.

## Требования
- Linux kernel headers (заголовки ядра)
- make
- gcc
- права root

Убедитесь, что заголовки установлены:
```sudo apt install linux-headers-$(uname -r)```
(или соответствующий пакет для вашей системы)

## Сборка

В корне проекта выполните:

```make```

После успешной сборки появится файл:
```rkmon.ko```

## Загрузка модуля

```sudo insmod rkmon.ko```

Проверить:
```dmesg | grep rkmon```

Ожидаемый вывод:
```
rkmon: loaded
rkmon: device major=XXX minor=0
```

Появится устройство:
```/dev/rkmon```

## Просмотр данных

```cat /dev/rkmon```

Пример вывода содержит:
- загрузку CPU
- частоты
- память
- список сетевых интерфейсов
- параметры каждого интерфейса

## IOCTL-примеры

### Установка частоты обновления
```c
int rate = 200; // ms
ioctl(fd, RKMON_SET_UPDATE_RATE, &rate);
```
Получение информации о сети
```c
struct rkmon_net_request req;
strncpy(req.ifname, "eth0", sizeof(req.ifname));
ioctl(fd, RKMON_GET_NETINFO, &req);
```
Удаление модуля
```sudo rmmod rkmon```
Проверка:
```
dmesg | grep rkmon
```
Очистка сборки
```make clean```
---
