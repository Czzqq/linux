// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2015 Josh Poimboeuf <jpoimboe@redhat.com>
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <subcmd/exec-cmd.h>
#include <subcmd/pager.h>
#include <linux/kernel.h>

#include <objtool/builtin.h>
#include <objtool/objtool.h>
#include <objtool/warn.h>

bool help;

static struct objtool_file file;

struct objtool_file *objtool_open_read(const char *filename)
{
	if (file.elf) {
		WARN("won't handle more than one file at a time");
		return NULL;
	}

	file.elf = elf_open_read(filename, O_RDWR);
	if (!file.elf)
		return NULL;

	hash_init(file.insn_hash);
	INIT_LIST_HEAD(&file.retpoline_call_list);
	INIT_LIST_HEAD(&file.return_thunk_list);
	INIT_LIST_HEAD(&file.static_call_list);
	INIT_LIST_HEAD(&file.mcount_loc_list);
	INIT_LIST_HEAD(&file.endbr_list);
	INIT_LIST_HEAD(&file.call_list);
	file.ignore_unreachables = opts.no_unreachable;
	file.hints = false;

	return &file;
}

void objtool_pv_add(struct objtool_file *f, int idx, struct symbol *func)
{
	if (!opts.noinstr)
		return;

	if (!f->pv_ops) {
		WARN("paravirt confusion");
		return;
	}

	/*
	 * These functions will be patched into native code,
	 * see paravirt_patch().
	 */
	if (!strcmp(func->name, "_paravirt_nop") ||
	    !strcmp(func->name, "_paravirt_ident_64"))
		return;

	/* already added this function */
	if (!list_empty(&func->pv_target))
		return;

	list_add(&func->pv_target, &f->pv_ops[idx].targets);
	f->pv_ops[idx].clean = false;
}

int main(int argc, const char **argv)
{
	static const char *UNUSED = "OBJTOOL_NOT_IMPLEMENTED";

	/* libsubcmd init */
	exec_cmd_init("objtool", UNUSED, UNUSED, UNUSED);
	pager_init(UNUSED);

	return objtool_run(argc, argv);
}
