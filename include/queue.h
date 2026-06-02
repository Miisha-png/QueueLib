/**
 * @file queue.h
 * @brief Библиотека для работы с очередью на динамическом кольцевом буфере.
 */
#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Структура очереди на динамическом кольцевом буфере.
 */
typedef struct {
    int    *data;     /**< Указатель на динамический массив элементов */
    size_t  head;     /**< Индекс первого элемента очереди */
    size_t  tail;     /**< Индекс следующей свободной ячейки для вставки */
    size_t  size;     /**< Текущее количество элементов в очереди */
    size_t  capacity; /**< Текущая максимальная ёмкость буфера */
} queue_t;

/**
 * @brief Изменяет размер внутреннего буфера очереди.
 * @details Выпрямляет кольцевой буфер в новый массив заданной ёмкости.
 * @param ptr_q       Указатель на очередь.
 * @param new_capacity Новая ёмкость буфера.
 * @return true в случае успеха, false при ошибке выделения памяти.
 */
bool queue_resize(queue_t *ptr_q, size_t new_capacity);

/**
 * @brief Создаёт и инициализирует новую очередь.
 * @param initial_capacity Начальная ёмкость буфера (должна быть больше 0).
 * @return Указатель на созданную очередь или NULL в случае ошибки.
 */
queue_t *queue_create(size_t initial_capacity);

/**
 * @brief Уничтожает очередь и освобождает всю выделенную память.
 * @param ptr_q Указатель на очередь. Если NULL, функция ничего не делает.
 */
void queue_destroy(queue_t *ptr_q);

/**
 * @brief Проверяет, пуста ли очередь.
 * @param ptr_q Указатель на очередь.
 * @return true, если в очереди нет элементов, иначе false.
 */
bool queue_is_empty(const queue_t *ptr_q);

/**
 * @brief Проверяет, заполнена ли очередь (достигла текущего лимита ёмкости).
 * @param ptr_q Указатель на очередь.
 * @return true, если буфер заполнен, иначе false.
 */
bool queue_is_full(const queue_t *ptr_q);

/**
 * @brief Добавляет элемент в конец очереди.
 * @details Если очередь заполнена, буфер автоматически расширяется вдвое.
 * @param ptr_q Указатель на очередь.
 * @param value Добавляемое целое число.
 * @return true, если элемент успешно добавлен, false при ошибке памяти.
 */
bool queue_push(queue_t *ptr_q, int value);

/**
 * @brief Извлекает и удаляет элемент из начала очереди (FIFO).
 * @param ptr_q       Указатель на очередь.
 * @param ptr_out_value Указатель на переменную для записи извлечённого значения.
 * @return true, если элемент успешно извлечён, false если очередь пуста.
 */
bool queue_pop(queue_t *ptr_q, int *ptr_out_value);

/**
 * @brief Просматривает первый элемент очереди без его удаления.
 * @param ptr_q       Указатель на очередь.
 * @param ptr_out_value Указатель на переменную для записи значения.
 * @return true, если элемент успешно прочитан, false если очередь пуста.
 */
bool queue_peek(const queue_t *ptr_q, int *ptr_out_value);

/**
 * @brief Возвращает текущее количество элементов в очереди.
 * @param ptr_q Указатель на очередь.
 * @return Количество элементов в очереди.
 */
size_t queue_get_size(const queue_t *ptr_q);

#endif /* QUEUE_H */

