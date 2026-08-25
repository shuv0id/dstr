// test_dstr.c

#include <stdio.h>
#include <string.h>
#include <errno.h>

#define DSTR_IMPLEMENTATION
#include "dstr.h"

#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define BLUE "\x1b[34m"
#define RESET "\x1b[0m"

const char *curr_test = NULL;

void assert_equal_size_t(const char *file, const size_t line, const char *expected_expr,
						 size_t expected, const char *actual_expr, size_t actual)
{
	if (!(expected == actual)) {
		fprintf(stderr,
				RED "[FAIL]" RESET " %s"
					"\n",
				curr_test);
		fprintf(stderr,
				"%s:%zu: "
				"%s == %s\n",
				file, line, expected_expr, actual_expr);
		fprintf(stderr, BLUE "  EXPECTED: " RESET "%zu\n", expected);
		fprintf(stderr, BLUE "  ACTUAL: " RESET "%zu\n", actual);
		exit(1);
	}
}

void assert_equal_int(const char *file, const size_t line, const char *expected_expr, int expected,
					  const char *actual_expr, int actual)
{
	if (!(expected == actual)) {
		fprintf(stderr,
				RED "[FAIL]" RESET " %s"
					"\n",
				curr_test);
		fprintf(stderr,
				"%s:%zu: "
				"%s == %s\n",
				file, line, expected_expr, actual_expr);
		fprintf(stderr, BLUE "  EXPECTED: " RESET "%d\n", expected);
		fprintf(stderr, BLUE "  ACTUAL: " RESET "%d\n", actual);
		exit(1);
	}
}

void assert_equal_char(const char *file, const size_t line, const char *expected_expr,
					   char expected, const char *actual_expr, char actual)
{
	if (!(expected == actual)) {
		fprintf(stderr, "[FAILED] %s\n\n", curr_test);
		fprintf(stderr,
				"%s:%zu: "
				"%s == %s",
				file, line, expected_expr, actual_expr);
		fprintf(stderr, BLUE "  EXPECTED: " RESET "%c\n", expected);
		fprintf(stderr, BLUE "  ACTUAL: " RESET "%c\n", actual);
		exit(1);
	}
}

#define ASSERT_EQUAL(type, expected, actual)                                                       \
	assert_equal_##type(__FILE__, __LINE__, #expected, expected, #actual, actual)

void assert_true(const char *file, const size_t line, const char *expr, bool expression)
{
	if (!expression) {
		fprintf(stderr,
				RED "[FAIL]" RESET " %s"
					"\n",
				curr_test);
		fprintf(stderr, "%s:%zu: " BLUE "%s\n" RESET, file, line, expr);
		exit(1);
	}
}

#define ASSERT_TRUE(expression) assert_true(__FILE__, __LINE__, #expression, expression)

void assert_false(const char *file, const size_t line, const char *expr, bool expression)
{
	if (expression) {
		fprintf(stderr,
				RED "[FAIL]" RESET " %s"
					"\n",
				curr_test);
		fprintf(stderr, "%s:%zu: " BLUE "%s\n" RESET, file, line, expr);
		exit(1);
	}
}

#define ASSERT_FALSE(expression) assert_false(__FILE__, __LINE__, #expression, expression)

void test_dstr_new_n_normal(void)
{
	char *greet = "Hello";

	dstr *str = dstr_new_n(greet, 5);

	ASSERT_EQUAL(size_t, strlen(greet), str->len);
	ASSERT_TRUE(str->cap > 5);
	ASSERT_TRUE(memcmp(str->data, greet, str->len) == 0);
	ASSERT_EQUAL(char, '\0', str->data[str->len]);

	dstr_free(str);
}

void test_dstr_new_n_null(void)
{
	char *buf = NULL;

	errno = 0;
	dstr *str = dstr_new_n(buf, 5);

	ASSERT_TRUE(str == NULL);
	ASSERT_EQUAL(int, EINVAL, errno);
}

void test_dstr_new_n_copies_input(void)
{
	char greet[] = "Hello";

	dstr *str = dstr_new_n(greet, 5);

	greet[0] = 'X';

	ASSERT_TRUE(memcmp(str->data, "Hello", str->len) == 0);

	dstr_free(str);
}

