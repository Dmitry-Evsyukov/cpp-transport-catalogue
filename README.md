# cpp-transport-catalogue
# Проект: Транспортный справочник

# Описание проекта
Данный проект нужен для оптимизации процессов связанных с построением маршрутов и карт.
Он получает на вход координаты автобусных остановок с названиями и расстоянием между собой, а также автобусные маршруты.
Он может построить svg-карту, выдать данные об автобусном маршруте или остановке, найти оптимальный маршрут между остановками. Маршрутизатор служит для наглядной визуализации автобусных путей, помогает не тратить время для рассматривания всех возможных дорог из Пункта А в пункт Б. Он даёт пошаговую инструкцию маршрута, это очень удобно. 


# Инструкция по развертыванию и использованию
1. Клонировать репозиторий с кодом
2. Собрать проект с помощью CMakeLists.txt
3. Входные и выходные данные представлены в формате Json, примеры есть в папке data

# Системные требования
1. C++17(STL)
2. CMake 3.22.0
3. GCC 11.2.0

# Планы по доработке
1. Добавить визуализацию оптимального маршрута
2. Добавить новый функционал, когда одна из станций, например, не работает, не строить маршрут через нее или отображать 
это на карте.
