#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

/**
 * @brief Выводит текущее состояние очереди.
 * @param ptr_q Указатель на очередь.
 */
void print_queue_status(const queue_t *ptr_q) {
    if (!ptr_q) {
        printf("Очередь не существует.\n");
    } else {
        printf("[Статус] Элементов: %zu / Ёмкость: %zu | Пуста: %s | Полна: %s\n",
               queue_get_size(ptr_q),
               ptr_q->capacity,
               queue_is_empty(ptr_q) ? "Да" : "Нет",
               queue_is_full(ptr_q)  ? "Да" : "Нет");
    }
}

int main(void) {
    size_t initial_cap = 3;
    int    peek_val    = 0;
    int    popped_val  = 0;
    int    exit_code   = EXIT_SUCCESS;

    printf("Старт демонстрации работы очереди:\n\n");

    /* 1. Создание очереди с небольшой начальной ёмкостью */
    printf("1. Создаём очередь с начальной ёмкостью: %zu\n", initial_cap);
    queue_t *ptr_my_queue = queue_create(initial_cap);

    if (!ptr_my_queue) {
        fprintf(stderr, "Не удалось создать очередь.\n");
        exit_code = EXIT_FAILURE;
    }

    if (!exit_code) {
        print_queue_status(ptr_my_queue);
        printf("=-=-=-=-=-=\n");

        /* 2. Заполнение очереди до предела */
        printf("2. Добавляем элементы: 10, 20, 30\n");
        queue_push(ptr_my_queue, 10);
        queue_push(ptr_my_queue, 20);
        queue_push(ptr_my_queue, 30);
        print_queue_status(ptr_my_queue);

        if (queue_peek(ptr_my_queue, &peek_val)) {
            printf("Первый элемент в очереди (peek): %d\n", peek_val);
        }
        printf("=-=-=-=-=-=\n");

        /* 3. Демонстрация автоматического расширения буфера */
        printf("3. Добавляем 4-й элемент (40). Очередь должна расшириться вдвое.\n");
        if (queue_push(ptr_my_queue, 40)) {
            printf("Элемент 40 успешно добавлен.\n");
        }
        print_queue_status(ptr_my_queue); /* Ёмкость должна стать 6 (3 * 2) */
        printf("=-=-=-=-=-=\n");

        /* 4. Извлечение нескольких элементов (FIFO) */
        printf("4. Извлекаем 2 элемента из очереди:\n");
        for (size_t ind = 0; ind < 2; ind++) {
            if (queue_pop(ptr_my_queue, &popped_val)) {
                printf("  Извлечён элемент: %d\n", popped_val);
            }
        }
        print_queue_status(ptr_my_queue);
        printf("=-=-=-=-=-=\n");

        /* 5. Проверка кольцевого буфера: добавление после извлечения */
        printf("5. Добавляем 50 и 60, задействуем освободившееся место по кольцу:\n");
        queue_push(ptr_my_queue, 50);
        queue_push(ptr_my_queue, 60);
        print_queue_status(ptr_my_queue);
        printf("=-=-=-=-=-=\n");

        /* 6. Опустошение очереди */
        printf("6. Извлекаем все оставшиеся элементы:\n");
        while (!queue_is_empty(ptr_my_queue)) {
            if (queue_pop(ptr_my_queue, &popped_val)) {
                printf("  Извлечён элемент: %d\n", popped_val);
            }
        }
        print_queue_status(ptr_my_queue);
        printf("=-=-=-=-=-=\n");

        /* 7. Попытка извлечения из пустой очереди */
        printf("7. Пробуем pop из пустой очереди:\n");
        if (!queue_pop(ptr_my_queue, &popped_val)) {
            printf("Ошибка: очередь пуста, извлечь элемент нельзя (как и требуется).\n");
        }
        printf("=-=-=-=-=-=\n");


        /* 8. Освобождение памяти */
        printf("8. Уничтожаем очередь и освобождаем память.\n");
        queue_destroy(ptr_my_queue);
        ptr_my_queue = NULL;

        printf("\nДемонстрация успешно завершена\n");
    }

    return exit_code;
}