void test_dstr_new_n_binary_data(void)
{
	char buf[] = {'A', 'B', '\0', 'D'};

	dstr *str = dstr_new_n(buf, sizeof(buf));

	ASSERT_EQUAL(size_t, sizeof(buf), str->len);
	ASSERT_TRUE(str->cap > 4);
	ASSERT_TRUE(memcmp(str->data, buf, str->len) == 0);
	ASSERT_EQUAL(char, '\0', str->data[str->len]);

	dstr_free(str);
}

void test_dstr_empty(void)
{
	dstr *str = dstr_empty();

	ASSERT_EQUAL(size_t, 0, str->len);
	ASSERT_TRUE(str->cap > 0);
	ASSERT_TRUE(str->data[0] == '\0');

	dstr_free(str);
}

void test_dstr_new_null_cstr(void)
{
	char *c_str = NULL;

	errno = 0;
	dstr *str = dstr_new(c_str);

	ASSERT_TRUE(str == NULL);
	ASSERT_EQUAL(int, EINVAL, errno);

	dstr_free(str);
}

void test_dstr_new_stops_at_null(void)
{
	const char *hello = "Hello\0World";

	dstr *str = dstr_new(hello);

	ASSERT_EQUAL(size_t, 5, str->len);
	ASSERT_TRUE(str->cap > 5);
	ASSERT_TRUE(strcmp(hello, str->data) == 0);
	ASSERT_EQUAL(char, '\0', str->data[str->len]);

	dstr_free(str);
}

void test_dstr_dup_normal(void)
{
	errno = 0;
	dstr *dup = dstr_dup(NULL);

	ASSERT_TRUE(dup == NULL);
	ASSERT_EQUAL(int, EINVAL, errno);

	dstr_free(dup);
}

void test_dstr_dup_null(void)
{
	dstr *original = dstr_new("tip");
	dstr *dup = dstr_dup(original);

	// Test-only: mutate the internal buffer to verify a deep copy.
	original->data[1] = 'a';

	ASSERT_EQUAL(size_t, dup->len, original->len);
	ASSERT_TRUE(original->data != dup->data);
	ASSERT_FALSE(memcmp(original->data, dup->data, original->len) == 0);

	dstr_free(original);
	dstr_free(dup);
}

void test_dstr_cat_n_empty_dstr(void)
{
	char *greet = "helloworld";
	dstr *dest = dstr_empty();

	int done = dstr_cat_n(&dest, greet, 10);

	ASSERT_EQUAL(int, 0, done);
	ASSERT_EQUAL(size_t, 10, dest->len);
	ASSERT_TRUE(dest->cap > 10);
	ASSERT_TRUE(memcmp(dest->data, greet, dest->len) == 0);

	dstr_free(dest);
}

void test_dstr_cat_n_null_dest_arg(void)
{
	errno = 0;
	int done = dstr_cat_n(NULL, NULL, 1);

	ASSERT_EQUAL(int, -1, done);
	ASSERT_EQUAL(int, EINVAL, errno);
}

void test_dstr_cat_n_null_dest(void)
{
	errno = 0;
	dstr *dest = NULL;
	int done = dstr_cat_n(&dest, NULL, 1);

	ASSERT_EQUAL(int, -1, done);
	ASSERT_EQUAL(int, EINVAL, errno);
}

void test_dstr_cat_n_null_src(void)
{
	dstr *hello = dstr_new("hello");
	const size_t prev_len = hello->len;
	const size_t prev_cap = hello->cap;

	errno = 0;
	int done = dstr_cat_n(&hello, NULL, 1);

	ASSERT_EQUAL(int, -1, done);
	ASSERT_EQUAL(int, EINVAL, errno);
	ASSERT_EQUAL(size_t, prev_len, hello->len);
	ASSERT_EQUAL(size_t, prev_cap, hello->cap);
	ASSERT_TRUE(memcmp(hello->data, "hello", hello->len) == 0);

	dstr_free(hello);
}

void test_dstr_cat_n_src_with_null_byte(void)
{
	char *greet = "\0world";
	dstr *dest = dstr_new_n("hello", 5);

	int ok = dstr_cat_n(&dest, greet, 6);

	ASSERT_EQUAL(int, 0, ok);
	ASSERT_EQUAL(size_t, 11, dest->len);
	ASSERT_TRUE(dest->cap > 11);
	ASSERT_TRUE(memcmp(dest->data, "hello\0world", dest->len) == 0);

	dstr_free(dest);
}

