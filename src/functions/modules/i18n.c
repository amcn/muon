/*
 * SPDX-FileCopyrightText: Stone Tickle <lattis@mochiro.moe>
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "compat.h"
#include "coerce.h"

#include "platform/path.h"
#include "platform/filesystem.h"
#include "lang/interpreter.h"
#include "functions/kernel/custom_target.h"
#include "functions/modules/i18n.h"

static bool
parse_linguas_file(struct workspace *wk, uint32_t node, obj *languages)
{
	SBUF(linguas_path);
	struct source linguas_src;

	path_join(wk, &linguas_path, get_cstr(wk, current_project(wk)->cwd), "LINGUAS");

	FILE *linguas_file = fs_fopen(linguas_path.buf, "r");
	if (!linguas_file) {

	}

	if (!fs_read_entire_file(linguas_path.buf, &linguas_src)) {
		return false;
	}

	fprintf(stderr, "Found a LINGUAS file at %s\n", linguas_path.buf);
	return true;
}

static bool
i18n_gettext_make_pot_target(struct workspace *wk, const char *package_name)
{
	obj pot_target;
	struct make_custom_target_opts pot_opts = {
		.name = make_strf(wk, "%s-pot", package_name),
		.input_orig =
	};

	if (!make_custom_target(wk, &pot_opts, &pot_target)) {
		return false;
	}

	obj_array_push(wk, current_project(wk)->targets, pot_target);

}

static bool
func_module_i18n_gettext(struct workspace *wk, obj rcvr, uint32_t args_node,
	obj *res)
{
	SBUF(msgfmt_path);
	SBUF(msginit_path);
	SBUF(msgmerge_path);
	SBUF(xgettext_path);

	struct args_norm an[] = { { obj_string }, ARG_TYPE_NULL };
	enum kwargs {
		kw_args,
		kw_data_dirs,
		kw_languages,
		kw_preset,
		kw_install,
		kw_install_dir
	};
	struct args_kw akw[] = {
		[kw_args] = { "args", ARG_TYPE_ARRAY_OF | tc_string },
		[kw_data_dirs] = { "data_dirs", ARG_TYPE_ARRAY_OF | tc_string },
		[kw_languages] = { "languages", ARG_TYPE_ARRAY_OF | tc_string },
		[kw_preset] = { "preset", tc_string },
		[kw_install] = { "install", tc_bool },
		[kw_install_dir] = { "install_dir", tc_string },
		0
	};

	enum tool_type {
		tool_msgfmt,
		tool_msginit,
		tool_msgmerge,
		tool_xgettext,
		tool_count
	};

	struct gettext_tool {
		const char *name;
		bool required;
		struct sbuf *path;
	} tools[tool_count] = {
		[tool_msgfmt] = {
			.name = "msgfmt", .required = true, .path = &msgfmt_path },
		[tool_msginit] = {
			.name = "msginit", .required = false, .path = &msginit_path },
		[tool_msgmerge] = {
			.name = "msgmerge", .required = false, .path = &msgmerge_path },
		[tool_xgettext] = {
			.name = "xgettext", .required = false, .path = &xgettext_path },
	};

	if (!interp_args(wk, args_node, an, NULL, akw)) {
		return false;
	}

	const char *package_name = get_cstr(wk, an[0].val);

	for (size_t i = 0; i < ARRAY_LEN(tools); ++i) {
		struct gettext_tool tool = tools[i];

		bool found = fs_find_cmd(wk, tool.path, tool.name);
		if (found)
			continue;

		if (tool.required) {
			interp_warning(wk, args_node, "%s not found, translation targets will be ignored", tool.name);
			*res = 0;
			return true;
		}

		interp_warning(wk, args_node, "%s not found, maintainer targets will not work", tool.name);
	}

	if (!i18n_gettext_make_pot_target(wk, package_name)) {
		return false;
	}

	// If the user did not explicitly pass a `languages` kwarg, try to read a
	// LINGUAS file.
	if (!akw[kw_languages].set) {
		if (!i18n_parse_linguas_file(wk, args_node, &akw[kw_languages].val)) {
			return false;
		}
	}

	obj update_po_target;
	struct make_custom_target_opts update_po_opts = {
		.name = make_str(wk, "update-po")
	};

	if (!make_custom_target(wk, &update_po_opts, &update_po_target)) {
		return false;
	}

	obj_array_push(wk, current_project(wk)->targets, update_po_target);

	obj mo_files_array;
	make_obj(wk, &mo_files_array, obj_array);

	make_obj(wk, res, obj_array);
	obj_array_push(wk, *res, mo_files_array);
	obj_array_push(wk, *res, pot_target);
	obj_array_push(wk, *res, update_po_target);
	interp_warning(wk, args_node, "Donezo");
	return true;
}

static bool
func_module_i18n_merge_file(struct workspace *wk, obj rcvr, uint32_t args_node,
	obj *res)
{
	make_obj(wk, res, obj_custom_target);
	return true;
}

static bool
func_module_i18n_itstool_join(struct workspace *wk, obj rcvr, uint32_t args_node,
	obj *res)
{
	make_obj(wk, res, obj_custom_target);

	return true;
}

const struct func_impl_name impl_tbl_module_i18n[] = {
	{ "gettext", func_module_i18n_gettext, tc_array },
	{ "merge_file", func_module_i18n_merge_file, tc_custom_target },
	{ "itstool_join", func_module_i18n_itstool_join, tc_custom_target },
	{ NULL, NULL },
};
