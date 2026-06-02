#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "queue.h"

/** Структура для накопления счётчиков прошедших/упавших тестов. */
typedef struct {
    size_t passed; /**< Число прошедших тестов. */
    size_t failed; /**< Число упавших тестов.   */
} test_state_t;

/**
 * Проверить условие и обновить счётчики.
 */
#define TEST_CHECK(expr, msg, ptr_ts)     \
    do {                                  \
        if (expr) {                       \
            printf("[PASS] %s\n", (msg)); \
            (ptr_ts)->passed++;           \
        } else {                          \
            printf("[FAIL] %s\n", (msg)); \
            (ptr_ts)->failed++;           \
        }                                 \
    } while (0)

void test_queue_create_destroy(test_state_t *ptr_ts)
{
    printf("\n[Группа] queue_create / queue_destroy\n");

    // Типовой случай: создание очереди с валидным размером
    {
        queue_t *ptr_q = queue_create(4);
        TEST_CHECK(ptr_q != NULL, "создание: указатель не NULL", ptr_ts);
        if (ptr_q) {
            TEST_CHECK(queue_get_size(ptr_q) == 0, "создание: начальный размер == 0", ptr_ts);
            TEST_CHECK(queue_is_empty(ptr_q),     "создание: очередь пуста",        ptr_ts);
            TEST_CHECK(!queue_is_full(ptr_q),     "создание: очередь не полная",    ptr_ts);
        }
        queue_destroy(ptr_q);
    }

    // Граничный случай: initial_capacity == 0 (должен вернуть NULL)
    {
        queue_t *ptr_q = queue_create(0);
        TEST_CHECK(ptr_q == NULL, "граничный: capacity == 0 возвращает NULL", ptr_ts);
    }

    // Ошибочный случай: уничтожение NULL-указателя не должно приводить к crash
    {
        queue_destroy(NULL);
        TEST_CHECK(1, "безопасность: queue_destroy(NULL) безопасен", ptr_ts);
    }
}

void test_queue_state_checks(test_state_t *ptr_ts)
{
    printf("\n[Группа] проверки состояния (empty/full/size)\n");

    // Защита от NULL-указателей
    {
        TEST_CHECK(queue_is_empty(NULL) == true,  "NULL ptr: is_empty == true",  ptr_ts);
        TEST_CHECK(queue_is_full(NULL)  == false, "NULL ptr: is_full == false",  ptr_ts);
        TEST_CHECK(queue_get_size(NULL) == 0,     "NULL ptr: get_size == 0",     ptr_ts);
    }

    // Изменение состояний в процессе жизненного цикла
    {
        queue_t *ptr_q = queue_create(2);

        TEST_CHECK(queue_is_empty(ptr_q) == true, "живая очередь: пустая при создании", ptr_ts);

        queue_push(ptr_q, 10);
        TEST_CHECK(queue_get_size(ptr_q) == 1,     "размер увеличился до 1", ptr_ts);
        TEST_CHECK(queue_is_empty(ptr_q) == false, "очередь больше не пустая", ptr_ts);
        TEST_CHECK(queue_is_full(ptr_q)  == false, "очередь еще не заполнена", ptr_ts);

        queue_push(ptr_q, 20);
        TEST_CHECK(queue_get_size(ptr_q) == 2,    "размер увеличился до 2", ptr_ts);
        TEST_CHECK(queue_is_full(ptr_q)  == true, "очередь заполнена до отказа", ptr_ts);

        queue_destroy(ptr_q);
    }
}

