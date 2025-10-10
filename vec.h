#ifndef VEC_H
#ifndef __GNUC__
#    error "this header file is made for gnuc in mind only"
#endif
#define VEC_H

#define _VEC_C95 199409L
#define _VEC_C99 199901L
#define _VEC_C11 201112L
#define _VEC_C17 201710L
#define _VEC_C23 202311L

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

/*
  type definitions
*/
enum vec_err {
    NONE,
    VEC_ALLOC_ERR,
    VEC_INDEX_OUT_OF_BOUNDS,
    VEC_RESIZE_ERR,
    VEC_NO_ITEM_TO_POP,
    VEC_TARGET_NOT_FOUND,
};

typedef struct {
    uint8_t err;
    size_t len;
    size_t cap;
    unsigned char items[];
} vec_t;


/*
  configurable constants
*/
#ifndef VEC_CONFIG_NO_BOUNDS_CHECKING
#    define _VEC_ASSERT _Static_assert
#else
#    define _VEC_ASSERT(...)
#endif

#ifndef VEC_INIT_CAP
#    define VEC_INIT_CAP 16
#endif
_VEC_ASSERT(VEC_INIT_CAP > 0 & VEC_INIT_CAP == (int)VEC_INIT_CAP,
            "VEC_INIT_CAP must be a positive integer");
#ifndef VEC_EXPAND_FACTOR
#    define VEC_EXPAND_FACTOR 2
#endif
_VEC_ASSERT(VEC_EXPAND_FACTOR > 1,
            "VEC_EXPAND_FACTOR must be bigger than 1");

/*
  configurable variables
*/
#define VEC_MALLOC malloc
#define VEC_REALLOC realloc


/*
  error messages
*/
#define VEC_ALLOC_ERR_MSG "failed to allocate for vector"


/*
  internal functions
*/
// non vec functions
#define _VEC_ARR_LEN(arr) (sizeof(arr)/sizeof(*(arr)))
#define _VEC_IF_REALLOC(cond1, cond2, expr) ((cond1)? ((cond2) && ((expr), true)) : ((expr), true))
#define _VEC_UNWRAP(x) x
#define _VEC_LE(a, b) ((a) <= (b))
#define _VEC_DEFAULT_CMP(a, b) ((a) - (b))
#define _VEC_SWAP(a, b) ({ __auto_type tmp = a; a = b; b = tmp; })
#define _VEC_SWAP_WITH_KEY(arr, keys, i, j)     \
    ({  typeof(*arr) tmp1 = (arr)[i];           \
        typeof(*keys) tmp2 = (keys)[i];         \
        (arr)[i] = (arr)[j];                    \
        (keys)[i] = (keys)[j];                  \
        (arr)[j] = tmp1;                        \
        (keys)[j] = tmp1;  })

// vec functions
#define _VEC_HEADER(self) (((vec_t *) (self))-1)
#define _VEC_VALID_INDEX(self, i) ((i >= 0) & (i < vec_len(self)))


/*
  vector type macro
*/
#define vec(T) ((T)*)


/*
  external functions
*/
#define vec_err(self) (_VEC_HEADER(self)->err)
#define vec_len(self) (_VEC_HEADER(self)->len)
#define vec_cap(self) (_VEC_HEADER(self)->cap)
#define vec_is_empty(self) (vec_len(self) == 0)
#define vec_free(self) free(_VEC_HEADER(self))
#define vec_free_items(self) vec_foreach((self), vec_free)
#define vec_maxed(self) (vec_cap(self) == SIZE_MAX)
#define vec_as(self, type) ({  type *tmp = (type*) self; tmp;  })
#define vec_debug(self)                                             \
    fprintf(stderr,                                                 \
            "vec: %lu\n"                                            \
            "err: %d\n"                                             \
            "len: %lu\n"                                            \
            "cap: %lu\n",                                           \
            (self), vec_err(self), vec_len(self), vec_cap(self))

#define _VEC_NEW(_0, _1, _2, fn, ...) fn
#define vec_new(...) _VEC_NEW(_0, ##__VA_ARGS__,                        \
                              vec_with_capacity(__VA_ARGS__),           \
                              vec_with_capacity(__VA_ARGS__, VEC_INIT_CAP), \
                              vec_with_capacity(struct {}, 0))
