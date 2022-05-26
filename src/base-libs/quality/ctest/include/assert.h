#pragma once

#define ASSERT_INT_EQUALS(msg, actual, expected) assertion_error += assert_int_equals(msg, actual, expected);

#define ASSERT_INT_LESS_THAN(msg, actual, expected) assertion_error += assert_int_less_than(msg, actual, expected);

#define ASSERT_TRUE(msg, expr) assertion_error += assert_true(msg, expr);

#define ASSERT_FALSE(msg, expr) assertion_error += assert_false(msg, expr);

#define ASSERT_NULL(msg, ptr) assertion_error += assert_null(msg, ptr);
#define ASSERT_NOT_NULL(msg, ptr) assertion_error += assert_not_null(msg, ptr);

#define ASSERT_LF_EQUALS(msg, actual, expected, epsilon) assertion_error += assert_long_double_equals(msg, actual, expected, epsilon);

char assert_null(char *assertion, void *ptr);
char assert_not_null(char *assertion, void *ptr);

char assert_int_equals(char *assertion, int num1, int num2);

char assert_int_less_than(char *assertion, int num1, int num2);

char assert_str_equals(char *assertion, char *str1, char *str2);

char assert_substr_in(char *assertion, char *str1, char *str2);

char assert_float_less_than(char *assertin, float n1, float n2);

char assert_double_equals(char *assertion, double num1, double num2);

char assert_long_double_equals(char *assertion, long double num1, long double num2, long double epsilon);

char assert_true(char *assertion, int value);

char assert_false(char *assertion, int value);
