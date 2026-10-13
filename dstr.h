#ifndef DSTR_H
#define DSTR_H
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#if defined(DSTR_ALLOC) && !defined(DSTR_FREE) || !defined(DSTR_ALLOC) && defined(DSTR_FREE)
#error "You must define both DSTR_ALLOC and DSTR_FREE, or neither"
#endif
#if !defined(DSTR_ALLOC) && !defined(DSTR_FREE)
#include <stdlib.h>
#define DSTR_ALLOC(p, s) realloc(p, s)
#define DSTR_FREE(p) free(p)
#endif

/* Opaque dstr type. A dstr can store binary data. Its data is always
 * NUL-terminated. */
typedef struct dstr dstr;

/* Get len of dstr data */
size_t dstr_len(const dstr *str);

/* Get number of bytes available to store string data. It is
 * greater than or equal to the length of the string. It does
 * not include space for the terminating NUL character. */
size_t dstr_cap(const dstr *str);

/* Get a pointer to the beginning of the dstr's data buffer */
const char *dstr_data(const dstr *str);

/* Create a new dstr with content specified by 'data' upto length
 * 'n' and return a pointer to it. Returns NULL on failure.
 */
dstr *dstr_new_n(const void *data, size_t n);

/* Returns pointer to a new dstr with a '\0' character at the beginning
 * of its data with a length of zero and a capacity of `cap`.
 */
dstr *dstr_empty(size_t cap);

/* Create a new dstr with NUL-terminated c string and return a pointer
 * to it. It is equivalent to dstr_new_n(c_str, strlen(c_str)). Returns
 * NULL on failure.
 */
dstr *dstr_new(const char *cstr);

/* Returns a new dstr containing a deep copy of the contents of `s`.
 * The returned dstr has the same length and contents as `s`. Returns
 * NULL on failure.
 */
dstr *dstr_dup(const dstr *s);

/* Overwrite the data of str with the provided data upto length n,
 * immediately followed by a '\0' character. The memory regions
 * containing the provided data and string data must not overlap.
 * This avoids re-allocation if n fits within the current capacity
 * of str.
 *
 * Returns -1 on failure and 0 on success.
 * If n is 0 then its a no-op and 0 is returned.
 */
int dstr_set_n(dstr **str, const void *data, size_t n);

/* Resets the length of the provided dstr to zero and places a '\0'
 * at the beginning of the data buffer. Passing NULL str is a no-op.
 * It does not free memory previously allocated for data buffer.
 */
void dstr_reset(dstr *str);

/* Free `str`. If str is NULL then its a no-op. The dstr is invalid
 * after being freed.
 */
void dstr_free(dstr *str);

/* Append 'src' of 'n' bytes to '*dest' by mutating `*dest`. This
 * function is binary-safe and automatically grows `dest` if
 * needed. The source and destination memory regions must not overlap.
 *
 * Returns 0 on success and -1 on failure.
 * If n is 0 then operation is no-op and returns 0.
 */
int dstr_cat_n(dstr **dest, const void *src, size_t n);

/* Append the contents of `src` to `dest`. Equivalent to
 * dstr_cat_n(dest, dstr_data(src), dstr_len(src)). Returns 0 on
 * success and -1 on failure.
 */
int dstr_cat(dstr **dest, const dstr *src);

/* Append NUL-terminated c strings to `dest` dstr. Equivalent to
 * dstr_cat_n(dest, src, strlen(src)). Returns 0 on success and -1 on
 * failure.
 */
int dstr_cat_cstr(dstr **dest, const char *cstr);

/* Compare at most n bytes of a and b lexicographically.
 * Returns <0, 0, or >0 if a is less than, equal to, or greater
 * than b respectively. If the compared bytes are equal and n
 * exceeds the length of either dstr, the lengths are used to
 * break the tie.
 */
int dstr_cmp_n(const dstr *a, const dstr *b, size_t n);

/* Returns true if `a` and `b` have the same length and contents.
 * Returns false if either is NULL or if they differ.
 */
bool dstr_eq(const dstr *a, const dstr *b);

/* Returns true if `a` and `b` have the same length and contents
 * ignoring ASCII case. Returns false if either is null or if they
 * differ.
 */
bool dstr_eq_ignorecase(const dstr *a, const dstr *b);

#endif // DSTR_H

#ifdef DSTR_IMPLEMENTATION

/*
 * Invariants:
 *
 * - cap is the number of bytes available for string data, excluding
 *   the NUL terminator
 * - len <= cap
 * - data[len] == '\0'
 *
 * Total allocation size:
 * - sizeof(dstr) + cap + 1(for NUL terminator) bytes
 *
 */
struct dstr {
	size_t len;
	size_t cap;
	char data[];
};

size_t dstr_len(const dstr *str)
{
	return str->len;
}

size_t dstr_cap(const dstr *str)
{
	return str->cap;
}

const char *dstr_data(const dstr *str)
{
	return str->data;
}

/* Helper function which returns total allocation size with provided
 * capacity. Returns 0 if overflows. */
static size_t dstr_alloc_sz(size_t cap)
{
	return cap > (SIZE_MAX - sizeof(dstr) - 1) ?
		0 : sizeof(dstr) + cap + 1;
}

