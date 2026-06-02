/**
 * @file queue.c
 * @brief Реализация библиотеки очереди на динамическом кольцевом буфере.
 */
#include "queue.h"
#include <stdint.h>
#include <stdlib.h>

#define RESIZE_GROW_FACTOR   2U
#define RESIZE_SHRINK_FACTOR 2U
#define SHRINK_THRESHOLD     4U

/**
 * @brief Внутренняя функция для динамического изменения размера буфера.
 * @details Выпрямляет кольцевой буфер в новый массив увеличенного/уменьшенного размера.
 * @param ptr_q Указатель на очередь.
 * @param new_capacity Новая ёмкость буфера.
 * @return true в случае успеха, false при ошибке выделения памяти.
 */
bool queue_resize(queue_t *ptr_q, size_t new_capacity) {
    int32_t *ptr_new_data = (int32_t *)malloc(new_capacity * sizeof(int32_t));
    bool is_success = ptr_new_data;

    if (is_success) {
        /* Копируем элементы из старого кольцевого буфера в новый линейный массив */
        for (size_t ind = 0; ind < ptr_q->size; ind++) {
            ptr_new_data[ind] = ptr_q->data[(ptr_q->head + ind) % ptr_q->capacity];
        }

        /* Освобождаем старый буфер и обновляем указатели */
        free(ptr_q->data);
        ptr_q->data     = ptr_new_data;
        ptr_q->head     = 0;
        ptr_q->tail     = ptr_q->size; /* Хвост указывает на первую свободную ячейку */
        ptr_q->capacity = new_capacity;
    }

    return is_success;
}

queue_t *queue_create(size_t initial_capacity) {
    queue_t *ptr_q = NULL;

    if (initial_capacity) {
        ptr_q = (queue_t *)malloc(sizeof(queue_t));

        if (ptr_q) {
            ptr_q->data = (int32_t *)malloc(initial_capacity * sizeof(int32_t));

            if (ptr_q->data) {
                ptr_q->head     = 0;
                ptr_q->tail     = 0;
                ptr_q->size     = 0;
                ptr_q->capacity = initial_capacity;
            } else {
                free(ptr_q);
                ptr_q = NULL;
            }
        }
    }

    return ptr_q;
}

void queue_destroy(queue_t *ptr_q) {
    if (ptr_q) {
        free(ptr_q->data);
        ptr_q->data = NULL;
        free(ptr_q);
    }
}

bool queue_is_empty(const queue_t *ptr_q) {
    return !ptr_q || !ptr_q->size;
}

bool queue_is_full(const queue_t *ptr_q) {
    return ptr_q && (ptr_q->size == ptr_q->capacity);
}

bool queue_push(queue_t *ptr_q, int32_t value) {
    bool is_success = ptr_q;

    if (is_success && queue_is_full(ptr_q)) {
        /* Буфер заполнен — удваиваем ёмкость */
        is_success = queue_resize(ptr_q, ptr_q->capacity * RESIZE_GROW_FACTOR);
    }

    if (is_success) {
        ptr_q->data[ptr_q->tail] = value;
        ptr_q->tail = (ptr_q->tail + 1U) % ptr_q->capacity;
        ptr_q->size++;
    }

    return is_success;
}

bool queue_pop(queue_t *ptr_q, int32_t *ptr_out_value) {
    bool is_success = ptr_q && ptr_out_value && !queue_is_empty(ptr_q);

    if (is_success) {
        *ptr_out_value = ptr_q->data[ptr_q->head];
        ptr_q->head    = (ptr_q->head + 1U) % ptr_q->capacity;
        ptr_q->size--;

        /* Сжатие буфера, если он заполнен менее чем на 25% и размер больше минимума */
        if ((ptr_q->capacity > SHRINK_THRESHOLD) &&
            ptr_q->size &&
            (ptr_q->size <= ptr_q->capacity / (RESIZE_SHRINK_FACTOR * RESIZE_SHRINK_FACTOR))) {
            queue_resize(ptr_q, ptr_q->capacity / RESIZE_SHRINK_FACTOR);
        }
    }

    return is_success;
}

bool queue_peek(const queue_t *ptr_q, int32_t *ptr_out_value) {
    bool is_success = ptr_q && ptr_out_value && !queue_is_empty(ptr_q);

    if (is_success) {
        *ptr_out_value = ptr_q->data[ptr_q->head];
    }

    return is_success;
}

size_t queue_get_size(const queue_t *ptr_q) {
    size_t result = 0;

    if (ptr_q) {
        result = ptr_q->size;
    }

    return result;
}
