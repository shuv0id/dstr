#ifndef DSTR_H
#define DSTR_H
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#if defined(DSTR_ALLOC) && !defined(DSTR_FREE) || !defined(DSTR_ALLOC) && defined(DSTR_FREE)
#error "You must define both DSTR_ALLOC and DSTR_FREE, or neither"
#endif
#if !defined(DSTR_ALLOC) && !defined(DSTR_FREE)
#include <stdlib.h>
#define DSTR_ALLOC(p, s) realloc(p, s)
#define DSTR_FREE(p) free(p)
#endif

/*
 * Invariants:
 *
 * - cap is the total number of bytes available in data[],
 * including space for the terminating NUL byte.
 * - len < cap
 * - data[len] == '\0'
 *
 * Total allocation size:
 * - sizeof(dstr) + cap bytes
 *
 */
typedef struct dstr {
	size_t len;
	size_t cap;
	char data[];
} dstr;

/* Create a new dstr with content specified by 'data' upto length
 * 'n' and return a pointer to it. Returns NULL on failure. If data
 * is NULL, returns NULL and sets errno to EINVAL. If the requested
 * allocation size overflows, returns NULL and sets errno to
 * EOVERFLOW.
 */
dstr *dstr_new_n(const void *data, size_t n);

/* Returns pointer to a new dstr with data holding a '\0' character
 * and zero length. Equivalent to dstr_new_n("", 0).
 */
dstr *dstr_empty(void);

/* Create a new dstr with NUL-terminated c string and return a pointer
 * to it. It is equivalent to dstr_new_n(c_str, strlen(c_str)). Returns
 * NULL if cstr is NULL.
 */
dstr *dstr_new(const char *cstr);

/* Returns a new dstr containing a deep copy of the contents of `s`.
 * The returned dstr has the same length and contents as `s`. Returns
 * NULL and sets errno to EINVAL if s is NULL.
 */
dstr *dstr_dup(const dstr *s);

/* Free the memory space pointed by str. If str is NULL then its a no-op.
 * Using str after is undefined behaviour.
 */
void dstr_free(dstr *str);

/* Append 'src' of 'n' bytes to 'dest' by mutating `dest`. This
 * function is binary-safe and automatically grows `dest` if
 * needed. src and *dest can be equal.
 * Returns 0 on success and -1 on failure.
 * If n is 0 then operation is no-op and returns 0.
 * Returns -1 and sets errno to EINVAL if following invalid
 * arguments are passed:
 * - NULL dest or
 * - NULL *dest or
 * - NULL src and n > 0
 * This operation can fail if requested size of bytes could not be
 * appended to dest in which case -1 is returned and errno is set
 * to EOVERFLOW.
 */
int dstr_cat_n(dstr **dest, const void *src, size_t n);

/* Append the contents of `src` to `dest`. Equivalent to
 * dstr_cat_n(dest, src->data, src->len) and inherits same invalid
 * argument and overflow handling.
 * Returns -1 if src is NULL and errno is set to
 * EINVAL.
 */
int dstr_cat(dstr **dest, const dstr *src);

/* Append NUL-terminated c strings to `dest` dstr. Equivalent to
 * dstr_cat_n(dest, src, strlen(src)) and inherits same invalid
 * argument and overflow handling. Returns -1 if src is NULL and
 * errno is set to EINVAL.
 */
int dstr_cat_cstr(dstr **dest, const char *cstr);

#endif // DSTR_H

#ifdef DSTR_IMPLEMENTATION

dstr *dstr_new_n(const void *data, size_t n)
{
	if (!data) {
		errno = EINVAL;
		return NULL;
	}

	if (n > SIZE_MAX - sizeof(dstr) - 1) {
		errno = EOVERFLOW;
		return NULL;
	}

	size_t cap = n + 1;

	dstr *str = DSTR_ALLOC(NULL, sizeof(dstr) + cap);

	if (!str)
		return NULL;

	str->len = n;
	str->cap = cap;
	memcpy(str->data, data, n);
	str->data[str->len] = '\0';

	return str;
}

dstr *dstr_empty(void)
{
	return dstr_new_n("", 0);
}

dstr *dstr_new(const char *c_str)
{
	if (!c_str) {
		errno = EINVAL;
		return NULL;
	}

	return dstr_new_n(c_str, strlen(c_str));
}

dstr *dstr_dup(const dstr *s)
{
	if (!s) {
		errno = EINVAL;
		return NULL;
	}

	return dstr_new_n(s->data, s->len);
}

void dstr_free(dstr *str)
{
	if (!str)
		return;
	DSTR_FREE(str);
}

int dstr_cat_n(dstr **dest, const void *src, size_t n)
{
	if (!dest || !(*dest) || (!src && n > 0)) {
		errno = EINVAL;
		return -1;
	}

	if (n == 0)
		return 0;

	if (n > SIZE_MAX - (*dest)->len - 1) {
		errno = EOVERFLOW;
		return -1;
	}

	size_t new_sz = (*dest)->len + n + 1;

	if ((*dest)->cap < new_sz - sizeof(dstr)) {
		size_t curr_sz = sizeof(dstr) + (*dest)->cap;
		if (curr_sz <= (SIZE_MAX / 2) && curr_sz * 2 > new_sz)
			new_sz = curr_sz * 2;

		dstr *old = *dest;
		dstr *new_str = DSTR_ALLOC(*dest, new_sz);

		if (!new_str) {
			return -1;
		}

		// Reallocation can make the previous pointer invalid
		*dest = new_str;
		if (old == src) {
			// Incase provided dest and src were equal we also
			// need to change src to the pointer to new memory
			// location
			src = new_str;
		}

		(*dest)->cap = new_sz - sizeof(dstr);
	}

	memmove((*dest)->data + (*dest)->len, src, n);
	(*dest)->len += n;
	(*dest)->data[(*dest)->len] = '\0';

	return 0;
}

int dstr_cat(dstr **dest, const dstr *src)
{
	if (!src) {
		errno = EINVAL;
		return -1;
	}
	return dstr_cat_n(dest, src->data, src->len);
}

int dstr_cat_cstr(dstr **dest, const char *src)
{
	if (!src) {
		errno = EINVAL;
		return -1;
	}
	return dstr_cat_n(dest, src, strlen(src));
}

#endif // DSTR_IMPLEMENTATION
