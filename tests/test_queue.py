#!/usr/bin/env python3

import ctypes as _ctypes
from pathlib import Path
import sys as _sys

# Загрузка библиотеки

_CURRENT_DIR = Path(__file__).resolve().parent

_LIB_PATHS = [
    _CURRENT_DIR / ".." / "build" / "libqueue.so",
    _CURRENT_DIR / "libqueue.so",
]

_lib = None
for _path in _LIB_PATHS:
    if _path.exists():
        _lib = _ctypes.CDLL(str(_path))
        break

if _lib is None:
    print("Ошибка: не удалось найти libqueue.so", file=_sys.stderr)
    print("Выполните сборку shared-библиотеки очереди", file=_sys.stderr)
    _sys.exit(1)


# Структура queue_t


class QueueT(_ctypes.Structure):
    """Отображение структуры queue_t из C."""

    _fields_ = [
        ("data", _ctypes.POINTER(_ctypes.c_int)),
        ("head", _ctypes.c_size_t),
        ("tail", _ctypes.c_size_t),
        ("size", _ctypes.c_size_t),
        ("capacity", _ctypes.c_size_t),
    ]


QueuePtr = _ctypes.POINTER(QueueT)


# Прототипы функций

_lib.queue_create.argtypes = [_ctypes.c_size_t]
_lib.queue_create.restype = QueuePtr

_lib.queue_destroy.argtypes = [QueuePtr]
_lib.queue_destroy.restype = None

_lib.queue_is_empty.argtypes = [QueuePtr]
_lib.queue_is_empty.restype = _ctypes.c_bool

_lib.queue_is_full.argtypes = [QueuePtr]
_lib.queue_is_full.restype = _ctypes.c_bool

_lib.queue_push.argtypes = [QueuePtr, _ctypes.c_int]
_lib.queue_push.restype = _ctypes.c_bool

_lib.queue_pop.argtypes = [QueuePtr, _ctypes.POINTER(_ctypes.c_int)]
_lib.queue_pop.restype = _ctypes.c_bool

_lib.queue_peek.argtypes = [QueuePtr, _ctypes.POINTER(_ctypes.c_int)]
_lib.queue_peek.restype = _ctypes.c_bool

_lib.queue_get_size.argtypes = [QueuePtr]
_lib.queue_get_size.restype = _ctypes.c_size_t


# Класс накопления результатов


class TestState:
    """Простой счётчик прошедших и упавших тестов."""

    def __init__(self):
        self.passed = 0
        self.failed = 0

    def check(self, condition: bool, message: str) -> None:
        if condition:
            print(f"[PASS] {message}")
            self.passed += 1
        else:
            print(f"[FAIL] {message}")
            self.failed += 1


# Тестовые группы


def test_queue_create_destroy(ts: TestState) -> None:
    print("\n[Группа] queue_create / queue_destroy")

    # Типовой: создание корректной очереди
    q = _lib.queue_create(4)
    ts.check(bool(q), "создание: указатель не NULL")
    if q:
        ts.check(q.contents.capacity == 4, "создание: capacity == 4")
        ts.check(q.contents.size == 0, "создание: size == 0")
        ts.check(q.contents.head == 0, "создание: head == 0")
        ts.check(q.contents.tail == 0, "создание: tail == 0")
        _lib.queue_destroy(q)

    # Граничный: initial_capacity == 0
    q_zero = _lib.queue_create(0)
    ts.check(not bool(q_zero), "граничный: capacity == 0 возвращает NULL")

    # Ошибочный: destroy NULL не должен падать
    try:
        _lib.queue_destroy(None)
        ts.check(True, "безопасность: уничтожение NULL-указателя")
    except Exception as e:
        ts.check(False, f"безопасность: уничтожение NULL упало с {e}")


