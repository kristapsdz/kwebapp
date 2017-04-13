/*	$Id$ */
/*
 * Copyright (c) 2017 Kristaps Dzonsons <kristaps@bsd.lv>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include "config.h"

#include <sys/queue.h>

#if HAVE_ERR
# include <err.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#include "extern.h"

/*
 * Generate the C API for a given field.
 */
static void
gen_strct_field(const struct field *p)
{

	if (NULL != p->doc)
		print_commentt(1, COMMENT_C, p->doc);

	switch (p->type) {
	case (FTYPE_STRUCT):
		printf("\tstruct %s %s;\n", 
			p->ref->tstrct, p->name);
		break;
	case (FTYPE_INT):
		printf("\tint64_t %s;\n", p->name);
		break;
	case (FTYPE_TEXT):
		printf("\tchar *%s;\n", p->name);
		break;
	default:
		break;
	}
}

/*
 * Generate the C API for a given structure.
 * This generates the TAILQ_ENTRY listing if the structure has any
 * listings declared on it.
 */
static void
gen_strct_structs(const struct strct *p)
{
	const struct field *f;

	print_commentt(0, COMMENT_C, p->doc);

	printf("struct\t%s {\n", p->name);
	TAILQ_FOREACH(f, &p->fq, entries)
		gen_strct_field(f);
	if (STRCT_HAS_QUEUE & p->flags)
		printf("\tTAILQ_ENTRY(%s) _entries;\n", p->name);
	puts("};\n"
	     "");

	if (STRCT_HAS_QUEUE & p->flags) {
		print_commentv(0, COMMENT_C, 
			"Queue of %s for listings.", p->name);
		printf("TAILQ_HEAD(%s_q, %s);\n\n", p->name, p->name);
	}

	if (STRCT_HAS_ITERATOR & p->flags) {
		print_commentv(0, COMMENT_C, 
			"Callback of %s for iteration.", p->name);
		printf("typedef void (*%s_cb)"
		       "(const struct %s *v, void *arg);\n\n", 
		       p->name, p->name);
	}
}

static void
gen_strct_func_update(const struct update *up)
{
	const struct uref *ref;
	enum cmtt	 ct = COMMENT_C_FRAG_OPEN;
	size_t		 pos;

	if (NULL != up->doc) {
		print_commentt(0, COMMENT_C_FRAG_OPEN, up->doc);
		print_commentt(0, COMMENT_C_FRAG, "\n");
		ct = COMMENT_C_FRAG;
	}

	print_commentv(0, ct,
		"Updates the given fields in struct %s:",
		up->parent->name);

	pos = 1;
	TAILQ_FOREACH(ref, &up->mrq, entries)
		print_commentv(0, COMMENT_C_FRAG,
			"\tv%zu: %s", pos++, ref->name);

	print_commentt(0, COMMENT_C_FRAG,
		"Constrains the updated records to:");

	TAILQ_FOREACH(ref, &up->crq, entries)
		print_commentv(0, COMMENT_C_FRAG,
			"\tv%zu: %s", pos++, ref->name);

	print_commentt(0, COMMENT_C_FRAG_CLOSE, NULL);
	print_func_update(up, 1);
	puts("");
}

/*
 * Generate a custom search function declaration.
 */
