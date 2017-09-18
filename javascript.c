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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "extern.h"

static void
gen_jsdoc_field(const struct field *f)
{

	if (FIELD_NOEXPORT & f->flags || FTYPE_BLOB == f->type)
		return;

	if (FIELD_NULL & f->flags) {
		print_commentv(2, COMMENT_JS_FRAG,
			"%s-has-%s: \"hide\" class "
			"removed if %s not null, otherwise "
			"\"hide\" class is added",
			f->parent->name, f->name, f->name);
		print_commentv(2, COMMENT_JS_FRAG,
			"%s-no-%s: \"hide\" class "
			"added if %s not null, otherwise "
			"\"hide\" class is removed",
			f->parent->name, f->name, f->name);
	} 

	if (FTYPE_STRUCT == f->type) {
		print_commentv(2, COMMENT_JS_FRAG,
			"%s-%s-obj: invoke %s.fillInner() method "
			"with %s data%s",
			f->parent->name, f->name, 
			f->ref->tstrct, f->name,
			FIELD_NULL & f->flags ? 
			" (if non-null)" : "");
	} else {
		print_commentv(2, COMMENT_JS_FRAG,
			"%s-%s-text: replace contents "
			"with %s data%s",
			f->parent->name, f->name, f->name,
			FIELD_NULL & f->flags ? 
			" (if non-null)" : "");
		print_commentv(2, COMMENT_JS_FRAG,
			"%s-%s-value: replace \"value\" "
			"attribute with %s data%s",
			f->parent->name, f->name, f->name,
			FIELD_NULL & f->flags ? 
			" (if non-null)" : "");
	}
}

static void
gen_js_field(const struct field *f)
{
	size_t	 indent;

	if (FIELD_NOEXPORT & f->flags || FTYPE_BLOB == f->type)
		return;

	if (FIELD_NULL & f->flags) {
		indent = 4;
		printf("\t\t\tif (null === o.%s) {\n"
		       "\t\t\t\t_hidecl(e, '%s-has-%s', inc);\n"
		       "\t\t\t\t_showcl(e, '%s-no-%s', inc);\n"
		       "\t\t\t} else {\n"
		       "\t\t\t\t_showcl(e, '%s-has-%s', inc);\n"
		       "\t\t\t\t_hidecl(e, '%s-no-%s', inc);\n",
		       f->name, 
		       f->parent->name, f->name,
		       f->parent->name, f->name,
		       f->parent->name, f->name,
		       f->parent->name, f->name);
	} else
		indent = 3;

	if (FTYPE_STRUCT != f->type) {
		print_src(indent,
			"_replcl(e, '%s-%s-text', o.%s, inc);",
			f->parent->name, f->name, f->name);
		print_src(indent,
			"_attrcl(e, 'value', "
			"'%s-%s-value', o.%s, inc);",
			f->parent->name, f->name, f->name);
	} else
		print_src(indent,
			"list = _elemList(e, '%s-%s-obj');\n"
		        "strct = new %s(o.%s);\n"
		        "for (i = 0; i < list.length; i++) {\n"
		        "strct.fillInner(list[i]);\n"
		        "}",
		        f->parent->name, f->name, 
		        f->ref->tstrct, f->name);

	if (FIELD_NULL & f->flags)
		puts("\t\t\t}");
}

