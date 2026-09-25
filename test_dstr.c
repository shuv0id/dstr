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

	ASSERT_EQUAL(size_t, strlen(greet), dstr_len(str));
	ASSERT_TRUE(dstr_cap(str) > 5);
	ASSERT_TRUE(memcmp(dstr_data(str), greet, dstr_len(str)) == 0);
	ASSERT_EQUAL(char, '\0', dstr_data(str)[dstr_len(str)]);

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

	ASSERT_TRUE(memcmp(dstr_data(str), "Hello", dstr_len(str)) == 0);

	dstr_free(str);
}

void test_dstr_new_n_binary_data(void)
{
	char buf[] = {'A', 'B', '\0', 'D'};

	dstr *str = dstr_new_n(buf, sizeof(buf));

	ASSERT_EQUAL(size_t, sizeof(buf), dstr_len(str));
	ASSERT_TRUE(dstr_cap(str) > 4);
	ASSERT_TRUE(memcmp(dstr_data(str), buf, dstr_len(str)) == 0);
	ASSERT_EQUAL(char, '\0', dstr_data(str)[dstr_len(str)]);

	dstr_free(str);
}

void test_dstr_empty(void)
{
	dstr *str = dstr_empty();

	ASSERT_EQUAL(size_t, 0, dstr_len(str));
	ASSERT_TRUE(dstr_cap(str) > 0);
	ASSERT_TRUE(dstr_data(str)[0] == '\0');

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

	ASSERT_EQUAL(size_t, 5, dstr_len(str));
	ASSERT_TRUE(dstr_cap(str) > 5);
	ASSERT_TRUE(strcmp(hello, dstr_data(str)) == 0);
	ASSERT_EQUAL(char, '\0', dstr_data(str)[dstr_len(str)]);

	dstr_free(str);
}

void test_dstr_dup_null(void)
{
	errno = 0;
	dstr *dup = dstr_dup(NULL);

	ASSERT_TRUE(dup == NULL);
	ASSERT_EQUAL(int, EINVAL, errno);

	dstr_free(dup);
}

void test_dstr_dup_normal(void)
{
	dstr *original = dstr_new("tip");
	dstr *dup = dstr_dup(original);

	ASSERT_TRUE(dstr_data(original) != dstr_data(dup));

	// Mutate original dstr by appending more data to its buffer.
	ASSERT_TRUE(dstr_cat_n(&original, "tap", 3) == 0);

	ASSERT_EQUAL(size_t, 6, dstr_len(original));
	ASSERT_EQUAL(size_t, 3, dstr_len(dup));

	ASSERT_TRUE(strcmp(dstr_data(original), "tiptap") == 0);

	// Appending to the original should not modify the duplicate.
	ASSERT_TRUE(strcmp(dstr_data(dup), "tip") == 0);

	dstr_free(original);
	dstr_free(dup);
}

void test_dstr_set(void)
{
	dstr *greet_world = dstr_new("hello, world!");
	char *greet_aliens = "hello, aliens!";

	int done = dstr_set(&greet_world, greet_aliens, strlen(greet_aliens));

	ASSERT_EQUAL(int, 0, done);
	ASSERT_EQUAL(size_t, strlen(greet_aliens), dstr_len(greet_world));
	ASSERT_TRUE(memcmp(dstr_data(greet_world), greet_aliens, dstr_len(greet_world)) == 0);
	ASSERT_EQUAL(char, '\0', dstr_data(greet_world)[dstr_len(greet_world)]);

	dstr_free(greet_world);
}

void test_dstr_set_null_str_arg(void)
{
	errno = 0;
	int done = dstr_cat_n(NULL, NULL, 1);

	ASSERT_EQUAL(int, -1, done);
	ASSERT_EQUAL(int, EINVAL, errno);
}

void test_dstr_set_null_str(void)
{
	errno = 0;
	dstr *str = NULL;
	int done = dstr_cat_n(&str, NULL, 1);

	ASSERT_EQUAL(int, -1, done);
	ASSERT_EQUAL(int, EINVAL, errno);
}

void test_dstr_set_null_data(void)
{
	dstr *hello = dstr_new("hello");
	const size_t prev_len = dstr_len(hello);
	const size_t prev_cap = dstr_cap(hello);

	errno = 0;
	int done = dstr_cat_n(&hello, NULL, 1);

	ASSERT_EQUAL(int, -1, done);
	ASSERT_EQUAL(int, EINVAL, errno);
	ASSERT_EQUAL(size_t, prev_len, dstr_len(hello));
	ASSERT_EQUAL(size_t, prev_cap, dstr_cap(hello));
	ASSERT_TRUE(memcmp(dstr_data(hello), "hello", dstr_len(hello)) == 0);

	dstr_free(hello);
}

