namespace ortPriv {
	/**
	 * Labels ("jslabel" in ort(5)) may have multiple languages.  This 
	 * maps a language name to a translated string.
	 */
	interface langmap { [lang: string]: string };

	/**
	 * Resolve a set of translated strings into a single one depending 
	 * upon the current language.
	 * @param vals All translations of a given word.
	 * @return The word in the current language (or the default) or an 
	 * emptry string on failure.
	 */
	function _strlang(vals: langmap): string
	{
		const lang: string|null = document.documentElement.lang;
		if (lang !== null && lang in vals)
			return vals[lang];
		else if ('_default' in vals)
			return vals['_default'];
		else
			return '';
	}

	/**
	 * Language replacement conditional upon the label (**jslabel** in 
	 * the configuration).  Like {@link _replcl} with inclusion set to 
	 * false.
	 * @param e The root of the DOM tree in which we query for elements 
	 * to fill into.
	 * @param name The class name we search for within the root (not 
	 * inclusive).
	 * @param vals All possible translations.
	 */
	function _replcllang(e: HTMLElement, name:string, vals: langmap): void
	{
		_replcl(e, name, _strlang(vals), false);
	}

	/**
	 * Language replacement conditional upon the label (**jslabel** in 
	 * the configuration). Like {@link _repl}.
	 * @param e The root of the DOM tree in which we query for elements 
	 * to fill into.
	 * @param vals All possible translations.
	 */
	function _repllang(e: HTMLElement, vals: langmap): void
	{
		_repl(e, _strlang(vals));
	}

	/**
	 * Set the attribute of an element.
	 * @param e The element whose attribute to set.
	 * @param attr The attribute name.
	 * @param text The attribute value.
	 */
	function _attr(e: HTMLElement, attr: string, text: string): void
	{
		e.setAttribute(attr, text);
	}

	/**
	 * Remove the attribute of an element.
	 * @param e The element whose attribute to set.
	 * @param attr The attribute name.
	 * @param text The attribute value.
	 */
	function _rattr(e: HTMLElement, attr: string): void
	{
		e.removeAttribute(attr);
	}

	/**
	 * Check inputs for all elements of class `strct-name-value-checked` 
	 * whose value matches the object's value. If the object is null, all 
	 * elements are unchecked.
	 * @param e The root of the DOM tree in which we query for elements 
	 * to fill into.
	 * @param strct The name of the structure we're filling in.
	 * @param name The name of the field.
	 * @param obj The data itself.
	 * @param inc Whether to include the root element in looking for 
	 * elements to fill.
	 */
	function _fillValueChecked(e: HTMLElement, fname: string,
			val: number|string|null, inc: boolean): void
	{
		let i: number;
		const list: HTMLElement[] = 
			_elemList(e, fname + '-value-checked', inc);
		const valstr: string|null = (val === null) ? null : 
			(typeof val === 'number' ? val.toString() : val);
		for (i = 0; i < list.length; i++)
			if (valstr === null)
				_rattr(list[i], 'checked');
			else if (valstr === (<HTMLInputElement>list[i]).value)
				_attr(list[i], 'checked', 'true');
			else
				_rattr(list[i], 'checked');
	}

	/**
	 * Take all `<option>` elements in the root and sets or unsets their 
	 * `selected` attribute depending upon whether it matches the 
	 * object's value.
	 * @param e The root of the DOM tree in which we query for elements 
	 * to fill into.
	 * @param val The object's value.
	 */
	function _fillValueSelect(e: HTMLElement, val: number|string): void
	{
		const list: HTMLCollectionOf<Element> = 
			e.getElementsByTagName('option');
		let i: number;
		let v: string|number;
		for (i = 0; i < list.length; i++) {
			v = typeof val === 'number' ? 
			     parseInt((<HTMLOptionElement>list[i]).value) :
			     (<HTMLOptionElement>list[i]).value;
			if (val === v)
				_attr(<HTMLOptionElement>list[i], 'selected', 'true');
			else
				_rattr(<HTMLOptionElement>list[i], 'selected');
		}
	}

	function _attrcl(e: HTMLElement, attr: string,
		name: string, text: string, inc: boolean): void
	{
		const list: HTMLElement[] =
			_elemList(e, name, inc);
		let i: number;
		for (i = 0; i < list.length; i++)
			_attr(list[i], attr, text);
	}

	function _elemList(e: HTMLElement|null,
		cls: string, inc: boolean): HTMLElement[]
	{
		const a: HTMLElement[] = [];
		let list: HTMLCollectionOf<Element>;
		let i: number;
		if (e === null)
			return a;
		list = e.getElementsByClassName(cls);
		for (i = 0; i < list.length; i++)
			a.push(<HTMLElement>list[i]);
		if (inc && e.classList.contains(cls))
			a.push(e);
		return a;
	}

	function _repl(e: HTMLElement, text: string): void
	{
		while (e.firstChild)
			e.removeChild(e.firstChild);
		e.appendChild(document.createTextNode(text));
	}

	/**
	 * Fill in ISO-8601 dates.
	 * @param e Root of the DOM tree in which we query for elements to 
	 * fill into.
	 * @param strct Name of the structure filled into.
	 * @param name Name of the field.
	 * @param val Epoch date itself.
	 * @param inc Whether to include the root element in looking for 
	 * elements to fill.
	 */
	function _fillDateValue(e: HTMLElement, strct: string,
		name: string, val: number|null, inc: boolean): void
	{
		let fname: string;
		let year: number;
		let mo: number;
		let day: number;
		let full: string;
		const d: Date = new Date();
		if (val === null)
			return;
		d.setTime(val * 1000);
		year = d.getFullYear();
		mo = d.getMonth() + 1;
		day = d.getDate();
		full = year + '-' +
			(mo < 10 ? '0' : '') + mo + '-' +
			(day < 10 ? '0' : '') + day;
		fname = strct + '-' + name + '-date-value';
		_attrcl(e, 'value', fname, full, inc);
		fname = strct + '-' + name + '-date-text';
		_replcl(e, fname, full, inc);
	}