#define vec_cnew vec_with_capacity
#define vec_with_capacity(T, _cap)                                      \
    ({  vec_t *_vec = VEC_MALLOC(sizeof(vec_t) + sizeof(T) * (_cap));   \
        _vec                                                            \
            ? (memset(_vec, 0, sizeof(vec_t)),                          \
               _vec->cap = _cap,                                        \
               &_vec->items)                                            \
            : (perror(VEC_ALLOC_ERR_MSG), NULL); })
#define vec_gnew vec_generic_clone
#define vec_gclone vec_generic_clone
#define vec_generic_clone(arr, _len)                                    \
    ({  typeof(*(arr)) *_vec = vec_with_capacity(typeof(*(arr)), (_len)); \
        _vec                                                            \
            ? (memcpy(_vec, (arr), sizeof(*(arr)) * (_len)),            \
               vec_len(_vec) = (_len),                                  \
               _vec)                                                    \
            : NULL;  })
#define vec_anew vec_array_new
#define vec_array_new vec_from_array
#define vec_from_array(arr) vec_generic_clone((arr), _VEC_ARR_LEN(arr))
#define vec_zero(T, _cap)                                               \
    ({  T *_vec = vec_with_capacity(T, (_cap));                         \
        _vec                                                            \
            ? (memset(_vec, 0, sizeof(T) * (_cap)), vec_len(_vec) = (_cap), _vec) \
            : (perror(VEC_ALLOC_ERR_MSG), NULL);  })
#define vec_clone(self)                                                 \
    ({  vec_t *_vec = VEC_MALLOC(sizeof(vec_t) + sizeof(*(self)) * vec_cap(self)); \
        _vec                                                            \
            ? (memcpy(_vec, _VEC_HEADER(self), sizeof(vec_t) + sizeof(*(self)) * vec_len(self)), \
               &_vec->items)                                            \
            : (vec_err(self) = VEC_ALLOC_ERR, perror(VEC_ALLOC_ERR_MSG), NULL); })

#define vec_reserve_exact(self, n)                                      \
    ({  size_t _n = (vec_cap(self) > SIZE_MAX - (n))                    \
            ? SIZE_MAX - vec_cap(self)                                  \
            : (n);                                                      \
        vec_t *_vec;                                                    \
        _n                                                              \
            ? (_vec = VEC_REALLOC(_VEC_HEADER(self),                    \
                                  sizeof(vec_t) + sizeof(*(self)) * (vec_cap(self) + _n)), \
               _vec                                                     \
               ? (_vec->cap += _n, ((self) = (typeof(self)) &_vec->items), true) \
               : (vec_err(self) = VEC_RESIZE_ERR, false))               \
            : true; })
#define vec_reserve(self, n)                                            \
    ({  size_t factor = VEC_EXPAND_FACTOR;                              \
        (vec_cap(self) == 0) && (vec_cap(self) = VEC_INIT_CAP);         \
        while (vec_cap(self) * factor < vec_cap(self) + (n)) factor *= factor; \
        vec_reserve_exact((self),                                       \
                          vec_cap(self) * (factor - 1));  })

#define vec_shrink_to(self, n)                                          \
    (vec_cap(self) > vec_len(self) + (n)                                \
     && ({   vec_t *_vec = VEC_REALLOC(_VEC_HEADER(self),               \
                 sizeof(vec_t) + sizeof(*(self)) * (vec_len(self) + (n))); \
             _vec                                                       \
                 ? (_vec->cap = vec_len(self) + (n), true)              \
                 : (vec_err(self) = VEC_RESIZE_ERR, false);  }))
#define vec_shrink_to_fit(self) vec_shrink_to((self), 0)

#define vec_truncate(self, n) (vec_len(self) >= (n) && (vec_len(self) = (n)))

#define vec_get(self, i)                                                \
    ((intmax_t)(i) < 0? _VEC_GET((self), vec_len(self)+(i)) : _VEC_GET((self), (i)))