def test_queue_state_checks(ts: TestState) -> None:
    print("\n[Группа] проверки состояния (empty/full/size)")

    # Проверки на NULL
    ts.check(_lib.queue_is_empty(None) == True, "NULL очередь: empty == True")
    ts.check(_lib.queue_is_full(None) == False, "NULL очередь: full == False")
    ts.check(_lib.queue_get_size(None) == 0, "NULL очередь: size == 0")

    # Проверки на живой очереди
    q = _lib.queue_create(2)
    ts.check(_lib.queue_is_empty(q) == True, "новая очередь: пустая")
    ts.check(_lib.queue_is_full(q) == False, "новая очередь: не полная")
    ts.check(_lib.queue_get_size(q) == 0, "новая очередь: размер 0")

    # Заполняем до отказа
    _lib.queue_push(q, 10)
    _lib.queue_push(q, 20)
    ts.check(_lib.queue_is_empty(q) == False, "заполненная: не пустая")
    ts.check(_lib.queue_is_full(q) == True, "заполненная: полная")
    ts.check(_lib.queue_get_size(q) == 2, "заполненная: размер == 2")

    _lib.queue_destroy(q)


def test_queue_push_pop_peek(ts: TestState) -> None:
    print("\n[Группа] push / pop / peek")

    q = _lib.queue_create(3)
    out_val = _ctypes.c_int(0)

    # Ошибочные операции с NULL
    ts.check(_lib.queue_push(None, 42) == False, "push в NULL -> False")
    ts.check(_lib.queue_pop(None, _ctypes.byref(out_val)) == False, "pop из NULL -> False")
    ts.check(_lib.queue_pop(q, None) == False, "pop в NULL-вывод -> False")
    ts.check(_lib.queue_peek(q, _ctypes.byref(out_val)) == False, "peek из пустой -> False")

    # Типовой цикл
    ts.check(_lib.queue_push(q, 100) == True, "push 100: OK")
    ts.check(_lib.queue_push(q, 200) == True, "push 200: OK")

    # Проверка peek
    ts.check(_lib.queue_peek(q, _ctypes.byref(out_val)) == True, "peek: OK")
    ts.check(out_val.value == 100, "peek: значение == 100")
    ts.check(_lib.queue_get_size(q) == 2, "peek не удалил элемент")

    # Извлечение (pop)
    ts.check(_lib.queue_pop(q, _ctypes.byref(out_val)) == True, "pop 1: OK")
    ts.check(out_val.value == 100, "pop 1: значение == 100")

    ts.check(_lib.queue_pop(q, _ctypes.byref(out_val)) == True, "pop 2: OK")
    ts.check(out_val.value == 200, "pop 2: значение == 200")

    ts.check(_lib.queue_is_empty(q) == True, "очередь снова пуста")
    ts.check(_lib.queue_pop(q, _ctypes.byref(out_val)) == False, "pop из пустой -> False")

    _lib.queue_destroy(q)


def test_queue_dynamic_resize(ts: TestState) -> None:
    print("\n[Группа] динамическое изменение размера (grow/shrink)")

    # Создаем очередь минимальной емкости
    q = _lib.queue_create(2)

    # Проверяем расширение (GROW): push больше capacity
    _lib.queue_push(q, 1)
    _lib.queue_push(q, 2)
    ts.check(q.contents.capacity == 2, "перед расширением: capacity == 2")
    ts.check(_lib.queue_is_full(q) == True, "очередь полна")

    # Этот push должен триггернуть queue_resize (2 * 2 = 4)
    ts.check(_lib.queue_push(q, 3) == True, "push 3 (триггер grow): OK")
    ts.check(q.contents.capacity == 4, "после расширения: capacity удваивается (== 4)")
    ts.check(q.contents.size == 3, "размер корректный (== 3)")

    # Добиваем до 5 элементов, чтобы емкость выросла до 8
    _lib.queue_push(q, 4)
    _lib.queue_push(q, 5)  # Тут capacity станет 8
    ts.check(q.contents.capacity == 8, "после второго grow: capacity == 8")

    # Проверяем сжатие (SHRINK)
    # По логике: сжатие происходит, если capacity > 4 AND size <= capacity / 4
    # При capacity=8 и size=5, вытаскиваем элементы:
    out = _ctypes.c_int(0)

    _lib.queue_pop(q, _ctypes.byref(out))  # size=4, cap=8 (4 > 8/4)
    _lib.queue_pop(q, _ctypes.byref(out))  # size=3, cap=8 (3 > 8/4)

    # Сейчас size = 3. Следующий pop сделает size = 2.
    # Условие сжатия: (8 > 4) && (2 > 0) && (2 <= 8 / 4). Условие 2 <= 2 выполняется!
    ts.check(_lib.queue_pop(q, _ctypes.byref(out)) == True, "pop 3 (триггер shrink): OK")
    ts.check(q.contents.capacity == 4, "после сжатия: capacity уменьшилась вдвое (== 4)")

    _lib.queue_destroy(q)