	/**
	 * Check inputs for all elements of class `strct-name-bits-checked` 
	 * whose value is the bit-wise AND of the object's value.  If the 
	 * object is null, all elements are unchecked.
	 * @param e Root of the DOM tree filled into.
	 * @param strct Name of the structure filled.
	 * @param name Name of the field.
	 * @param val Bit-field itself.
	 * @param inc Whether to include the root element in looking for 
	 * elements to fill.
	 */
	function _fillBitsChecked(e: HTMLElement, strct: string,
		name: string, val: number|null, inc: boolean): void
	{
		const list: HTMLElement[] = _elemList
			(e, strct + '-' + name + '-bits-checked', inc);
		let i: number;
		let v: number;
		for (i = 0; i < list.length; i++) {
			if (val === null) {
				_rattr(list[i], 'checked');
				continue;
			}
			v = parseInt((<HTMLInputElement>list[i]).value);
			if (isNaN(v))
				_rattr(list[i], 'checked');
			else if (0 === v && 0 === val)
				_attr(list[i], 'checked', 'true');
			else if ((1 << (v - 1)) & val)
				_attr(list[i], 'checked', 'true');
			else
				_rattr(list[i], 'checked');
		}
	}

	/**
	 * Fill a structure field.  This first does the has/no class setting 
	 * for null values, then optionally returns if null (running the 
	 * custom fields first), otherwise the generic text/value/etc fields, 
	 * then finally the custom fields.
	 * @param e Root of the DOM tree filled into.
	 * @param strct Name of the structure filling in.
	 * @param name The name of the field.
	 * @param custom Custom callback functions.
	 * @param obj The data itself.
	 * @param inc Whether to include the root element in looking for 
	 * elements to fill. Nested structures are always filled 
	 * non-inclusively.
	 * @param cannull Whether the data may be null.
	 * @param isblob Whether the data is a blob.
	 * @param sub If the data object is a nested structure interface, the 
	 * allocated class of that interface.
	 */
	function _fillField(e: HTMLElement, strct: string, name: string,
		custom: DataCallbacks|null, obj: any, inc: boolean,
		cannull: boolean, isblob: boolean, sub: any): void
	{
		let i: number;
		const fname: string = strct + '-' + name;
		let list: HTMLElement[];
		/* First handle our has/no null situation. */
		if (cannull) {
			if (obj === null) {
				_hidecl(e, strct + '-has-' + name, inc);
				_showcl(e, strct + '-no-' + name, inc);
			} else {
				_showcl(e, strct + '-has-' + name, inc);
				_hidecl(e, strct + '-no-' + name, inc);
			}
		}
		/* Don't process null values that can be null. */
		if (cannull && null === obj) {
			if (null !== custom && fname in custom) {
				if (custom[fname] instanceof Array) {
					for (i = 0; i < custom[fname].length; i++)
						custom[fname][i](e, fname, null);
				} else {
					custom[fname](e, fname, null);
				}
			}
			return;
		}
		/* Non-null non-structs. */
		/* Don't account for blobs. */
		if (null !== sub) {
			list = _elemList(e, fname + '-obj', inc);
			for (i = 0; i < list.length; i++) {
				sub.fillInner(list[i], custom);
			}
		} else if ( ! isblob) {
			list = _elemList(e, fname + '-enum-select', inc);
			for (i = 0; i < list.length; i++) {
				_fillValueSelect(list[i], obj);
			}
			_replcl(e, fname + '-text', obj, inc);
			_attrcl(e, 'value', fname + '-value', obj, inc);
			_fillValueChecked(e, fname, obj, inc);
		}
		/* Lastly, handle the custom callback. */
		if (null !== custom && fname in custom) {
			if (custom[fname] instanceof Array) {
				for (i = 0; i < custom[fname].length; i++)
					custom[fname][i](e, fname, obj);
			} else {
				custom[fname](e, fname, obj);
			}
		}
	}

	function _replcl(e: HTMLElement, name: string, text: string, inc: boolean): void
	{
		let list: HTMLElement[];
		let i: number;
		if (null === e)
			return;
		list = _elemList(e, name, inc);
		for (i = 0; i < list.length; i++)
			_repl(list[i], text);
	}

	function _classadd(e: HTMLElement, name: string): HTMLElement
	{
		if ( ! e.classList.contains(name))
			e.classList.add(name);
		return(e);
	}

	function _classaddcl(e: HTMLElement, name: string, cls: string, inc: boolean): void
	{
		let list: HTMLElement[];
		let i: number;
		list = _elemList(e, name, inc);
		for (i = 0; i < list.length; i++)
			_classadd(list[i], cls);
	}

	function _hide(e: HTMLElement): HTMLElement
	{
		if ( ! e.classList.contains('hide'))
			e.classList.add('hide');
		return e;
	}

	function _hidecl(e: HTMLElement, name: string, inc: boolean): void
	{
		let list: HTMLElement[];
		let i: number;
		list = _elemList(e, name, inc);
		for (i = 0; i < list.length; i++)
			_hide(list[i]);
	}

	function _show(e: HTMLElement): HTMLElement
	{
		if (e.classList.contains('hide'))
			e.classList.remove('hide');
		return e;
	}

	function _showcl(e: HTMLElement, name: string, inc: boolean): void
	{
		let list: HTMLElement[];
		let i: number;
		list = _elemList(e, name, inc);
		for (i = 0; i < list.length; i++)
			_show(list[i]);
	}
}