void test_dstr_set_zero_n(void)
{
	char *hello = "hello";
	char *world = "world";

	dstr *hello_dstr = dstr_new(hello);
	const size_t prev_len = dstr_len(hello_dstr);
	const size_t prev_cap = dstr_cap(hello_dstr);

	errno = 0;
	int done = dstr_cat_n(&hello_dstr, world, 0);

	ASSERT_EQUAL(int, 0, done);
	ASSERT_EQUAL(size_t, prev_len, dstr_len(hello_dstr));
	ASSERT_EQUAL(size_t, prev_cap, dstr_cap(hello_dstr));
	ASSERT_TRUE(memcmp(dstr_data(hello_dstr), hello, dstr_len(hello_dstr)) == 0);

	dstr_free(hello_dstr);
}

void test_dstr_reset(void)
{
	dstr *abc = dstr_new("abc");
	size_t cap = dstr_cap(abc);

	dstr_reset(abc);

	ASSERT_EQUAL(size_t, 0, dstr_len(abc));
	ASSERT_EQUAL(size_t, cap, dstr_cap(abc));
	ASSERT_TRUE(dstr_data(abc)[0] == '\0');

	dstr_free(abc);
}

void test_dstr_cat_n_empty_dstr(void)
{
	char *greet = "helloworld";
	dstr *dest = dstr_empty();

	int done = dstr_cat_n(&dest, greet, 10);

	ASSERT_EQUAL(int, 0, done);
	ASSERT_EQUAL(size_t, 10, dstr_len(dest));
	ASSERT_TRUE(dstr_cap(dest) > 10);
	ASSERT_TRUE(memcmp(dstr_data(dest), greet, dstr_len(dest)) == 0);

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
	const size_t prev_len = dstr_len(hello);
	const size_t prev_cap = dstr_cap(hello);

	errno = 0;
	int done = dstr_cat_n(&hello, NULL, 1);

	ASSERT_EQUAL(int, -1, done);
	ASSERT_EQUAL(int, EINVAL, errno);
	ASSERT_EQUAL(size_t, prev_len, dstr_len(hello));
	ASSERT_EQUAL(size_t, prev_cap, dstr_cap(hello));
	ASSERT_TRUE(memcmp(dstr_data(hello), "hello", dstr_len(hello)) == 0);

	dstr_free(hello);
}

void test_dstr_cat_n_src_with_null_byte(void)
{
	char *greet = "\0world";
	dstr *dest = dstr_new_n("hello", 5);

	int ok = dstr_cat_n(&dest, greet, 6);

	ASSERT_EQUAL(int, 0, ok);
	ASSERT_EQUAL(size_t, 11, dstr_len(dest));
	ASSERT_TRUE(dstr_cap(dest) > 11);
	ASSERT_TRUE(memcmp(dstr_data(dest), "hello\0world", dstr_len(dest)) == 0);

	dstr_free(dest);
}

void test_dstr_cat_n_large_data(void)
{
	dstr *abc = dstr_new("abc");

	size_t old_len = dstr_len(abc);
	size_t old_cap = dstr_cap(abc);

	// Append data with size larger than the
	// capacity of abc dstr to ensure reallocation
	size_t large_data_sz = old_cap * 2;
	char large_data[large_data_sz];
	for (size_t i = 0; i < large_data_sz; i++) {
		large_data[i] = '#';
	}

	int done = dstr_cat_n(&abc, large_data, large_data_sz);

	ASSERT_EQUAL(int, 0, done);
	ASSERT_EQUAL(size_t, old_len + large_data_sz, dstr_len(abc));
	ASSERT_TRUE(memcmp(dstr_data(abc), "abc", old_len) == 0);
	ASSERT_TRUE(memcmp(dstr_data(abc) + old_len, large_data, large_data_sz) == 0);
	ASSERT_EQUAL(char, '\0', dstr_data(abc)[dstr_len(abc)]);

	dstr_free(abc);
}

void test_dstr_cat_n_self_append(void)
{
	dstr *abc = dstr_new("abc");
	size_t old_len = dstr_len(abc);

	int done = dstr_cat_n(&abc, dstr_data(abc), old_len);

	ASSERT_EQUAL(int, 0, done);
	ASSERT_EQUAL(size_t, old_len * 2, dstr_len(abc));
	ASSERT_TRUE(memcmp(dstr_data(abc), "abcabc", dstr_len(abc)) == 0);
	ASSERT_EQUAL(char, '\0', dstr_data(abc)[dstr_len(abc)]);

	dstr_free(abc);
}

