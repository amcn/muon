/*
 * SPDX-FileCopyrightText: Stone Tickle <lattis@mochiro.moe>
 * SPDX-License-Identifier: GPL-3.0-only
 */

#ifndef FUNCTIONS_BOTH_LIBS_H
#define FUNCTIONS_BOTH_LIBS_H
#include "lang/func_lookup.h"

obj decay_both_libs(struct workspace *wk, obj both_libs);

void both_libs_build_impl_tbl(void);
extern struct func_impl impl_tbl_both_libs[];
#endif
