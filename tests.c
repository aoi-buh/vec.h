#include <assert.h>
#include <time.h>
#include <limits.h>

#include "vec.h"


#define arr_len(arr) (sizeof(arr) / sizeof(*(arr)))
#define is_unsigned(var) ((var) >= 0 && ~(var) >= 0)
#define fill_random(arr) do {                           \
        srand(time(NULL));                              \
        typeof(*(arr)) _tmp;                            \
        if (is_unsigned(_tmp))                          \
            for (size_t i = 0; i < arr_len(arr); i++)   \
                (arr)[i] = rand();                      \
        else                                            \
            for (size_t i = 0; i < arr_len(arr); i++)   \
                (arr)[i] = rand() - (RAND_MAX / 2);     \
    } while (0)
#define assert_same_type(a, b)                                          \
    ((void)sizeof(char[__builtin_types_compatible_p(typeof(a), typeof(b)) ? 1 : -1]))
#define debug(...) fprintf(stderr, __VA_ARGS__)
#define fori(i, len) for (size_t i = 0; i < len; i++)


int main() {
    // the type of vec can be changed to test other types \
    (the sorting tests are only setup to work with type that can be directly compared with operators)
    int *vec;
    typeof(vec) _vec;
    typeof(*vec) arr[1000];
    typeof(arr) _arr;

    vec = vec_new();
    assert("vec_new() test" &&
           vec_err(vec) == 0
           & vec_len(vec) == 0
           & vec_cap(vec) == 0);
    vec_free(vec);

    vec = vec_new(typeof(*vec));
    assert("vec_new() test" &&
           vec_err(vec) == 0
           & vec_len(vec) == 0
           & vec_cap(vec) == VEC_INIT_CAP);
    vec_free(vec);

    vec = vec_new(typeof(*vec), 100);
    assert("vec_cnew() test" &&
           vec_err(vec) == 0
           & vec_len(vec) == 0
           & vec_cap(vec) == 100);
    vec_free(vec);

    fill_random(arr);
    vec = vec_anew(arr);
    assert("vec_anew() test" &&
           vec_err(vec) == 0
           & vec_len(vec) == arr_len(arr)
           & vec_cap(vec) == arr_len(arr));
    fori (i, arr_len(arr))
        assert("vec_anew() element test" &&
               vec[i] == arr[i]);

    {
#define is_long(x) _Generic((x), long: 1, default: 0)
        intptr_t _vec = (intptr_t) vec;
        assert("vec_as() test" &&
               is_long(*vec_as(vec, long)));
#undef is_long
    }

    _vec = vec_clone(vec);
    assert("vec_clone() test" &&
           vec_err(_vec) == 0
           & vec_len(_vec) == vec_len(vec)
           & vec_cap(_vec) == vec_cap(vec));
    fori (i, vec_len(vec))
        assert("vec_clone() element test" &&
               _vec[i] == vec[i]);
    vec_free(vec);
    vec_free(_vec);

    vec = vec_zero(typeof(*vec), 1000);
    assert("vec_zero() test" &&
           vec_err(vec) == 0
           & vec_len(vec) == 1000
           & vec_cap(vec) == 1000);
    fori (i, vec_len(vec))
        assert("vec_zero() element test" &&
               vec[i] == 0);
    vec_free(vec);

    vec = vec_new();
    vec_reserve_exact(vec, 100);
    assert("vec_reserve_exact() test" &&
           vec_err(vec) == 0
           & vec_len(vec) == 0
           & vec_cap(vec) == 100);
    vec_reserve(vec, 150);
    assert("vec_reserve() test" &&
           vec_err(vec) == 0
           & vec_len(vec) == 0
           & vec_cap(vec) == 400);

    vec_shrink_to(vec, 100);
    assert("vec_shrink_to() test" &&
           vec_err(vec) == 0
           & vec_len(vec) == 0
           & vec_cap(vec) == 100);
    vec_shrink_to_fit(vec);
    assert("vec_shrink_to_fit() test" &&
           vec_err(vec) == 0
           & vec_len(vec) == 0
           & vec_cap(vec) == 0);
    vec_free(vec);

    fill_random(arr);
    vec = vec_anew(arr);
    vec_truncate(vec, arr_len(arr) - 10);
    assert("vec_truncate() test" &&
           vec_err(vec) == 0
           & vec_len(vec) == arr_len(arr) - 10
           & vec_cap(vec) == arr_len(arr));
    vec_truncate(vec, arr_len(arr));
    assert("vec_truncate() negative test" &&
           vec_err(vec) == 0
           & vec_len(vec) == arr_len(arr) - 10
           & vec_cap(vec) == arr_len(arr));
    vec_free(vec);

    fill_random(arr);
    vec = vec_anew(arr);
    fori (i, vec_len(vec))
        assert("vec_get() positive index test" &&
               vec_get(vec, i) == arr[i]);
    for (size_t i = 1; i < arr_len(arr)-1; i++)
        assert("vec_get() negative index test" &&
               vec_get(vec, -i) == arr[arr_len(arr) - i]
               & vec_err(vec) == NONE);
    assert("vec_get() negative test" &&
           vec_get(vec, -arr_len(arr) - 1) == 0
           & vec_err(vec) == VEC_INDEX_OUT_OF_BOUNDS
           & vec_get(vec, arr_len(arr)) == 0
           & vec_err(vec) == VEC_INDEX_OUT_OF_BOUNDS);
    vec_free(vec);

    fill_random(arr);
    vec = vec_anew(arr);
    vec_reverse(vec);
    for (size_t i = 0, j = arr_len(arr)-1; i < arr_len(arr) & j >= 0; i++, j--)
        assert("vec_reverse() test" &&
               vec[j] == arr[i]);
    vec_free(vec);

    fill_random(arr);
    vec = vec_anew(arr);
    for (size_t i = 0, j = 1000; i < j; i++) {
        size_t i = rand();
        if (!_VEC_VALID_INDEX(vec, i)) continue;
        assert("vec_swap_remove() test" &&
               vec[i] == vec_swap_remove(vec, i)
               & vec[i] == arr[arr_len(arr)-1]);
    }
    assert("vec_swap_remove() negative test" &&
           vec_swap_remove(vec, vec_len(vec)+1+rand()) == 0
           & vec_err(vec) == VEC_INDEX_OUT_OF_BOUNDS);
    vec_free(vec);
    vec = vec_new();
    assert("vec_swap_remove() negative test" &&
           vec_swap_remove(vec, rand()) == 0
           & vec_err(vec) == VEC_INDEX_OUT_OF_BOUNDS);
    vec_free(vec);

    fori (i, arr_len(arr))
        arr[i] = i;
    vec = vec_anew(arr);
#define test_fn(i) (i % 2)
    vec_retain(vec, test_fn);
    for (size_t i = 0, j = 1; j < arr_len(arr); i++, j+=2)
        assert("vec_retain() test" &&
               vec[i] == j
               & vec_len(vec) == arr_len(arr)/2);
    vec_free(vec);

    vec = vec_new();
    fill_random(arr);
    fori (i, arr_len(arr)) {
        vec_push(vec, arr[i]);
        assert("vec_push() test" &&
               vec_err(vec) == 0
               & vec_len(vec) == i + 1
               & vec_cap(vec) >= vec_len(vec)
               & vec[vec_len(vec)-1] == arr[i]);
    }
    vec_free(vec);

    fill_random(arr);
    vec = vec_anew(arr);
    for (size_t i = arr_len(arr); i--;)
        assert("vec_pop() test" &&
               vec_pop(vec) == arr[i]
               & vec_err(vec) == 0
               & vec_len(vec) == i
               & vec_cap(vec) == arr_len(arr));
    assert("vec_pop() negative test" &&
           vec_pop(vec) == 0
           & vec_err(vec) == VEC_NO_ITEM_TO_POP
           & vec_len(vec) == 0
           & vec_cap(vec) == arr_len(arr));
    vec_free(vec);

    fill_random(arr);
    vec = vec_mnew(arr, arr_len(arr)/2);
    _vec = vec_mnew(arr+arr_len(arr)/2, arr_len(arr)/2);
    assert("vec_apppend() test" &&
           vec_append(vec, _vec)
           & vec_err(vec) == 0
           & vec_len(vec) == arr_len(arr)/2*2 // /2*2 so it works for odd len arr
           & vec_err(_vec) == 0
           & vec_len(_vec) == 0
           & memcmp(vec, arr, sizeof(*vec) * vec_len(vec)) == 0);
    vec_free(vec);
    vec_free(_vec);

    fill_random(arr);
    vec = vec_anew(arr);
    {
        size_t a = rand() % vec_len(vec);
        int b = rand();
        vec_insert(vec, a, b);
        assert("vec_insert() test" &&
               vec_err(vec) == 0
               & vec_len(vec) == arr_len(arr)+1
               & memcmp(vec, arr, sizeof(*vec) * a) == 0
               & vec[a] == b
               & memcmp(vec+a+1, arr+a, sizeof(*vec) * (vec_len(vec)-a-1)) == 0);
    }
    vec_free(vec);

    fill_random(arr);
    vec = vec_anew(arr);
    fill_random(_arr);
    {
        size_t a = rand() % vec_len(vec);
        vec_ainsert(vec, a, _arr);
        assert("vec_ainsert() test" &&
               vec_err(vec) == 0
               & vec_len(vec) == arr_len(arr) + arr_len(_arr)
               & memcmp(vec, arr, sizeof(*vec) * a) == 0
               & memcmp(vec+a, _arr, sizeof(*vec) * arr_len(_arr)) == 0
               & memcmp(vec+a+arr_len(_arr), arr+a, sizeof(*vec) * (vec_len(vec)-a-arr_len(_arr))) == 0);
    }
    vec_free(vec);

    fill_random(arr);
    vec = vec_anew(arr);
    fill_random(_arr);
    _vec = vec_anew(arr);
    {
        size_t a = rand() % vec_len(vec);
        vec_vinsert(vec, a, _vec);
        assert("vec_vinsert() test" &&
               vec_err(vec) == 0
               & vec_len(vec) == arr_len(arr) + arr_len(_arr)
               & memcmp(vec, arr, sizeof(*vec) * a) == 0
               & memcmp(vec+a, _arr, sizeof(*vec) * arr_len(_arr)) == 0
               & memcmp(vec+a+arr_len(_arr), arr+a, sizeof(*vec) * (vec_len(vec)-a-arr_len(_arr))) == 0);
    }
    vec_free(vec);
    vec_free(_vec);

    fill_random(arr);
    vec = vec_anew(arr);
    _vec = vec_reversed(vec);
    for (size_t i = 0, j = arr_len(arr)-1; i < arr_len(arr) & j >= 0; i++, j--)
        assert("vec_reverse() test" &&
               vec[j] == _vec[i]);
    vec_free(vec);

    fill_random(arr);
    vec = vec_anew(arr);
    {
#undef test_fn
#define test_fn(i) ((long)(i*2))
        long *_vec = vec_map(vec, test_fn);
        assert("vec_map() test" &&
               vec_err(vec) == 0
               & vec_err(_vec) == 0);
        fori (i, vec_len(vec))
            assert("vec_map() element test" &&
                   _vec[i] == test_fn(vec[i]));
        vec_free(_vec);
    }
    vec_free(vec);

    fill_random(arr);
    vec = vec_anew(arr);
#undef test_fn
#define test_fn(i) (i%2 == 0)
    _vec = vec_filter(vec, test_fn);
    assert("vec_filter() test" &&
           vec_err(vec) == 0
           & vec_err(_vec) == 0
           & vec_len(_vec) <= vec_len(vec)
           & vec_cap(_vec) == vec_len(vec));
    for (size_t i = 0, j = 0; i < vec_len(vec); i++)
        if (test_fn(vec[i]))
            assert("vec_filter() element test" &&
                   _vec[j++] == vec[i]);
    vec_free(vec);
    vec_free(_vec);

    fill_random(arr);
    fori (i, arr_len(arr))
        arr[i] = arr[i] % 10;
    vec = vec_anew(arr);
#undef test_fn
#define test_fn(a, i) (a + i)
    {
        typeof(0) expect = 0;
        fori (i, arr_len(arr))
            expect += arr[i];
        assert("vec_reduce() test" &&
               vec_err(vec) == 0
               & vec_reduce(vec, test_fn, 0) == expect);
    }
    vec_free(vec);

    {
        char arr[CHAR_MAX];
        fori (i, arr_len(arr)) arr[i] = i;
        char *vec = vec_anew(arr);
        FILE *f = tmpfile();
#undef test_fn
#define test_fn(i) fputc(i, f)
        vec_foreach(vec, test_fn);
        rewind(f);
        fori (i, vec_len(vec))
            assert("vec_foreach() test" &&
                   fgetc(f) == vec[i]);
        fclose(f);
        vec_free(vec);
    }

    {
        typeof(*arr) _arr[] = { 1, 3, 4, 5, 6 };
        vec = vec_anew(_arr);
#undef test_fn
#define test_fn(i) (i % 2 == 0)
        assert("vec_find() test" &&
               vec_find(vec, test_fn) == 2
               & vec_err(vec) == 0);
#undef test_fn
#define test_fn(i) (i == 10)
        assert("vec_find() negative test" &&
               vec_find(vec, test_fn) == SIZE_MAX
               & vec_err(vec) == VEC_TARGET_NOT_FOUND);
        vec_free(vec);
    }

    {
        typeof(*arr) _arr[] = { 1, 2, 3, 4, 5 };
        vec = vec_anew(_arr);
#undef test_fn
#define test_fn(i) (i % 2 == 0)
        assert("vec_any() test" &&
               vec_any(vec, test_fn)
               & vec_err(vec) == 0);
#undef test_fn
#define test_fn(i) (i == 0)
        assert("vec_any() negative test" &&
               vec_any(vec, test_fn) == false
               & vec_err(vec) == 0);

#undef test_fn
#define test_fn(i) (i)
        assert("vec_all() test" &&
               vec_all(vec, test_fn)
               & vec_err(vec) == 0);
#undef test_fn
#define test_fn(i) (i % 2)
        assert("vec_all() test" &&
               vec_all(vec, test_fn) == false
               & vec_err(vec) == 0);
        vec_free(vec);
    }

    static_assert(arr_len(arr) >= 10);
    fori (i, arr_len(arr)) arr[i] = i;
    vec = vec_anew(arr);
    {
#undef test_fn
#define test_fn(i) (i < 5)
        assert("vec_take_while() test" &&
               (_vec = vec_take_while(vec, test_fn)) != NULL
               & vec_err(_vec) == 0
               & vec_len(_vec) == 5
               & memcmp(vec, _vec, sizeof(*vec) * vec_len(_vec)) == 0);
        vec_free(_vec);

        assert("vec_drop_while() test" &&
               (_vec = vec_drop_while(vec, test_fn)) != NULL
               & vec_err(_vec) == 0
               & vec_len(_vec) == arr_len(arr) - 5
               & memcmp(vec+5, _vec, sizeof(*vec) * vec_len(_vec)) == 0);
    }
    vec_free(vec);

    static_assert(arr_len(arr) % 2 == 0);
    fori (i, arr_len(arr)) arr[i] = i;
    vec = vec_anew(arr);
    {
#undef test_fn
#define test_fn(i) (i % 2 == 0)
        typeof(vec) *_vec = vec_partition(vec, test_fn);
        assert("vec_partition() test" &&
               vec_err(vec) == 0
               & vec_len(_vec[0]) + vec_len(_vec[1]) == vec_len(vec)
               & !vec_any(_vec[0], test_fn)
               & vec_all(_vec[1], test_fn));
        vec_free_items(_vec);
        vec_free(_vec);
    }
    vec_free(vec);

    fori (i, arr_len(arr)) arr[i] = i+1;
    vec = vec_anew(arr);
    {
        size_t a = rand() % vec_len(vec);
        assert("vec_search() test" &&
               vec_search(vec, a) == a-1
               & vec_err(vec) == 0);
        assert("vec_search() negative test" &&
               vec_search(vec, vec_len(vec) + 1) == SIZE_MAX
               & vec_err(vec) == VEC_TARGET_NOT_FOUND);
        int arr[10 * 100];
        vec_free(vec);
        for (size_t i = 0; i < 100; i++)
            for (size_t j = 0; j < 10; j++)
                arr[i*10+j] = i;
        vec = vec_anew(arr);
        a = rand() % 10;
        assert("vec_search() duplicates test" &&
               vec_search(vec, a) == a*10
               & vec_err(vec) == 0);
        assert("vec_search() 0 index duplicates test" &&
               vec_search(vec, 0) == 0
               & vec_err(vec) == 0);
    }
    vec_free(vec);

    for (size_t i = 0, j = arr_len(arr)-1; i < arr_len(arr); i++, j--)
        arr[i] = j;
    vec = vec_anew(arr);
    {
        int a = rand() % vec_len(vec);
#undef test_fn
#define test_fn(i) (i > a? -1 : (i < a? 1: 0))
#define test_fn2(i) (!test_fn(i))
        assert("vec_search_by() test" &&
               vec_search_by(vec, test_fn) == vec_find(vec, test_fn2)
               & vec_err(vec) == 0);
        a = -1;
        assert("vec_search_by() negative test" &&
               vec_search_by(vec, test_fn) == SIZE_MAX
               & vec_err(vec) == VEC_TARGET_NOT_FOUND);

#undef test_fn
#define test_fn(i) (-(i))
#undef test_fn2
#define test_fn2(i) (-(i) == a)
        a = -(rand() % vec_len(vec));
        vec_err(vec) = 0;
        assert("vec_search_by_key() test" &&
               vec_search_by_key(vec, a, test_fn) == vec_find(vec, test_fn2)
               & vec_err(vec) == 0);
        a = rand();
        vec_err(vec) = 0;
        assert("vec_search_by_key() negative test" &&
               vec_search_by_key(vec, a, test_fn) == SIZE_MAX
               & vec_err(vec) == VEC_TARGET_NOT_FOUND);
    }
    vec_free(vec);

    fill_random(arr);
    vec = vec_anew(arr);
    {
        typedef struct {
            bool sorted;
            typeof(*vec) item;
        } is_sorted_acc;
        vec_reverse(vec);
#undef test_fn
#define test_fn(a, i) ((a.sorted = a.sorted && (a.item <= i)), a)
        assert("vec_sort() test" &&
               vec_sort(vec)
               & vec_reduce(vec, test_fn, ((is_sorted_acc) { true, vec[0] })).sorted
               & vec_err(vec) == 0);

#undef test_fn
#define test_fn(a, i) ((a.sorted = a.sorted && (a.item >= i)), a)
        bool cmp_fn(typeof(*vec) i, typeof(*vec) j) {
            return i >= j;
        }
        assert("vec_sort_by() test" &&
               vec_sort_by(vec, cmp_fn)
               & vec_reduce(vec, test_fn, ((is_sorted_acc) { true, vec[0] })).sorted
               & vec_err(vec) == 0);

#undef test_fn
#define test_fn(a, i) ((a.sorted = a.sorted && (a.item <= i)), a)
        typeof(*vec) key_fn(typeof(*vec) i) {
            return i + 1;
        }
        assert("vec_sort_by_key() test" &&
               vec_sort_by_key(vec, key_fn)
               & vec_reduce(vec, test_fn, ((is_sorted_acc) { true, vec[0] })).sorted
               & vec_err(vec) == 0);
    }
    vec_free(_vec);
    vec_free(vec);

    fill_random(arr);
    vec = vec_anew(arr);
    {
        typedef struct {
            bool sorted;
            typeof(*vec) item;
        } is_sorted_acc;
        vec_reverse(vec);
#undef test_fn
#define test_fn(a, i) ((a.sorted = a.sorted && (a.item <= i)), a)
        assert("vec_sort_unstable() test" &&
               vec_sort_unstable(vec)
               & vec_reduce(vec, test_fn, ((is_sorted_acc) { true, vec[0] })).sorted
               & vec_err(vec) == 0);

#undef test_fn
#define test_fn(a, i) ((a.sorted = a.sorted && (a.item >= i)), a)
#undef test_fn2
#define test_fn2(i, j) (i >= j)
        assert("vec_sort_unstable_by() test" &&
               vec_sort_unstable_by(vec, test_fn2)
               & vec_reduce(vec, test_fn, ((is_sorted_acc) { true, vec[0] })).sorted
               & vec_err(vec) == 0);

#undef test_fn
#define test_fn(a, i) ((a.sorted = a.sorted && (a.item <= i)), a)
#undef test_fn2
#define test_fn2(i) (i+1)
        assert("vec_sort_unstable_by_key() test" &&
               vec_sort_unstable_by_key(vec, test_fn2)
               & vec_reduce(vec, test_fn, ((is_sorted_acc) { true, vec[0] })).sorted
               & vec_err(vec) == 0);
    }
    vec_free(vec);

    printf("\x1b[32m" "all tests successful\n");
}