void test_dstr_cat_normal(void)
{
	dstr *hello = dstr_new_n("hello", 5);
	dstr *world = dstr_new_n("\0world", 6);

	int done = dstr_cat(&hello, world);
	ASSERT_EQUAL(int, 0, done);
	ASSERT_EQUAL(size_t, 11, dstr_len(hello));
	ASSERT_TRUE(dstr_cap(hello) > 11);
	ASSERT_TRUE(memcmp(dstr_data(hello), "hello\0world", dstr_len(hello)) == 0);

	dstr_free(hello);
	dstr_free(world);
}

void test_dstr_cat_null_src(void)
{
	dstr *hello = dstr_new("hello");
	const size_t prev_len = dstr_len(hello);
	const size_t prev_cap = dstr_cap(hello);

	errno = 0;
	int done = dstr_cat(&hello, NULL);

	ASSERT_EQUAL(int, -1, done);
	ASSERT_EQUAL(int, EINVAL, errno);
	ASSERT_EQUAL(size_t, prev_len, dstr_len(hello));
	ASSERT_EQUAL(size_t, prev_cap, dstr_cap(hello));
	ASSERT_TRUE(memcmp(dstr_data(hello), "hello", dstr_len(hello)) == 0);

	dstr_free(hello);
}

void test_dstr_cat_cstr_normal(void)
{
	char *world = "world";

	dstr *hello = dstr_new("hello");
	const size_t prev_len = dstr_len(hello);

	int done = dstr_cat_cstr(&hello, world);

	ASSERT_EQUAL(int, 0, done);
	ASSERT_EQUAL(size_t, prev_len + strlen(world), dstr_len(hello));
	ASSERT_TRUE(dstr_cap(hello) > 10);
	ASSERT_TRUE(memcmp(dstr_data(hello), "helloworld", dstr_len(hello)) == 0);

	dstr_free(hello);
}

void test_dstr_cat_cstr_null_src(void)
{
	dstr *hello = dstr_new("hello");
	const size_t prev_len = dstr_len(hello);
	const size_t prev_cap = dstr_cap(hello);

	errno = 0;
	int done = dstr_cat_cstr(&hello, NULL);

	ASSERT_EQUAL(int, -1, done);
	ASSERT_EQUAL(int, EINVAL, errno);
	ASSERT_EQUAL(size_t, prev_len, dstr_len(hello));
	ASSERT_EQUAL(size_t, prev_cap, dstr_cap(hello));
	ASSERT_TRUE(memcmp(dstr_data(hello), "hello", dstr_len(hello)) == 0);

	dstr_free(hello);
}

void test_dstr_cat_cstr_src_with_null_byte(void)
{
	char *world = "world\0anduniverse";

	dstr *hello = dstr_new("hello");
	const size_t prev_len = dstr_len(hello);

	int done = dstr_cat_cstr(&hello, world);

	ASSERT_EQUAL(int, 0, done);
	ASSERT_EQUAL(size_t, prev_len + strlen(world), dstr_len(hello));
	ASSERT_TRUE(dstr_cap(hello) > 10);
	ASSERT_EQUAL(char, '\0', dstr_data(hello)[dstr_len(hello)]);
	ASSERT_TRUE(memcmp(dstr_data(hello), "helloworld", dstr_len(hello)) == 0);

	dstr_free(hello);
}

void test_dstr_cmp_n_identical(void)
{
	dstr *a = dstr_new("abcd");
	dstr *b = dstr_new("abcd");

	ASSERT_EQUAL(int, 0, dstr_cmp_n(a, b, 4));

	dstr_free(a);
	dstr_free(b);
}

void test_dstr_cmp_n_mismatch(void)
{
	dstr *a = dstr_new("abcd");
	dstr *b = dstr_new("abce");

	ASSERT_TRUE(dstr_cmp_n(a, b, 4) < 0);

	dstr_free(a);
	dstr_free(b);
}

void test_dstr_cmp_n_prefix_within_n(void)
{
	dstr *a = dstr_new("abcd");
	dstr *b = dstr_new("abcde");

	ASSERT_EQUAL(int, 0, dstr_cmp_n(a, b, 4));

	dstr_free(a);
	dstr_free(b);
}

void test_dstr_cmp_n_shorter(void)
{
	dstr *a = dstr_new("abcd");
	dstr *b = dstr_new("abcde");

	ASSERT_TRUE(dstr_cmp_n(a, b, 5) < 0);

	dstr_free(a);
	dstr_free(b);
}

void test_dstr_cmp_n_longer(void)
{
	dstr *a = dstr_new("abcd");
	dstr *b = dstr_new("abc");

	ASSERT_TRUE(dstr_cmp_n(a, b, 4) > 0);

	dstr_free(a);
	dstr_free(b);
}

