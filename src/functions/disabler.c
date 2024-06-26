/*
 * SPDX-FileCopyrightText: Stone Tickle <lattis@mochiro.moe>
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "compat.h"

#include "lang/func_lookup.h"
#include "functions/disabler.h"
#include "lang/typecheck.h"
#include "log.h"

static bool
func_disabler_found(struct workspace *wk, obj self, obj *res)
{
	if (!pop_args(wk, NULL, NULL)) {
		return false;
	}

	make_obj(wk, res, obj_bool);
	set_obj_bool(wk, *res, false);
	return true;
}

const struct func_impl impl_tbl_disabler[] = {
	{ "found", func_disabler_found, tc_bool },
	{ NULL, NULL },
};