static void
gen_strct_func_search(const struct search *s)
{
	const struct sent *sent;
	const struct sref *sr;
	size_t	 pos = 1, first;

	if (NULL != s->doc)
		print_commentt(0, COMMENT_C_FRAG_OPEN, s->doc);
	else if (STYPE_SEARCH == s->type)
		print_commentv(0, COMMENT_C_FRAG_OPEN,
			"Search for a specific %s.", 
			s->parent->name);
	else
		print_commentv(0, COMMENT_C_FRAG_OPEN,
			"Search for a set of %s.", 
			s->parent->name);

	print_commentv(0, COMMENT_C_FRAG,
		"\nUses the given fields in struct %s:",
	       s->parent->name);

	TAILQ_FOREACH(sent, &s->sntq, entries) {
		printf(" * ");
		first = 1;
		TAILQ_FOREACH(sr, &sent->srq, entries) {
			if (first) {
				printf("\tv%zu: ", pos);
				first = 0;
			} else
				putchar('.');
			printf("%s", sr->name);
		}
		sr = TAILQ_LAST(&sent->srq, srefq);
		puts("");
		pos++;
	}

	if (STYPE_SEARCH == s->type)
		print_commentv(0, COMMENT_C_FRAG_CLOSE,
			"Returns a pointer or NULL on fail.\n"
			"Free the pointer with db_%s_free().",
			s->parent->name);
	else if (STYPE_LIST == s->type)
		print_commentv(0, COMMENT_C_FRAG_CLOSE,
			"Always returns a queue pointer.\n"
			"Free this with db_%s_freeq().",
			s->parent->name);
	else
		print_commentv(0, COMMENT_C_FRAG_CLOSE,
			"Invokes the given callback with "
			"retrieved data.");

	print_func_search(s, 1);
	puts("");
}

/*
 * Generate the function declarations for a given structure.
 */
static void
gen_strct_funcs(const struct strct *p)
{
	const struct search *s;
	const struct field *f;
	const struct update *u;
	size_t	 pos;

	if (NULL != p->rowid) {
		print_commentv(0, COMMENT_C,
		       "Return the %s with rowid \"id\".\n"
		       "Returns NULL if no object was found.\n"
		       "Pointer must be freed with db_%s_free().",
		       p->name, p->name);
		print_func_by_rowid(p, 1);
		puts("");
	}

	print_commentv(0, COMMENT_C,
	       "Call db_%s_unfill() and free \"p\".\n"
	       "Has no effect if \"p\" is NULL.",
	       p->name);
	print_func_free(p, 1);
	puts("");

	if (STRCT_HAS_QUEUE & p->flags) {
		print_commentv(0, COMMENT_C,
		     "Unfill and free all queue members.\n"
		     "Has no effect if \"q\" is NULL.");
		print_func_freeq(p, 1);
		puts("");
	}

	print_commentv(0, COMMENT_C, 
	       "Fill in a %s from an open statement \"stmt\".\n"
	       "This starts grabbing results from \"pos\", "
	       "which may be NULL to start from zero.\n"
	       "This recursively invokes the \"fill\" function "
	       "for all nested structures.",
	       p->name);
	print_func_fill(p, 1);
	puts("");

	print_commentt(0, COMMENT_C_FRAG_OPEN,
		"Insert a new row into the database.\n"
		"Only native (and non-rowid) fields may "
		"be set.");
	pos = 1;
	TAILQ_FOREACH(f, &p->fq, entries) {
		if (FTYPE_STRUCT == f->type ||
		    FIELD_ROWID & f->flags)
			continue;
		printf(" *\tv%zu: %s\n", pos++, f->name);
	}
	print_commentt(0, COMMENT_C_FRAG_CLOSE,
		"Returns zero on failure, non-zero "
		"otherwise.");
	print_func_insert(p, 1);
	puts("");

	print_commentv(0, COMMENT_C,
	       "Free memory allocated by db_%s_fill().\n"
	       "Also frees for all contained structures.\n"
	       "Has not effect if \"p\" is NULL.",
	       p->name);
	print_func_unfill(p, 1);
	puts("");

	TAILQ_FOREACH(s, &p->sq, entries)
		gen_strct_func_search(s);

	TAILQ_FOREACH(u, &p->uq, entries)
		gen_strct_func_update(u);
}

void
gen_header(const struct strctq *q)
{
	const struct strct *p;

	puts("#ifndef DB_H\n"
	     "#define DB_H\n"
	     "");
	print_commentt(0, COMMENT_C, 
	       "WARNING: automatically generated by "
	       "kwebapp " VERSION ".\n"
	       "DO NOT EDIT!");
	puts("");

	TAILQ_FOREACH(p, q, entries)
		gen_strct_structs(p);

	puts("__BEGIN_DECLS\n"
	     "");

	TAILQ_FOREACH(p, q, entries)
		gen_strct_funcs(p);

	puts("__END_DECLS\n"
	     "\n"
	     "#endif");
}
