/*
 * SPDX-FileCopyrightText: Stone Tickle <lattis@mochiro.moe>
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "compat.h"

#include "functions/array.h"
#include "lang/func_lookup.h"
#include "lang/typecheck.h"
#include "util.h"

static bool
func_array_length(struct workspace *wk, obj self, obj *res)
{
	if (!pop_args(wk, NULL, NULL)) {
		return false;
	}

	*res = make_obj(wk, obj_number);
	set_obj_number(wk, *res, get_obj_array(wk, self)->len);
	return true;
}

static bool
func_array_get(struct workspace *wk, obj self, obj *res)
{
	struct args_norm an[] = { { obj_number }, { tc_any, .optional = true }, ARG_TYPE_NULL };
	if (!pop_args(wk, an, NULL)) {
		return false;
	}

	int64_t i = get_obj_number(wk, an[0].val);

	if (!bounds_adjust(get_obj_array(wk, self)->len, &i)) {
		if (an[1].set) {
			*res = an[1].val;
		} else {
			vm_error_at(wk, an[0].node, "index out of bounds");
			return false;
		}
	} else {
		*res = obj_array_index(wk, self, i);
	}

	return true;
}

struct array_contains_ctx {
	obj item;
	bool found;
};

static enum iteration_result
array_contains_iter(struct workspace *wk, void *_ctx, obj val)
{
	struct array_contains_ctx *ctx = _ctx;

	if (get_obj_type(wk, val) == obj_array) {
		obj_array_foreach(wk, val, ctx, array_contains_iter);
		if (ctx->found) {
			return ir_done;
		}
	}

	if (obj_equal(wk, val, ctx->item)) {
		ctx->found = true;
		return ir_done;
	}

	return ir_cont;
}

static bool
func_array_contains(struct workspace *wk, obj self, obj *res)
{
	struct args_norm an[] = { { tc_any }, ARG_TYPE_NULL };
	if (!pop_args(wk, an, NULL)) {
		return false;
	}

	struct array_contains_ctx ctx = { .item = an[0].val };
	obj_array_foreach(wk, self, &ctx, array_contains_iter);

	*res = make_obj_bool(wk, ctx.found);
	return true;
}

static bool
func_array_delete(struct workspace *wk, obj self, obj *res)
{
	struct args_norm an[] = { { tc_number }, ARG_TYPE_NULL };
	if (!pop_args(wk, an, NULL)) {
		return false;
	}

	int64_t idx = get_obj_number(wk, an[0].val);
	if (!boundscheck(wk, an[0].node, get_obj_array(wk, self)->len, &idx)) {
		return false;
	}

	obj_array_del(wk, self, idx);
	return true;
}

static bool
func_array_slice(struct workspace *wk, obj self, obj *res)
{
	struct args_norm an[] = { { obj_number, .optional = true }, { obj_number, .optional = true }, ARG_TYPE_NULL };
	if (!pop_args(wk, an, NULL)) {
		return false;
	}

	const struct obj_array *a = get_obj_array(wk, self);
	int64_t start = 0, end = a->len;

	if (an[0].set) {
		start = get_obj_number(wk, an[0].val);
	}

	if (an[1].set) {
		end = get_obj_number(wk, an[1].val);
	}

	bounds_adjust(a->len, &start);
	bounds_adjust(a->len, &end);

	end = MIN(end, a->len);

	*res = obj_array_slice(wk, self, start, end);
	return true;
}

static bool
func_array_clear(struct workspace *wk, obj self, obj *res)
{
	if (!pop_args(wk, 0, 0)) {
		return false;
	}

	obj_array_clear(wk, self);
	return true;
}

static bool
func_array_dedup(struct workspace *wk, obj self, obj *res)
{
	if (!pop_args(wk, 0, 0)) {
		return false;
	}

	obj_array_dedup(wk, self, res);
	return true;
}

const struct func_impl impl_tbl_array[] = {
	{ "length", func_array_length, tc_number, true },
	{ "get", func_array_get, tc_any, true },
	{ "contains", func_array_contains, tc_bool, true },
	{ NULL, NULL },
};

const struct func_impl impl_tbl_array_internal[] = {
	{ "length", func_array_length, tc_number, true },
	{ "get", func_array_get, tc_any, true },
	{ "contains", func_array_contains, tc_bool, true },
	{ "delete", func_array_delete },
	{ "slice", func_array_slice, tc_array, true },
	{ "clear", func_array_clear },
	{ "dedup", func_array_dedup, tc_array, true },
	{ NULL, NULL },
};