void
gen_javascript(const struct strctq *sq)
{
	const struct strct *s;
	const struct field *f;

	/*
	 * Begin with the methods we'll use throughout the js file.
	 * This includes _attr, which sets an attribute of an element; 
	 * _attrcl, which sets the attribute of all elements of a given
	 * className under a root; _repl, which is like _attr except
	 * in setting the element's child as a text node (after clearing
	 * it); _replcl, which is like _attrcl but for _repl; _hide,
	 * which adds the "hide" class to an element; and _show, which
	 * removes the "hide" class.
	 * These use _elemList, which is like getElementsByClassName
	 * except that it also considers the root and returns an array,
	 * not an HTMLCollection.
	 */

	puts("(function(root) {\n"
	     "\t'use strict';\n"
	     "\n"
	     "\tfunction _attr(e, attr, text)\n"
	     "\t{\n"
	     "\t\tif (null === e)\n"
	     "\t\t\treturn;\n"
	     "\t\te.setAttribute(attr, text);\n"
	     "\t}\n"
	     "\n"
	     "\tfunction _attrcl(e, attr, name, text, inc)\n"
	     "\t{\n"
	     "\t\tvar list, i;\n"
	     "\t\tif (null === e)\n"
	     "\t\t\treturn;\n"
	     "\t\tlist = _elemList(e, name, inc);\n"
	     "\t\tfor (i = 0; i < list.length; i++)\n"
	     "\t\t\t_attr(list[i], attr, text);\n"
	     "\t}\n"
	     "\n"
	     "\tfunction _elemList(e, cls, inc)\n"
	     "\t{\n"
	     "\t\tvar a = [], list, i;\n"
	     "\t\tif (null === e)\n"
	     "\t\t\treturn(a);\n"
	     "\t\tlist = e.getElementsByClassName(cls);\n"
	     "\t\tfor (i = 0; i < list.length; i++)\n"
	     "\t\t\ta.push(list[i]);\n"
	     "\t\tif (inc && e.classList.contains(cls))\n"
	     "\t\t\ta.push(e);\n"
	     "\t\treturn(a);\n"
	     "\t}\n"
	     "\n"
	     "\tfunction _repl(e, text)\n"
	     "\t{\n"
	     "\t\tif (null === e)\n"
	     "\t\t\treturn;\n"
	     "\t\twhile (e.firstChild)\n"
	     "\t\t\te.removeChild(e.firstChild);\n"
	     "\t\te.appendChild(document.createTextNode(text));\n"
	     "\t}\n"
	     "\n"
	     "\tfunction _replcl(e, name, text, inc)\n"
	     "\t{\n"
	     "\t\tvar list, i;\n"
	     "\t\tif (null === e)\n"
	     "\t\t\treturn;\n"
	     "\t\tlist = _elemList(e, name, inc);\n"
	     "\t\tfor (i = 0; i < list.length; i++)\n"
	     "\t\t\t_repl(list[i], text);\n"
	     "\t}\n"
	     "\n"
	     "\tfunction _hide(e)\n"
	     "\t{\n"
	     "\t\tif (null === e)\n"
	     "\t\t\treturn(null);\n"
	     "\t\tif ( ! e.classList.contains('hide'))\n"
	     "\t\t\te.classList.add('hide');\n"
	     "\t\treturn(e);\n"
	     "\t}\n"
	     "\t\n"
	     "\tfunction _hidecl(e, name, inc)\n"
	     "\t{\n"
	     "\t\tvar list, i;\n"
	     "\t\tif (null === e)\n"
	     "\t\t\treturn;\n"
	     "\t\tlist = _elemList(e, name, inc);\n"
	     "\t\tfor (i = 0; i < list.length; i++)\n"
	     "\t\t\t_hide(list[i]);\n"
	     "\t}\n"
	     "\n"
	     "\tfunction _show(e)\n"
	     "\t{\n"
	     "\t\tif (null === e)\n"
	     "\t\t\treturn(null);\n"
	     "\t\tif (e.classList.contains('hide'))\n"
	     "\t\t\te.classList.remove('hide');\n"
	     "\t\treturn(e);\n"
	     "\t}\n"
	     "\t\n"
	     "\tfunction _showcl(e, name, inc)\n"
	     "\t{\n"
	     "\t\tvar list, i;\n"
	     "\t\tif (null === e)\n"
	     "\t\t\treturn;\n"
	     "\t\tlist = _elemList(e, name, inc);\n"
	     "\t\tfor (i = 0; i < list.length; i++)\n"
	     "\t\t\t_show(list[i]);\n"
	     "\t}\n"
	     "");

	/*
	 * This is pretty straightforward.
	 * Each structure is an object initialised by either an object
	 * from the server or an array of objects.
	 * Each object has the "fill" and "fillArray" methods.
	 * These use the internal _fill method, which accepts both the
	 * object (or array) and the element to be filled.
	 */

	TAILQ_FOREACH(s, sq, entries) {
		print_commentv(1, COMMENT_JS,
			"Represent a \"%s\" object (or array of "
			"objects) for filling into a DOM tree.\n"
			"@constructor\n"
			"@param {(Object|Object[])} obj - The %s "
			"object, which may also be an array of "
			"objects.",
			s->name, s->name);
		printf("\tfunction %s(obj)\n"
		       "\t{\n"
		       "\t\tthis.obj = obj;\n"
		       "\n",
		       s->name);

		print_commentv(2, COMMENT_JS_FRAG_OPEN,
			"Fill in a \"%s\" object at the given "
			"element in the DOM tree.\n"
			"If the object was initialised with an "
			"array, the first element is used.\n"
			"Elements within (and including) #e having "
			"the following classes are manipulated as "
			"follows:", s->name);
		TAILQ_FOREACH(f, &s->fq, entries)
			gen_jsdoc_field(f);
		print_commentt(2, COMMENT_JS_FRAG_CLOSE,
			"@param {Object} e - The DOM element.");
		puts("\t\tthis.fill = function(e) {\n"
		     "\t\t\tthis._fill(e, this.obj, 1);\n"
		     "\t\t};\n"
		     "");

		print_commentv(2, COMMENT_JS,
			"Like fill() but not including the root "
			"element #e.\n"
			"@param {Object} e - The DOM element.");
		puts("\t\tthis.fillInner = function(e) {\n"
		     "\t\t\tthis._fill(e, this.obj, 0);\n"
		     "\t\t};\n"
		     "");

		print_commentv(2, COMMENT_JS,
			"Implements all fill() style functions.\n"
			"@private\n"
			"@param {Object} e - the DOM element.\n"
			"@param {(Object|Object[])} o - the object "
			"(or array) to fill\n"
			"@param {number} inc - whether to include "
			"the root or not when processing");
		puts("\t\tthis._fill = function(e, o, inc) {");
		TAILQ_FOREACH(f, &s->fq, entries)
			if ( ! (FIELD_NOEXPORT & f->flags) &&
			    FTYPE_STRUCT == f->type) {
				puts("\t\t\tvar list, strct, i;");
				break;
			}
		puts("\t\t\tif (null === o || null === e)\n"
		     "\t\t\t\treturn;\n"
		     "\t\t\tif (o instanceof Array) {\n"
		     "\t\t\t\tif (0 === o.length)\n"
		     "\t\t\t\t\treturn;\n"
		     "\t\t\t\to = o[0];\n"
		     "\t\t\t}");
		TAILQ_FOREACH(f, &s->fq, entries)
			gen_js_field(f);
		puts("\t\t};\n"
		     "");

		print_commentv(2, COMMENT_JS,
			"Like fill() but for an array of %s.\n"
			"This will remove the first element within "
			"#e then repeatedly clone and re-append it,\n"
			"filling in the cloned subtree with the "
			"array.\n"
			"If #e is not an array, it is construed "
			"as an array of one.\n"
			"If the input array is empty, #e is hidden "
			"by using the \"hide\" class.\n"
			"Otherwise, the \"hide\" class is removed.\n"
			"@param {Object} e - The DOM element.",
			s->name);
		puts("\t\tthis.fillArray = function(e) {");
		TAILQ_FOREACH(f, &s->fq, entries)
			if ( ! (FIELD_NOEXPORT & f->flags) &&
			    FTYPE_STRUCT == f->type) {
				puts("\t\t\tvar list, strct, i;");
				break;
			}
		puts("\t\t\tvar o = this.obj;\n"
		     "\t\t\tvar j, row, cln;\n"
		     "\t\t\tif (null === o || null === e)\n"
		     "\t\t\t\treturn;\n"
		     "\t\t\tif ( ! (o instanceof Array)) {\n"
		     "\t\t\t\tvar ar = [];\n"
		     "\t\t\t\tar.push(o);\n"
		     "\t\t\t\to = ar;\n"
		     "\t\t\t}\n"
		     "\t\t\tif (0 === o.length) {\n"
		     "\t\t\t\t_hide(e);\n"
		     "\t\t\t\treturn;\n"
		     "\t\t\t}\n"
		     "\t\t\t_show(e);\n"
		     "\t\t\trow = e.children[0];\n"
		     "\t\t\tif (null === row)\n"
		     "\t\t\t\treturn;\n"
		     "\t\t\te.removeChild(row);\n"
		     "\t\t\twhile (null !== e.firstChild)\n"
		     "\t\t\t\te.removeChild(e.firstChild)\n"
		     "\t\t\tfor (j = 0; j < o.length; j++) {\n"
		     "\t\t\t\tcln = row.cloneNode(true);\n"
		     "\t\t\t\te.appendChild(cln);\n"
		     "\t\t\t\tthis._fill(cln, o[j], 1);\n"
		     "\t\t\t}\n"
		     "\t\t};");

		/*puts("\t\tthis.prepareForm(e) {");
		TAILQ_FOREACH(f, &s->fq, entries) {
		}*/

		puts("\t}\n"
		     "");
	}

	TAILQ_FOREACH(s, sq, entries)
		printf("\troot.%s = %s;\n", s->name, s->name);

	puts("})(this);");

}