#define _VEC_GET(self, i)                                               \
    (_VEC_VALID_INDEX((self), (i))                                      \
     ? ((self)+(i))                                                     \
     : (vec_err(self) = VEC_INDEX_OUT_OF_BOUNDS, (typeof(*(self))*) NULL))
/* ((_VEC_VALID_INDEX((self), (i)) || (vec_err(self) = VEC_INDEX_OUT_OF_BOUNDS)), (self)[i]) */
/* (_VEC_VALID_INDEX((self), (i))? (self)[i] : (vec_err(self) = VEC_INDEX_OUT_OF_BOUNDS, (self)[0])) */

#define vec_reverse(self)                                           \
    ({  typeof(*(self)) _item;                                      \
        for (size_t i = 0, j = vec_len(self)-1; i < j; i++, j--) {  \
            _item = (self)[i];                                      \
            (self)[i] = (self)[j];                                  \
            (self)[j] = _item;  }})

#define _VEC_REMOVE(self, i, body)                      \
    ((i) != vec_len(self)-1                             \
     ? (_VEC_VALID_INDEX((self), (i))                   \
        ? (body)                                        \
        : (vec_err(self) = VEC_INDEX_OUT_OF_BOUNDS), 0) \
     : (self)[--vec_len(self)])
#define vec_swap_remove(self, i)                            \
    _VEC_REMOVE((self), (i),                                \
                ({  typeof(*(self)) _item = (self)[i];      \
                    (self)[i] = (self)[--vec_len(self)];    \
                    _item;  }))
#define vec_remove(self, i)                                             \
    _VEC_REMOVE((self), (i),                                            \
                ({  typeof(*(self)) _item = (self)[i];                  \
                    memmove((self)+i, (self)+i+1, vec_len(self)-(i+1)); \
                    vec_len(self)--;                                    \
                    _item;  }))

#define vec_retain(self, f)                         \
    ({  size_t j = 0;                               \
        for (size_t i = 0; i < vec_len(self); i++)  \
            if (f(((self)[i])))                     \
                (self)[j++] = (self)[i];            \
        vec_len(vec) = j;  })

#define vec_push(self, item)                            \
    _VEC_IF_REALLOC(vec_len(self)+1 > vec_cap(self),    \
                    vec_reserve((self), 1),             \
                    (self)[vec_len(self)++] = item)
#define vec_pop(self)                                               \
    (vec_len(self)                                                  \
     ? (self)[--vec_len(self)]                                      \
     : (vec_err(self) = VEC_NO_ITEM_TO_POP, (typeof(*(self))) 0))

// array append and insert both assume typeof(self[0]) == typeof(arr[0])
#define vec_gappend vec_generic_append
#define vec_generic_append(self, arr, len)                              \
    ((len)                                                              \
     && _VEC_IF_REALLOC(vec_len(self) + (len) > vec_cap(self),          \
                        vec_reserve((self), (len)),                     \
                        (memcpy((self)+vec_len(self), (arr), sizeof(*(self)) * (len)), \
                         vec_len(self) += (len))))
#define vec_append(self, other)                             \
    (vec_generic_append((self), (other), vec_len(other))    \
     && (vec_len(other) = 0, vec_len(self)))
#define vec_aappend vec_array_append
#define vec_array_append(self, arr) vec_generic_append((self), (arr), _VEC_ARR_LEN(arr))

#define vec_insert(self, i, item)                                       \
    (_VEC_VALID_INDEX((self), (i))                                      \
     ? _VEC_IF_REALLOC(vec_len(self)+1 > vec_cap(self),                 \
                       vec_reserve((self), 1),                          \
                       (memmove((self)+(i)+1, (self)+(i), sizeof(*(self)) * (vec_len(self)-(i))), \
                        (self)[i] = item,                               \
                        vec_len(self)++))                               \
     : (vec_err(self) = VEC_INDEX_OUT_OF_BOUNDS), false)