void test_queue_push_pop_peek(test_state_t *ptr_ts)
{
    printf("\n[Группа] push / pop / peek\n");

    queue_t *ptr_q   = queue_create(3);
    int      out_val = 0;

    // Передача NULL в качестве аргументов
    {
        TEST_CHECK(queue_push(NULL, 42) == false,            "push в NULL очередь -> false",   ptr_ts);
        TEST_CHECK(queue_pop(NULL, &out_val) == false,       "pop из NULL очереди -> false",   ptr_ts);
        TEST_CHECK(queue_pop(ptr_q, NULL) == false,          "pop в NULL приемник -> false",   ptr_ts);
        TEST_CHECK(queue_peek(NULL, &out_val) == false,      "peek из NULL очереди -> false",  ptr_ts);
        TEST_CHECK(queue_peek(ptr_q, NULL) == false,         "peek в NULL приемник -> false",  ptr_ts);
        TEST_CHECK(queue_peek(ptr_q, &out_val) == false,     "peek из пустой очереди -> false", ptr_ts);
    }

    // Типовой FIFO сценарий
    {
        TEST_CHECK(queue_push(ptr_q, 100) == true, "push 100: OK", ptr_ts);
        TEST_CHECK(queue_push(ptr_q, 200) == true, "push 200: OK", ptr_ts);

        // Тестируем чтение без удаления (peek)
        TEST_CHECK(queue_peek(ptr_q, &out_val) == true, "peek: OK", ptr_ts);
        TEST_CHECK(out_val == 100,                      "peek: прочитано верхнее значение (100)", ptr_ts);
        TEST_CHECK(queue_get_size(ptr_q) == 2,          "peek не изменил фактический размер", ptr_ts);

        // Тестируем извлечение первого элемента (pop)
        TEST_CHECK(queue_pop(ptr_q, &out_val) == true, "pop 1: OK", ptr_ts);
        TEST_CHECK(out_val == 100,                     "pop 1: получено значение 100", ptr_ts);
        TEST_CHECK(queue_get_size(ptr_q) == 1,         "размер уменьшился до 1", ptr_ts);

        // Тестируем извлечение второго элемента (pop)
        TEST_CHECK(queue_pop(ptr_q, &out_val) == true, "pop 2: OK", ptr_ts);
        TEST_CHECK(out_val == 200,                     "pop 2: получено значение 200", ptr_ts);

        // Очередь опустошена
        TEST_CHECK(queue_is_empty(ptr_q) == true,       "очередь пуста", ptr_ts);
        TEST_CHECK(queue_pop(ptr_q, &out_val) == false, "pop из пустой очереди -> false", ptr_ts);
    }

    queue_destroy(ptr_q);
}

void test_queue_dynamic_resize(test_state_t *ptr_ts)
{
    printf("\n[Группа] динамическое изменение размера (grow/shrink)\n");

    // Проверка логики расширения (Grow factor = 2)
    queue_t *ptr_q = queue_create(2);
    if (ptr_q) {
        queue_push(ptr_q, 1);
        queue_push(ptr_q, 2);
        TEST_CHECK(queue_is_full(ptr_q) == true, "очередь заполнена (size==2, capacity==2)", ptr_ts);

        // Этот push должен инициировать внутренний queue_resize
        TEST_CHECK(queue_push(ptr_q, 3)  == true, "push 3 (аллокация/расширение): OK", ptr_ts);
        TEST_CHECK(queue_get_size(ptr_q) == 3,   "размер стал равен 3", ptr_ts);
        TEST_CHECK(!queue_is_full(ptr_q),        "очередь перестала быть полной", ptr_ts);

        // Доводим внутренний capacity до 8 (накапливаем 5 элементов)
        queue_push(ptr_q, 4);
        queue_push(ptr_q, 5); // При добавлении 5-го элемента при capacity==4 произошел ресайз до 8.

        // Проверяем логику сжатия (Shrink)
        // Условие сжатия: capacity > 4 && size > 0 && size <= capacity / 4
        // Текущее состояние: capacity = 8, size = 5.
        int out_val = 0;

        queue_pop(ptr_q, &out_val); // size = 4 (4 > 8/4) -> без сжатия
        queue_pop(ptr_q, &out_val); // size = 3 (3 > 8/4) -> без сжатия

        // Следующий pop снизит size до 2. Проверяем: 2 <= (8 / 4), то есть 2 <= 2 — сжатие сработает!
        TEST_CHECK(queue_pop(ptr_q, &out_val) == true, "pop 3 (вызов shrink): OK", ptr_ts);

        // Проверим, что данные не потерялись и извлекаются корректно в рамках FIFO после ресайзов
        queue_pop(ptr_q, &out_val);
        TEST_CHECK(out_val == 4, "FIFO после shrink: извлечено правильное значение (4)", ptr_ts);

        queue_pop(ptr_q, &out_val);
        TEST_CHECK(out_val == 5, "FIFO после shrink: извлечено правильное значение (5)", ptr_ts);
    }
    queue_destroy(ptr_q);
}

