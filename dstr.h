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

/* Opaque dstr type */
typedef struct dstr dstr;

/* Get len of dstr data */
size_t dstr_len(const dstr *str);

/* Get capacity (allocated size of data buffer) */
size_t dstr_cap(const dstr *str);

/* Get a pointer to the beginning of the dstr's data buffer */
const char *dstr_data(const dstr *str);

/* Create a new dstr with content specified by 'data' upto length
 * 'n' and return a pointer to it. Returns NULL on failure. If data
 * is NULL, returns NULL and sets errno to EINVAL. If the requested
 * allocation size overflows, returns NULL and sets errno to
 * EOVERFLOW. On allocation failures it returns -1 and sets errno to
 * ENOMEM.
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

/* Overwrite the data of str with the provided data upto length n,
 * followed by a NUL terminator. Passing str and data with overlapping
 * memory areas results in undefined behaviour. This avoids re-allocation
 * if n fits within the current capacity of str.
 *
 * Returns -1 on failure and 0 on success.
 * If n is 0 then its a no-op and 0 is returned.
 *
 * Returns -1 and sets errno to EINVAL if any of following invalid arguments
 * are passed:
 * - NULL str
 * - NULL *str
 * - NULL data with n > 0
 *
 * If the required size calculation overflows, returns -1 and sets
 * errno to EOVERFLOW.
 * On allocation failure, it returns -1 and sets errno to ENOMEM.
 */
int dstr_set(dstr **str, const void *data, size_t n);

/* Resets the length of the provided dstr to zero and places a '\0'
 * at the beginning of the data buffer. Passing NULL str is a no-op.
 * It does not free memory previously allocated for data buffer.
 */
void dstr_reset(dstr *str);

/* Free the memory space pointed by str. If str is NULL then its a no-op.
 * Using str after is undefined behaviour.
 */
void dstr_free(dstr *str);

/* Append 'src' of 'n' bytes to '*dest' by mutating `*dest`. This
 * function is binary-safe and automatically grows `dest` if
 * needed. The source and destination memory regions must not overlap.
 *
 * Returns 0 on success and -1 on failure.
 * If n is 0 then operation is no-op and returns 0.
 *
 * Returns -1 and sets errno to EINVAL if following invalid arguments
 * are passed:
 * - NULL dest or
 * - NULL *dest or
 * - NULL src and n > 0
 *
 * If the required size calculation overflows, returns -1 and sets
 * errno to EOVERFLOW.
 * On allocation failures, it returns -1 and sets errno to ENOMEM.
 */
int dstr_cat_n(dstr **dest, const void *src, size_t n);

/* Append the contents of `src` to `dest`. Equivalent to
 * dstr_cat_n(dest, dstr_data(src), dstr_len(src)) and inherits the
 * same invalid argument and overflow handling. Returns -1 if src
 * is NULL and errno is set to EINVAL.
 */
int dstr_cat(dstr **dest, const dstr *src);

/* Append NUL-terminated c strings to `dest` dstr. Equivalent to
 * dstr_cat_n(dest, src, strlen(src)) and inherits same invalid
 * argument and overflow handling. Returns -1 if src is NULL and
 * errno is set to EINVAL.
 */
int dstr_cat_cstr(dstr **dest, const char *cstr);

/* compare at most n bytes of a and b lexicographically.
 * Returns <0, 0, or >0 if a is less than, equal to, or greater
 * than b respectively. If the compared bytes are equal and n
 * exceeds the length of either dstr, the lengths are used to
 * break the tie. If a or b is NULL, returns -1 and sets errno
 * to EINVAL.
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
 * - cap is the total number of bytes available in data[],
 * including space for the terminating NUL byte.
 * - len < cap
 * - data[len] == '\0'
 *
 * Total allocation size:
 * - sizeof(dstr) + cap bytes
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

	if (!str) {
		errno = ENOMEM;
		return NULL;
	}

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

int dstr_set(dstr **str, const void *data, size_t n)
{
	if (!str || !(*str) || (!data && n > 0)) {
		errno = EINVAL;
		return -1;
	}

	if (n == 0)
		return 0;

	if (n > SIZE_MAX - sizeof(dstr) - 1) {
		errno = EOVERFLOW;
		return -1;
	}

	size_t reqd_cap = n + 1;

	if ((*str)->cap < reqd_cap) {
		size_t curr_alloc_sz = sizeof(dstr) + (*str)->cap;
		size_t new_alloc_sz = sizeof(dstr) + reqd_cap;

		if (curr_alloc_sz <= (SIZE_MAX / 2) && curr_alloc_sz * 2 > new_alloc_sz)
			new_alloc_sz = curr_alloc_sz * 2;

		dstr *new_str = DSTR_ALLOC(*str, new_alloc_sz);

		if (!new_str) {
			errno = ENOMEM;
			return -1;
		}

		*str = new_str;
		(*str)->cap = new_alloc_sz - sizeof(dstr);
	}

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
	if (!dest || !(*dest) || (!src && n > 0)) {
		errno = EINVAL;
		return -1;
	}

	if (n == 0)
		return 0;

	// Reserve space for the header and NUL terminator before
	// validating variable-sized component to prevent underflow
	if ((*dest)->len > SIZE_MAX - sizeof(dstr) - 1) {
		errno = EOVERFLOW;
		return -1;
	}

	// len is bounded, so this subtraction cannot underflow
	// Now validate whether n fits in the remaining space.
	if (n > SIZE_MAX - sizeof(dstr) - (*dest)->len - 1) {
		errno = EOVERFLOW;
		return -1;
	}

	size_t reqd_cap = (*dest)->len + n + 1;

	if ((*dest)->cap < reqd_cap) {
		size_t curr_alloc_sz = sizeof(dstr) + (*dest)->cap;
		size_t new_alloc_sz = sizeof(dstr) + reqd_cap;

		if (curr_alloc_sz <= (SIZE_MAX / 2) && curr_alloc_sz * 2 > new_alloc_sz)
			new_alloc_sz = curr_alloc_sz * 2;

		dstr *new_str = DSTR_ALLOC(*dest, new_alloc_sz);

		if (!new_str) {
			errno = ENOMEM;
			return -1;
		}

		*dest = new_str;
		(*dest)->cap = new_alloc_sz - sizeof(dstr);
	}

	memcpy((*dest)->data + (*dest)->len, src, n);
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

int dstr_cmp_n(const dstr *a, const dstr *b, size_t n)
{
	if (!a || !b) {
		errno = EINVAL;
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