#define vec_ginsert vec_generic_insert
#define vec_generic_insert(self, i , arr, len)                          \
    (_VEC_VALID_INDEX((self), (i))                                      \
     ? _VEC_IF_REALLOC(vec_len(self) + (len) > vec_cap(self),           \
                       vec_reserve((self), (len)),                      \
                       (memmove((self)+(i)+(len), (self)+(i), sizeof(*(self)) * (vec_len(self)-(i))), \
                        memcpy((self)+(i), (arr), sizeof(*(self)) * (len)), \
                        vec_len(self) += (len)))                        \
     : (vec_err(self) = VEC_INDEX_OUT_OF_BOUNDS), false)
#define vec_ainsert vec_array_insert
#define vec_array_insert(self, i, arr) vec_generic_insert((self), (i), (arr), _VEC_ARR_LEN(arr))
#define vec_vinsert vec_vector_insert
#define vec_vector_insert(self, i, other) vec_generic_insert((self), (i), (other), vec_len(other))

#define vec_clear(self) (vec_len(self) = 0)

#define vec_reversed(self)                                              \
    ({  typeof(self) _vec = vec_with_capacity(typeof(*(self)), vec_len(self)); \
        _vec                                                            \
            ? ({    for (size_t i = vec_len(self), j = 0; i--; j++) _vec[i] = (self)[j]; \
                    vec_len(_vec) = vec_len(self);                      \
                    _vec;  })                                           \
            : NULL;  })
#define vec_map(self, func)                                             \
    ({  typeof(func(*(self))) *_vec_vec = vec_with_capacity(typeof(func(*(self))), vec_len(self)); \
        _vec_vec                                                        \
            ? ({    for (size_t _vec_i = 0; _vec_i < vec_len(self); _vec_i++) \
                        _vec_vec[_vec_i] = func((self)[_vec_i]);        \
                    vec_len(_vec_vec) = vec_len(self);                  \
                    _vec_vec; })                                        \
            : NULL;  })
#define vec_filter(self, func)                                          \
    ({  typeof(self) _vec_vec = vec_with_capacity(typeof(*(self)), vec_len(self)); \
        _vec_vec                                                        \
            ? ({    for (size_t _vec_i = 0, j = 0; _vec_i < vec_len(self); _vec_i++) \
                        if (func((self)[_vec_i])) _vec_vec[j++] = (self)[_vec_i]; \
                    _vec_vec; })                                        \
            : NULL;  })
#define vec_reduce(self, func, init)                                    \
    ({  __auto_type _vec_acc = init;                                    \
        for (size_t _vec_i = 0; _vec_i < vec_len(self); _vec_i++) _vec_acc = func(_vec_acc, (self)[_vec_i]); \
        _vec_acc;  })
#define vec_foreach(self, func)                                         \
    ({  for (size_t _vec_i = 0; _vec_i < vec_len(self); _vec_i++) func((self)[_vec_i]);  })
#define vec_find(self, func)                                            \
    ({  size_t _vec_i = 0;                                              \
        bool _vec_found = false;                                        \
        for (; _vec_i < vec_len(self); _vec_i++) if (_vec_found = func((self)[_vec_i])) break; \
        _vec_found                                                      \
            ? _vec_i                                                    \
            : ((vec_err(self) = VEC_TARGET_NOT_FOUND), SIZE_MAX);  })
#define vec_any(self, func)                                             \
    ({  bool _vec_any = false;                                          \
        for (size_t _vec_i = 0; !_vec_any & _vec_i < vec_len(self); _vec_i++) \
            _vec_any = func((self)[_vec_i]);                            \
        _vec_any;  })
#define vec_all(self, func)                                             \
    ({  bool _vec_all = true;                                           \
        for (size_t _vec_i = 0; _vec_all & _vec_i < vec_len(self); _vec_i++) \
            _vec_all = func((self)[_vec_i]);                            \
        _vec_all;  })

// I don't think these are possible with different types
#define vec_zip(self, other)
#define vec_unzip(self)

#define vec_take(self, n) vec_generic_clone(self, n)
#define vec_drop(self, n) vec_generic_clone((self)+(n), vec_len(self)-(n))
#define vec_take_while(self, func)                                      \
    ({  typeof(self) _vec_vec = vec_new(typeof(*(self)), vec_len(self)); \
        size_t _vec_i = 0;                                              \
        for (; _vec_i < vec_len(self) && func((self)[_vec_i]); _vec_i++) _vec_vec[_vec_i] = (self)[_vec_i]; \
        vec_len(_vec_vec) = _vec_i;                                     \
        _vec_vec;  })
