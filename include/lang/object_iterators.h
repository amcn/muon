/*
 * SPDX-FileCopyrightText: Stone Tickle <lattis@mochiro.moe>
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef LANG_OBJECT_ITERATORS_H
#define LANG_OBJECT_ITERATORS_H

#include <stdbool.h>
#include <stdint.h>

#include "preprocessor_helpers.h"

/******************************************************************************
 * obj_array_for
 ******************************************************************************/

struct obj_array_for_helper {
	struct obj_array *a;
};

#define obj_array_for_(__wk, __arr, __val, __iter)                                        \
	struct obj_array_for_helper __iter = {                                            \
		.a = get_obj_array(__wk, __arr),                                          \
	};                                                                                \
	for (__val = __iter.a->len ? __iter.a->val : 0; __iter.a;                         \
		__iter.a = __iter.a->have_next ? get_obj_array(__wk, __iter.a->next) : 0, \
	    __val = __iter.a ? __iter.a->val : 0)

#define obj_array_for(__wk, __arr, __val) obj_array_for_(__wk, __arr, __val, CONCAT(__iter, __LINE__))

/******************************************************************************
 * obj_dict_for
 ******************************************************************************/

struct obj_dict_for_helper {
	struct obj_dict *d;
	struct hash *h;
	struct obj_dict_elem *e;
	void *k;
	uint64_t *v;
	uint32_t i;
	bool big;
};

#define obj_dict_for_get_kv_big(__iter, __key, __val)                                           \
	__iter.k = arr_get(&__iter.h->keys, __iter.i), __iter.v = hash_get(__iter.h, __iter.k), \
	__key = *__iter.v >> 32, __val = *__iter.v & 0xffffffff

#define obj_dict_for_get_kv(__iter, __key, __val) __key = __iter.e->key, __val = __iter.e->val

#define obj_dict_for_(__wk, __dict, __key, __val, __iter)                                                              \
	struct obj_dict_for_helper __iter = {                                                                          \
		.d = get_obj_dict(wk, __dict),                                                                         \
	};                                                                                                             \
	for (__key = 0,                                                                                                \
	    __val = 0,                                                                                                 \
	    __iter.big = __iter.d->flags & obj_dict_flag_big,                                                          \
	    __iter.h = __iter.big ? bucket_arr_get(&wk->vm.objects.dict_hashes, __iter.d->data) : 0,                   \
	    __iter.e = __iter.big    ? 0 :                                                                             \
		       __iter.d->len ? bucket_arr_get(&wk->vm.objects.dict_elems, __iter.d->data) :                    \
				       0,                                                                              \
	    __iter.big ? (__iter.i < __iter.h->keys.len ? (obj_dict_for_get_kv_big(__iter, __key, __val)) : 0) :       \
			 (__iter.e ? (obj_dict_for_get_kv(__iter, __key, __val)) : 0);                                 \
		__iter.big ? __iter.i < __iter.h->keys.len : !!__iter.e;                                               \
		__iter.big ? (++__iter.i,                                                                              \
			(__iter.i < __iter.h->keys.len ? (obj_dict_for_get_kv_big(__iter, __key, __val)) : 0)) :       \
			     (__iter.e = __iter.e->next ? bucket_arr_get(&wk->vm.objects.dict_elems, __iter.e->next) : \
							  0,                                                           \
			     (__iter.e ? (obj_dict_for_get_kv(__iter, __key, __val)) : 0)))

#define obj_dict_for(__wk, __dict, __key, __val) obj_dict_for_(__wk, __dict, __key, __val, CONCAT(__iter, __LINE__))

/******************************************************************************
 * obj_array_flat_for
 ******************************************************************************/

struct obj_array_flat_for_helper {
	struct workspace *wk;
	struct obj_array *a;
	uint32_t stack_base;
};

#define obj_array_flat_for_begin(__wk, __arr, __val)                                                                  \
	{                                                                                                             \
		struct obj_array_flat_for_helper __flat_iter = {                                                      \
			.wk = __wk,                                                                                   \
			.a = get_obj_array(__wk, __arr),                                                              \
			.stack_base = __wk->stack.len,                                                                \
		};                                                                                                    \
                                                                                                                      \
		while (true) {                                                                                        \
			__val = __flat_iter.a->val;                                                                   \
			if (get_obj_type(__flat_iter.wk, __val) == obj_array) {                                       \
				stack_push(                                                                           \
					&__flat_iter.wk->stack, __flat_iter.a, get_obj_array(__flat_iter.wk, __val)); \
				continue;                                                                             \
			}                                                                                             \
                                                                                                                      \
			if (__val)

#define obj_array_flat_for_end                                                                            \
	}                                                                                                 \
	if (__flat_iter.a->have_next) {                                                                   \
		__flat_iter.a = get_obj_array(__flat_iter.wk, __flat_iter.a->next);                       \
	} else if (__flat_iter.stack_base < __flat_iter.wk->stack.len) {                                  \
		stack_pop(&__flat_iter.wk->stack, __flat_iter.a);                                         \
		while (__flat_iter.stack_base < __flat_iter.wk->stack.len && !__flat_iter.a->have_next) { \
			stack_pop(&__flat_iter.wk->stack, __flat_iter.a);                                 \
		}                                                                                         \
		if (__flat_iter.a->have_next) {                                                           \
			__flat_iter.a = get_obj_array(__flat_iter.wk, __flat_iter.a->next);               \
		} else {                                                                                  \
			break;                                                                            \
		}                                                                                         \
	} else {                                                                                          \
		break;                                                                                    \
	}                                                                                                 \
	}

#endif