/* Helper function which returns a pointer to a newly allocated dstr with
 * provided capacity, length of zero and '\0' character at the beginning
 * of the data buffer. Returns NULL on failure. */
static dstr *dstr_alloc(size_t cap)
{
	size_t sz = dstr_alloc_sz(cap);
	if (sz == 0) return NULL;

	dstr *new = DSTR_ALLOC(NULL, sz);
	if (!new) return NULL;

	new->cap = cap;
	new->len = 0;
	new->data[0] = '\0';

	return new;
}

/* Helper function to reallocate `*str` to grow its buffer upto a minimum
 * capacity of provided `need` if `need` is greater than current capacity
 * of `*str`. Returns 0 on success and -1 on failure. It assumes `str` and
 * `*str` is not NULL. */
int dstr_reserve(dstr **str, size_t need)
{
	if (need <= (*str)->cap) return 0;

	size_t new_cap = (*str)->cap > SIZE_MAX / 2 ?
						need : (*str)->cap * 2;
	if (new_cap < need) new_cap = need;

	size_t sz = dstr_alloc_sz(new_cap);
	if (sz == 0) return -1;

	dstr *tmp = DSTR_ALLOC(*str, sz);
	if (!tmp) return -1;

	*str = tmp;
	(*str)->cap = new_cap;
	return 0;
}

/* Helper function to reallocate `*str` to grow its buffer upto a minimum
 * capacity of current length of `*str plus additional `extra` bytes if
 * current capacity cannot hold additional `extra` bytes. Returns 0 on
 * success and -1 on failure. It assumes  `str` and  `*str` is not NULL. */
int dstr_reserve_extra(dstr **str, size_t extra)
{
	if (extra == 0) return 0;

	// Ensure there is remaining space for more data
	if ((*str)->len >= SIZE_MAX - sizeof(dstr) - 1)
		return -1;

	// Now validate whether n fits in the remaining space
	if (extra > SIZE_MAX - sizeof(dstr) - (*str)->len - 1)
		return -1;

	return dstr_reserve(str, (*str)->len + extra);
}

dstr *dstr_new_n(const void *data, size_t n)
{
	if (!data) return NULL;

	dstr *str = dstr_alloc(n);

	if (!str) return NULL;

	str->len = n;
	memcpy(str->data, data, n);
	str->data[str->len] = '\0';

	return str;
}

dstr *dstr_empty(size_t cap)
{
	return dstr_alloc(cap);
}

dstr *dstr_new(const char *c_str)
{
	if (!c_str) {
		return NULL;
	}

	return dstr_new_n(c_str, strlen(c_str));
}

dstr *dstr_dup(const dstr *s)
{
	if (!s) {
		return NULL;
	}

	return dstr_new_n(s->data, s->len);
}

int dstr_set_n(dstr **str, const void *data, size_t n)
{
	if (!str || !(*str) || (!data && n > 0))
		return -1;

	if (dstr_reserve(str, n) == -1) return -1;

	memcpy((*str)->data, data, n);
	(*str)->len = n;
	(*str)->data[n] = '\0';

	return 0;
}

void dstr_reset(dstr *str)
{
	if (!str)
		return;

	str->len = 0;
	str->data[0] = '\0';
}

void dstr_free(dstr *str)
{
	if (!str)
		return;
	DSTR_FREE(str);
}

int dstr_cat_n(dstr **dest, const void *src, size_t n)
{
	if (!dest || !(*dest) || (!src && n > 0))
		return -1;

	if (dstr_reserve_extra(dest, n) == -1)
		return -1;

	memcpy((*dest)->data + (*dest)->len, src, n);
	(*dest)->len += n;
	(*dest)->data[(*dest)->len] = '\0';

	return 0;
}

int dstr_cat(dstr **dest, const dstr *src)
{
	if (!src) {
		return -1;
	}
	return dstr_cat_n(dest, src->data, src->len);
}

int dstr_cat_cstr(dstr **dest, const char *src)
{
	if (!src) {
		return -1;
	}
	return dstr_cat_n(dest, src, strlen(src));
}

int dstr_cmp_n(const dstr *a, const dstr *b, size_t n)
{
	if (!a || !b) {
		return -1;
	}

	size_t common = a->len < b->len ? a->len : b->len;
	if (common > n)
		common = n;

	int cmp = memcmp(a->data, b->data, common);

	if (cmp != 0)
		return cmp;

	if (common == n)
		return 0;

	return (a->len > b->len) - (a->len < b->len);
}

bool dstr_eq(const dstr *a, const dstr *b)
{
	if (!a || !b)
		return false;

	if (a->len != b->len)
		return false;

	return memcmp(a->data, b->data, a->len) == 0;
}

bool dstr_eq_ignorecase(const dstr *a, const dstr *b)
{
	if (!a || !b)
		return false;

	if (a->len != b->len)
		return false;

	for (size_t i = 0; i < a->len; i++) {
		char ca = a->data[i];
		char cb = b->data[i];

		if (ca >= 'A' && ca <= 'Z')
			ca += 'a' - 'A';

		if (cb >= 'A' && cb <= 'Z')
			cb += 'a' - 'A';

		if (ca != cb)
			return false;
	}

	return true;
}

#endif // DSTR_IMPLEMENTATION