#define vec_drop_while(self, func)                                      \
    ({  typeof(self) _vec_vec = vec_new(typeof(*(self)), vec_len(self)); \
        bool _vec_cond = true;                                          \
        for (size_t _vec_i = 0; _vec_i < vec_len(self); _vec_i++)       \
            if (_vec_cond && (_vec_cond = func((self)[_vec_i]))) continue; \
            else vec_push(_vec_vec, ((self)[_vec_i]));                  \
        _vec_vec;  })
#define vec_partition(self, func)                                       \
    ({  typeof(self) _vec_arr[2] = {                                    \
            vec_new(typeof(*(self)), vec_len(self)),                    \
            vec_new(typeof(*(self)), vec_len(self)) };                  \
        ((uintptr_t) _vec_arr[0] & (uintptr_t) _vec_arr[1])             \
            ? ({    bool _vec_ret;                                      \
                    for (size_t _vec_i = 0; _vec_i < vec_len(self); _vec_i++) { \
                        _vec_ret = func((self)[_vec_i]);                \
                        _vec_arr[_vec_ret][vec_len(_vec_arr[_vec_ret])++] = (self)[_vec_i]; \
                    }                                                   \
                    vec_anew(_vec_arr); })                              \
            : (vec_err(self) = VEC_ALLOC_ERR, NULL); })

#define vec_flatten(self) // maybe it's possible to check for sub vectors by checking if the memory where the header would be is allocated

#define _VEC_THREE_WAY_CMP(a, b)                \
    ({  __auto_type _a = a;                     \
        __auto_type _b = b;                     \
        (_a > _b) - (_a < _b);  })
#define vec_search(self, target) vec_search_body((self), _VEC_THREE_WAY_CMP(vec_i, (target)))
#define vec_search_by(self, func) vec_search_body((self), func(vec_i))
#define vec_search_by_key(self, target, key)                            \
    vec_search_body((self), _VEC_THREE_WAY_CMP(key(vec_i), (target)))
#define vec_search_body(self, body)                                     \
    ({  size_t _vec_i = vec_generic_search((self), vec_len(self), (body)); \
        typeof(*(self)) vec_i = (self)[_vec_i];                         \
        ((intmax_t) (body) == 0)? _vec_i : (vec_err(self) = VEC_TARGET_NOT_FOUND, SIZE_MAX);  })
#define vec_generic_search_by(arr, len, func) vec_generic_search((arr), (len), func(vec_i))
#define vec_generic_search(arr, len, body)                              \
    ({  size_t _vec_idx[3] = { 0, 0, (len)-1 };                         \
        typeof(*(arr)) vec_i;                                           \
        int _vec_ret;                                                   \
        while (_vec_idx[0] <= _vec_idx[2]) {                            \
            _vec_idx[1] = _vec_idx[0] + (_vec_idx[2] - _vec_idx[0]) / 2; \
            vec_i = (arr)[_vec_idx[1]];                                 \
            _vec_ret = (body);                                          \
            if (_vec_ret < 0) _vec_idx[0] = _vec_idx[1] + 1;            \
            else if (_vec_ret > 0) _vec_idx[2] = _vec_idx[1] - 1;       \
            else break;  }                                              \
        do vec_i = (arr)[--(_vec_idx[1])]; while (_vec_idx[1] != SIZE_MAX && ((intmax_t) (body) == 0)); \
        _vec_idx[1]+1;  })

#define vec_sort(self) vec_sort_by((self), _VEC_LE)
#define vec_sort_by(self, cmp)                          \
    (vec_generic_sort_by((self), vec_len(self), cmp)    \
     || (vec_err(self) = VEC_ALLOC_ERR, false))
