/*
 * SPDX-FileCopyrightText: Stone Tickle <lattis@mochiro.moe>
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef MUON_FUNCTIONS_MODULES_H
#define MUON_FUNCTIONS_MODULES_H

#include "lang/func_lookup.h"

struct module_info {
	const char *name;
	const char *path;
	bool implemented;
};

extern const struct module_info module_info[module_count];
extern const struct func_impl impl_tbl_module[];
extern struct func_impl_group module_func_impl_groups[module_count][language_mode_count];

bool module_import(struct workspace *wk, const char *name, bool encapsulate, obj *res);
bool module_func_lookup(struct workspace *wk, const char *name, enum module mod, uint32_t *idx);
#endif