def test_ring_buffer_concurrency(ts: TestState) -> None:
    print("\n[Группа] работа кольцевого буфера (перемотка индексов)")

    # Проверим, что head и tail корректно крутятся по кольцу без изменения размера
    q = _lib.queue_create(3)
    out = _ctypes.c_int(0)

    # Заполняем 2 элемента из 3
    _lib.queue_push(q, 10)
    _lib.queue_push(q, 20)

    # Выбиваем один, сдвигая head
    _lib.queue_pop(q, _ctypes.byref(out))  # head=1, tail=2, size=1

    # Добавляем еще два, заставляя tail зациклиться (tail = (2+1)%3 = 0, затем (0+1)%3 = 1)
    _lib.queue_push(q, 30)
    _lib.queue_push(q, 40)  # Буфер забит: head=1, tail=1, size=3, capacity=3

    ts.check(q.contents.head == 1, "кольцо: head сместился")
    ts.check(q.contents.tail == 1, "кольцо: tail завернулся по модулю")
    ts.check(_lib.queue_is_full(q) == True, "кольцо: буфер полон")

    # Вытаскиваем всё и сверяем FIFO порядок
    _lib.queue_pop(q, _ctypes.byref(out))
    ts.check(out.value == 20, "FIFO по кольцу: получили 20")
    _lib.queue_pop(q, _ctypes.byref(out))
    ts.check(out.value == 30, "FIFO по кольцу: получили 30")
    _lib.queue_pop(q, _ctypes.byref(out))
    ts.check(out.value == 40, "FIFO по кольцу: получили 40")

    _lib.queue_destroy(q)


def test_large_queue(ts: TestState) -> None:
    print("\n[Группа] Нагрузочный тест")
    count = 5000

    q = _lib.queue_create(10)
    ts.check(bool(q), "нагрузка: очередь создана")

    # Массовый пуш
    push_success = True
    for i in range(count):
        if not _lib.queue_push(q, i):
            push_success = False
            break
    ts.check(push_success, f"нагрузка: успешно запушено {count} элементов")
    ts.check(_lib.queue_get_size(q) == count, f"нагрузка: размер равен {count}")

    # Массовый поп с валидацией данных
    pop_success = True
    out = _ctypes.c_int(0)
    for i in range(count):
        if not _lib.queue_pop(q, _ctypes.byref(out)) or out.value != i:
            pop_success = False
            break
    ts.check(pop_success, "нагрузка: все элементы извлечены в строгом FIFO порядке")
    ts.check(_lib.queue_is_empty(q) == True, "нагрузка: после очистки очередь пуста")

    _lib.queue_destroy(q)


# Точка входа

if __name__ == "__main__":
    ts = TestState()

    test_queue_create_destroy(ts)
    test_queue_state_checks(ts)
    test_queue_push_pop_peek(ts)
    test_queue_dynamic_resize(ts)
    test_ring_buffer_concurrency(ts)
    test_large_queue(ts)

    print(f"\nИтого: {ts.passed} прошло, {ts.failed} упало")

    _sys.exit(0 if ts.failed == 0 else 1)