#define vec_generic_sort_by(arr, len, cmp)                              \
    ({  bool ok = false;                                                \
        typeof(arr) tmp = vec_with_capacity(typeof(*(arr)), (len));     \
        if (tmp) {                                                      \
            typeof(arr) src = arr, dst = tmp;                           \
            for (size_t width = 1; width < (len); width *= 2) {         \
                for (size_t i = 0; i < (len); i += 2 * width) {         \
                    size_t left = i, mid = i + width, right = i + 2 * width; \
                    if (mid > (len)) mid = (len);                       \
                    if (right > (len)) right = (len);                   \
                    size_t l = left, r = mid, d = left;                 \
                    while (l < mid & r < right)                         \
                        dst[d++] = cmp(src[l], src[r])? src[l++] : src[r++]; \
                    while (l < mid)                                     \
                        dst[d++] = src[l++];                            \
                    while (r < right)                                   \
                        dst[d++] = src[r++];                            \
                }                                                       \
                _VEC_SWAP(src, dst);                                    \
            }                                                           \
            if (src != (arr))                                           \
                for (size_t i = 0; i < (len); i++)                      \
                    (arr)[i] = src[i];                                  \
            vec_free(tmp);                                              \
            ok = true;                                                  \
        } ok; })
#define vec_sort_by_key(self, key) vec_msort((self), _VEC_LE, key)
#define vec_msort(self, cmp, key)                               \
    (vec_generic_sort_by_key((self), vec_len(self), cmp, key)   \
     || (vec_err(self) = VEC_ALLOC_ERR, false))
#define vec_generic_sort_by_key(arr, len, cmp, key)                     \
    ({  bool success = false;                                           \
        typeof(arr) tmp = vec_with_capacity(typeof(*(arr)), (len));     \
        typeof(key(*(arr)))                                             \
            *key1 = vec_with_capacity(typeof(*key1), (len)),            \
            *key2 = vec_with_capacity(typeof(*key1), (len));            \
        if ((uintptr_t) tmp & (uintptr_t) key1 & (uintptr_t) key2) {    \
            for (size_t i = 0; i < (len); i++) key1[i] = key((arr)[i]); \
            typeof(arr) src = arr, dst = tmp;                           \
            typeof(key1) ksrc = key1, kdst = key2;                      \
            for (size_t width = 1; width < (len); width *= 2) {         \
                for (size_t i = 0; i < (len); i += 2 * width) {         \
                    size_t left = i, mid = i + width, right = i + 2 * width; \
                    if (mid > (len)) mid = (len);                       \
                    if (right > (len)) right = (len);                   \
                    size_t l = left, r = mid, d = left;                 \
                                                                        \
                    while (l < mid & r < right)                         \
                        if (cmp(ksrc[l], ksrc[r])) {                    \
                            dst[d] = src[l];                            \
                            kdst[d++] = ksrc[l++];                      \
                        } else {                                        \
                            dst[d] = src[r];                            \
                            kdst[d++] = ksrc[r++];                      \
                        }                                               \
                    while (l < mid){                                    \
                        dst[d] = src[l];                                \
                        kdst[d++] = ksrc[l++];                          \
                    }                                                   \
                    while (r < right) {                                 \
                        dst[d] = src[r];                                \
                        kdst[d++] = ksrc[r++];                          \
                    }                                                   \
                }                                                       \
                _VEC_SWAP(src, dst);                                    \
                _VEC_SWAP(ksrc, kdst);                                  \
            }                                                           \
            if (src != (arr)) memcpy((arr), src, sizeof(*(arr)) * (len)); \
            success = true;                                             \
        }                                                               \
        if (tmp) vec_free(tmp);                                         \
        if (key1) vec_free(key1);                                       \
        if (key2) vec_free(key2);                                       \
        success; })

#define vec_sort_unstable(self) vec_sort_unstable_by((self), _VEC_LE)
#define vec_sort_unstable_by(self, cmp)                         \
    (vec_generic_sort_unstable_by((self), vec_len(self), cmp)   \
     || (vec_err(self) = VEC_ALLOC_ERR, false))