void test_dstr_cat_normal(void)
{
	dstr *hello = dstr_new_n("hello", 5);
	dstr *world = dstr_new_n("\0world", 6);

	int done = dstr_cat(&hello, world);
	ASSERT_EQUAL(int, 0, done);
	ASSERT_EQUAL(size_t, 11, hello->len);
	ASSERT_TRUE(hello->cap > 11);
	ASSERT_TRUE(memcmp(hello->data, "hello\0world", hello->len) == 0);

	dstr_free(hello);
	dstr_free(world);
}

void test_dstr_cat_null_src(void)
{
	dstr *hello = dstr_new("hello");
	const size_t prev_len = hello->len;
	const size_t prev_cap = hello->cap;

	errno = 0;
	int done = dstr_cat(&hello, NULL);

	ASSERT_EQUAL(int, -1, done);
	ASSERT_EQUAL(int, EINVAL, errno);
	ASSERT_EQUAL(size_t, prev_len, hello->len);
	ASSERT_EQUAL(size_t, prev_cap, hello->cap);
	ASSERT_TRUE(memcmp(hello->data, "hello", hello->len) == 0);

	dstr_free(hello);
}

void test_dstr_cat_cstr_normal(void)
{
	char *world = "world";

	dstr *hello = dstr_new("hello");
	const size_t prev_len = hello->len;

	int done = dstr_cat_cstr(&hello, world);

	ASSERT_EQUAL(int, 0, done);
	ASSERT_EQUAL(size_t, prev_len + strlen(world), hello->len);
	ASSERT_TRUE(hello->cap > 10);
	ASSERT_TRUE(memcmp(hello->data, "helloworld", hello->len) == 0);

	dstr_free(hello);
}

void test_dstr_cat_cstr_null_src(void)
{
	dstr *hello = dstr_new("hello");
	const size_t prev_len = hello->len;
	const size_t prev_cap = hello->cap;

	errno = 0;
	int done = dstr_cat_cstr(&hello, NULL);

	ASSERT_EQUAL(int, -1, done);
	ASSERT_EQUAL(int, EINVAL, errno);
	ASSERT_EQUAL(size_t, prev_len, hello->len);
	ASSERT_EQUAL(size_t, prev_cap, hello->cap);
	ASSERT_TRUE(memcmp(hello->data, "hello", hello->len) == 0);

	dstr_free(hello);
}

void test_dstr_cat_cstr_src_with_null_byte(void)
{
	char *world = "world\0anduniverse";

	dstr *hello = dstr_new("hello");
	const size_t prev_len = hello->len;

	int done = dstr_cat_cstr(&hello, world);

	ASSERT_EQUAL(int, 0, done);
	ASSERT_EQUAL(size_t, prev_len + strlen(world), hello->len);
	ASSERT_TRUE(hello->cap > 10);
	ASSERT_EQUAL(char, '\0', hello->data[hello->len]);
	ASSERT_TRUE(memcmp(hello->data, "helloworld", hello->len) == 0);

	dstr_free(hello);
}

#define RUN_TEST(test)                                                                             \
	do {                                                                                           \
		curr_test = #test;                                                                         \
		test();                                                                                    \
		printf(GREEN "[PASS] " RESET "%s\n", #test);                                               \
	} while (0)

int main(void)
{
	RUN_TEST(test_dstr_new_n_normal);
	RUN_TEST(test_dstr_new_n_null);
	RUN_TEST(test_dstr_new_n_copies_input);
	RUN_TEST(test_dstr_new_n_binary_data);
	RUN_TEST(test_dstr_empty);
	RUN_TEST(test_dstr_new_null_cstr);
	RUN_TEST(test_dstr_new_stops_at_null);
	RUN_TEST(test_dstr_dup_normal);
	RUN_TEST(test_dstr_dup_null);
	RUN_TEST(test_dstr_cat_n_null_dest_arg);
	RUN_TEST(test_dstr_cat_n_empty_dstr);
	RUN_TEST(test_dstr_cat_n_null_dest);
	RUN_TEST(test_dstr_cat_n_null_src);
	RUN_TEST(test_dstr_cat_n_src_with_null_byte);
	RUN_TEST(test_dstr_cat_normal);
	RUN_TEST(test_dstr_cat_null_src);
	RUN_TEST(test_dstr_cat_cstr_normal);
	RUN_TEST(test_dstr_cat_cstr_null_src);
	RUN_TEST(test_dstr_cat_cstr_src_with_null_byte);
	printf("\n" GREEN "All tests passed." RESET "\n");
	return 0;
}