void test_dstr_cmp_n_zero_bytes(void)
{
	dstr *a = dstr_new("abcd");
	dstr *b = dstr_new("wxyz");

	ASSERT_EQUAL(int, 0, dstr_cmp_n(a, b, 0));

	dstr_free(a);
	dstr_free(b);
}

void test_dstr_cmp_n_null_arg(void)
{
	dstr *a = NULL;

	errno = 0;
	dstr *b = dstr_new("wxyz");

	ASSERT_EQUAL(int, -1, dstr_cmp_n(a, b, 0));
	ASSERT_EQUAL(int, EINVAL, errno);

	dstr_free(b);
}

void test_dstr_eq_equal_data(void)
{
	dstr *a = dstr_new_n("hello\0", 6);
	dstr *b = dstr_new_n("hello\0", 6);

	bool eq = dstr_eq(a, b);

	ASSERT_TRUE(eq);

	dstr_free(a);
	dstr_free(b);
}

void test_dstr_eq_unequal_data(void)
{
	dstr *a = dstr_new_n("hello\0", 6);
	dstr *b = dstr_new_n("hel", 3);

	bool eq = dstr_eq(a, b);

	ASSERT_FALSE(eq);

	dstr_free(a);
	dstr_free(b);
}

void test_dstr_eq_null_arg(void)
{
	dstr *a = NULL;
	dstr *b = dstr_new_n("hello\0", 6);

	bool eq = dstr_eq(a, b);

	ASSERT_FALSE(eq);

	dstr_free(b);
}

void test_dstr_eq_ignorecase_equal_data_different_case(void)
{
	dstr *a = dstr_new("hello,World!");
	dstr *b = dstr_new("hello,world!");

	bool eq = dstr_eq_ignorecase(a, b);

	ASSERT_TRUE(eq);

	dstr_free(a);
	dstr_free(b);
}

void test_dstr_eq_ignorecase_unequal_data(void)
{
	dstr *a = dstr_new("HeLLo,WorlD!");
	dstr *b = dstr_new("hello,mortals!");

	bool eq = dstr_eq_ignorecase(a, b);

	ASSERT_FALSE(eq);

	dstr_free(a);
	dstr_free(b);
}

void test_dstr_eq_ignorecase_null_arg(void)
{
	dstr *a = NULL;
	dstr *b = dstr_new_n("hello\0", 6);

	bool eq = dstr_eq_ignorecase(a, b);

	ASSERT_FALSE(eq);

	dstr_free(b);
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
	RUN_TEST(test_dstr_dup_null);
	RUN_TEST(test_dstr_dup_normal);
	RUN_TEST(test_dstr_set);
	RUN_TEST(test_dstr_set_null_str_arg);
	RUN_TEST(test_dstr_set_null_str);
	RUN_TEST(test_dstr_set_null_data);
	RUN_TEST(test_dstr_set_zero_n);
	RUN_TEST(test_dstr_reset);
	RUN_TEST(test_dstr_cat_n_null_dest_arg);
	RUN_TEST(test_dstr_cat_n_empty_dstr);
	RUN_TEST(test_dstr_cat_n_null_dest);
	RUN_TEST(test_dstr_cat_n_null_src);
	RUN_TEST(test_dstr_cat_n_src_with_null_byte);
	RUN_TEST(test_dstr_cat_n_large_data);
	RUN_TEST(test_dstr_cat_n_self_append);
	RUN_TEST(test_dstr_cat_normal);
	RUN_TEST(test_dstr_cat_null_src);
	RUN_TEST(test_dstr_cat_cstr_normal);
	RUN_TEST(test_dstr_cat_cstr_null_src);
	RUN_TEST(test_dstr_cat_cstr_src_with_null_byte);
	RUN_TEST(test_dstr_cmp_n_identical);
	RUN_TEST(test_dstr_cmp_n_mismatch);
	RUN_TEST(test_dstr_cmp_n_prefix_within_n);
	RUN_TEST(test_dstr_cmp_n_shorter);
	RUN_TEST(test_dstr_cmp_n_longer);
	RUN_TEST(test_dstr_cmp_n_zero_bytes);
	RUN_TEST(test_dstr_cmp_n_null_arg);
	RUN_TEST(test_dstr_eq_equal_data);
	RUN_TEST(test_dstr_eq_unequal_data);
	RUN_TEST(test_dstr_eq_null_arg);
	RUN_TEST(test_dstr_eq_ignorecase_equal_data_different_case);
	RUN_TEST(test_dstr_eq_ignorecase_unequal_data);
	RUN_TEST(test_dstr_eq_ignorecase_null_arg);
	printf("\n" GREEN "All tests passed." RESET "\n");
	return 0;
}