#define vec_generic_sort_unstable_by(arr, len, cmp)                     \
    ({  bool success = false;                                           \
        if ((len) > 1) {                                                \
            size_t *stack = vec_with_capacity(size_t, ceil(log2(len)) * 2 * 5); \
            if (stack) {                                                \
                vec_push(stack, 0);                                     \
                vec_push(stack, (len) - 1);                             \
                while (vec_len(stack)) {                                \
                    size_t high = vec_pop(stack);                       \
                    size_t low = vec_pop(stack);                        \
                    size_t mid = low + (high - low) / 2;                \
                                                                        \
                    if (cmp((arr)[mid], (arr)[low])) _VEC_SWAP((arr)[low], (arr)[mid]); \
                    if (cmp((arr)[low], (arr)[high])) _VEC_SWAP((arr)[high], (arr)[low]); \
                    if (cmp((arr)[high], (arr)[mid])) _VEC_SWAP((arr)[mid], (arr)[high]); \
                    _VEC_SWAP((arr)[mid], (arr)[high]);                 \
                    typeof(*(arr)) pivot = (arr)[high];                 \
                    size_t i = low;                                     \
                                                                        \
                    for (size_t j = low; j < high; j++)                 \
                        if (cmp((arr)[j], pivot)) {                     \
                            _VEC_SWAP((arr)[i], (arr)[j]);              \
                            i++;                                        \
                        }                                               \
                                                                        \
                    _VEC_SWAP((arr)[i], (arr)[high]);                   \
                                                                        \
                    if (i > low + 1)                                    \
                        (vec_push(stack, low),                          \
                         vec_push(stack, i - 1));                       \
                    if (i + 1 < high)                                   \
                        (vec_push(stack, i + 1),                        \
                         vec_push(stack, high));                        \
                }                                                       \
                vec_free(stack);                                        \
                success = true;                                         \
            }                                                           \
        } success; })
#define vec_sort_unstable_by_key(self, key) vec_qsort((self), _VEC_LE, key)
#define vec_qsort(self, cmp, key)                                       \
    (vec_generic_sort_unstable_by_key((self), vec_len(self), cmp, key)  \
     || (vec_err(self) = VEC_ALLOC_ERR, false))
#define vec_generic_sort_unstable_by_key(arr, len, cmp, key)            \
    ({  bool success = false;                                           \
        if ((len) > 1) {                                                \
            size_t *stack = vec_with_capacity(size_t, ceil(log2(len)) * 2 * 5); \
            typeof(key(*(arr))) *keys = vec_map((arr), key);            \
                                                                        \
            if ((uintptr_t) stack & (uintptr_t) keys) {                 \
                vec_push(stack, 0);                                     \
                vec_push(stack, (len)-1);                               \
                while (vec_len(stack)) {                                \
                    size_t high = vec_pop(stack);                       \
                    size_t low = vec_pop(stack);                        \
                    size_t mid = low + (high - low) / 2;                \
                                                                        \
                    if (cmp(keys[mid], keys[low])) _VEC_SWAP_WITH_KEY((arr), keys, low, mid); \
                    if (cmp(keys[low], keys[high])) _VEC_SWAP_WITH_KEY((arr), keys, high, low); \
                    if (cmp(keys[high], keys[mid])) _VEC_SWAP_WITH_KEY((arr), keys, mid, high); \
                    _VEC_SWAP_WITH_KEY((arr), keys, mid, high);         \
                    typeof(*keys) pivot = keys[high];                   \
                    size_t i = low;                                     \
                                                                        \
                    for (size_t j = low; j < high; j++)                 \
                        if (cmp((arr)[j], pivot)) {                     \
                            _VEC_SWAP_WITH_KEY((arr), keys, i, j);      \
                            i++;                                        \
                        }                                               \
                                                                        \
                    _VEC_SWAP_WITH_KEY((arr), keys, i, high);           \
                                                                        \
                    if (i > low + 1)                                    \
                        (vec_push(stack, low),                          \
                         vec_push(stack, i - 1));                       \
                    if (i + 1 < high)                                   \
                        (vec_push(stack, i + 1),                        \
                         vec_push(stack, high));                        \
                }                                                       \
                success = true;                                         \
            }                                                           \
            if (stack) vec_free(stack);                                 \
            if (keys) vec_free(keys);                                   \
        } success; })

#endif // VEC_H
