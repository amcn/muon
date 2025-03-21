/*
 * SPDX-FileCopyrightText: Stone Tickle <lattis@mochiro.moe>
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "compat.h"

#include "lang/func_lookup.h"
#include "functions/feature_opt.h"
#include "lang/typecheck.h"

static bool
feature_opt_common(struct workspace *wk, obj self, obj *res, enum feature_opt_state state)
{
	if (!pop_args(wk, NULL, NULL)) {
		return false;
	}

	*res = make_obj_bool(wk, get_obj_feature_opt(wk, self) == state);
	return true;
}

static bool
func_feature_opt_auto(struct workspace *wk, obj self, obj *res)
{
	return feature_opt_common(wk, self, res, feature_opt_auto);
}

static bool
func_feature_opt_disabled(struct workspace *wk, obj self, obj *res)
{
	return feature_opt_common(wk, self, res, feature_opt_disabled);
}

static bool
func_feature_opt_enabled(struct workspace *wk, obj self, obj *res)
{
	return feature_opt_common(wk, self, res, feature_opt_enabled);
}

static bool
func_feature_opt_allowed(struct workspace *wk, obj self, obj *res)
{
	if (!pop_args(wk, NULL, NULL)) {
		return false;
	}

	enum feature_opt_state state = get_obj_feature_opt(wk, self);

	*res = make_obj_bool(wk, state == feature_opt_auto || state == feature_opt_enabled);
	return true;
}

static bool
func_feature_opt_disable_auto_if(struct workspace *wk, obj self, obj *res)
{
	struct args_norm an[] = { { tc_bool }, ARG_TYPE_NULL };
	if (!pop_args(wk, an, NULL)) {
		return false;
	}

	enum feature_opt_state state = get_obj_feature_opt(wk, self);

	if (!get_obj_bool(wk, an[0].val)) {
		*res = self;
		return true;
	} else if (state == feature_opt_disabled || state == feature_opt_enabled) {
		*res = self;
		return true;
	} else {
		*res = make_obj(wk, obj_feature_opt);
		set_obj_feature_opt(wk, *res, feature_opt_disabled);
		return true;
	}
}

static bool
func_feature_opt_enable_auto_if(struct workspace *wk, obj self, obj *res)
{
	struct args_norm an[] = { { tc_bool }, ARG_TYPE_NULL };
	if (!pop_args(wk, an, NULL)) {
		return false;
	}

	enum feature_opt_state state = get_obj_feature_opt(wk, self);

	if (!get_obj_bool(wk, an[0].val)) {
		*res = self;
		return true;
	} else if (state == feature_opt_disabled || state == feature_opt_enabled) {
		*res = self;
		return true;
	} else {
		*res = make_obj(wk, obj_feature_opt);
		set_obj_feature_opt(wk, *res, feature_opt_enabled);
		return true;
	}
}

static bool
func_feature_opt_enable_if(struct workspace *wk, obj self, obj *res)
{
	struct args_norm an[] = { { tc_bool }, ARG_TYPE_NULL };
	enum kwargs {
		kw_error_message,
	};
	struct args_kw akw[] = { [kw_error_message] = { "error_message", obj_string }, 0 };
	if (!pop_args(wk, an, akw)) {
		return false;
	}

	enum feature_opt_state state = get_obj_feature_opt(wk, self);

	if (!get_obj_bool(wk, an[0].val)) {
		*res = self;
		return true;
	} else if (state == feature_opt_disabled) {
		const char *err_msg = akw[kw_error_message].set ? get_cstr(wk, akw[kw_error_message].set) :
								  "requirement not met";

		vm_error_at(wk, an[0].node, "%s", err_msg);
		return false;
	} else {
		*res = make_obj(wk, obj_feature_opt);
		set_obj_feature_opt(wk, *res, feature_opt_enabled);
		return true;
	}

	return true;
}

static bool
func_feature_opt_disable_if(struct workspace *wk, obj self, obj *res)
{
	struct args_norm an[] = { { tc_bool }, ARG_TYPE_NULL };
	enum kwargs {
		kw_error_message,
	};
	struct args_kw akw[] = { [kw_error_message] = { "error_message", obj_string }, 0 };
	if (!pop_args(wk, an, akw)) {
		return false;
	}

	enum feature_opt_state state = get_obj_feature_opt(wk, self);

	if (!get_obj_bool(wk, an[0].val)) {
		*res = self;
		return true;
	} else if (state == feature_opt_enabled) {
		const char *err_msg = akw[kw_error_message].set ? get_cstr(wk, akw[kw_error_message].set) :
								  "requirement not met";

		vm_error_at(wk, an[0].node, "%s", err_msg);
		return false;
	} else {
		*res = make_obj(wk, obj_feature_opt);
		set_obj_feature_opt(wk, *res, feature_opt_disabled);
		return true;
	}

	return true;
}

static bool
func_feature_opt_require(struct workspace *wk, obj self, obj *res)
{
	struct args_norm an[] = { { tc_bool }, ARG_TYPE_NULL };
	enum kwargs {
		kw_error_message,
	};
	struct args_kw akw[] = { [kw_error_message] = { "error_message", obj_string }, 0 };

	if (!pop_args(wk, an, akw)) {
		return false;
	}

	enum feature_opt_state state = get_obj_feature_opt(wk, self);
	if (!get_obj_bool(wk, an[0].val)) {
		if (state == feature_opt_enabled) {
			vm_error_at(wk,
				an[0].node,
				"%s",
				akw[kw_error_message].set ? get_cstr(wk, akw[kw_error_message].set) :
							    "requirement not met");
			return false;
		} else {
			*res = make_obj(wk, obj_feature_opt);
			set_obj_feature_opt(wk, *res, feature_opt_disabled);
		}
	} else {
		*res = self;
	}

	return true;
}

const struct func_impl impl_tbl_feature_opt[] = {
	{ "allowed", func_feature_opt_allowed, tc_bool, true },
	{ "auto", func_feature_opt_auto, tc_bool, true },
	{ "disable_auto_if", func_feature_opt_disable_auto_if, tc_feature_opt, true },
	{ "disable_if", func_feature_opt_disable_if, tc_feature_opt, true },
	{ "disabled", func_feature_opt_disabled, tc_bool, true },
	{ "enable_auto_if", func_feature_opt_enable_auto_if, tc_feature_opt, true },
	{ "enable_if", func_feature_opt_enable_if, tc_feature_opt, true },
	{ "enabled", func_feature_opt_enabled, tc_bool, true },
	{ "require", func_feature_opt_require, tc_feature_opt, true },
	{ NULL, NULL },
};