void test_ring_buffer_concurrency(test_state_t *ptr_ts)
{
    printf("\n[Группа] работа кольцевого буфера (перемотка индексов)\n");

    // Задача: зациклить индексы head/tail по кругу, проверяя корректность сдвигов по модулю capacity
    queue_t *ptr_q = queue_create(3);
    if (ptr_q) {
        int out = 0;

        queue_push(ptr_q, 10);
        queue_push(ptr_q, 20);

        // Смещаем head вперед на 1 позицию
        queue_pop(ptr_q, &out); // out = 10; head указывает на индекс 1

        // Добавляем элементы дальше, заставляя tail обернуться через конец массива в начало (индекс 0)
        queue_push(ptr_q, 30);
        queue_push(ptr_q, 40); // Очередь заполнена (size == 3, capacity == 3), без ресайза.

        TEST_CHECK(queue_is_full(ptr_q) == true, "кольцевой буфер заполнен внахлест", ptr_ts);

        // Проверяем, последовательно ли вычитываются данные из кольца
        queue_pop(ptr_q, &out);
        TEST_CHECK(out == 20, "кольцо FIFO: прочитано 20", ptr_ts);

        queue_pop(ptr_q, &out);
        TEST_CHECK(out == 30, "кольцо FIFO: прочитано 30", ptr_ts);

        queue_pop(ptr_q, &out);
        TEST_CHECK(out == 40, "кольцо FIFO: прочитано 40", ptr_ts);

        TEST_CHECK(queue_is_empty(ptr_q) == true, "кольцо полностью очищено", ptr_ts);
    }
    queue_destroy(ptr_q);
}

void test_large_queue(test_state_t *ptr_ts)
{
    printf("\n[Группа] Нагрузочный тест\n");

    const size_t count = 10000;
    queue_t     *ptr_q = queue_create(16);

    TEST_CHECK(ptr_q != NULL, "нагрузка: очередь инициализирована", ptr_ts);

    // Массовое заполнение
    bool push_ok = true;
    for (size_t i = 0; i < count; i++) {
        if (!queue_push(ptr_q, (int)i)) {
            push_ok = false;
            break;
        }
    }
    TEST_CHECK(push_ok, "нагрузка: успешно добавлено 10000 элементов", ptr_ts);
    TEST_CHECK(queue_get_size(ptr_q) == count, "нагрузка: актуальный размер равен 10000", ptr_ts);

    // Массовое чтение с валидацией порядка
    bool pop_ok  = true;
    int  out_val = 0;
    for (size_t i = 0; i < count; i++) {
        if (!queue_pop(ptr_q, &out_val) || out_val != (int)i) {
            pop_ok = false;
            break;
        }
    }
    TEST_CHECK(pop_ok, "нагрузка: все элементы извлечены с сохранением порядка FIFO", ptr_ts);
    TEST_CHECK(queue_is_empty(ptr_q), "нагрузка: после вычитки буфер пуст", ptr_ts);

    queue_destroy(ptr_q);
}

int main(void)
{
    test_state_t ts = {0, 0};

    test_queue_create_destroy(&ts);
    test_queue_state_checks(&ts);
    test_queue_push_pop_peek(&ts);
    test_queue_dynamic_resize(&ts);
    test_ring_buffer_concurrency(&ts);
    test_large_queue(&ts);

    printf("\nИтого: %zu прошло, %zu упало\n", ts.passed, ts.failed);

    return ts.failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
