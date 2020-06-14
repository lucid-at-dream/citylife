
#include "config.h"
#include "hashmap/map.h"
#include "ctest/test.h"
#include "ctest/assert.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

void setup_env() {
}

char before_test() {
    return 0;
}

char after_test() {
    return 0;
}

void clean_env() {
}

char test_single_string_argument_in_long_form_happy_path() {

    arg_t arg_desc[] = {
        {"t", "test-arg", STRING}
    };

    char *argv[] = {"test", "--test-arg", "some value"};

    map *args = arg_parse(sizeof(arg_desc)/sizeof(arg_t), arg_desc, 3, argv);

    char *parsed_value = (char *)map_get(args, "test-arg");
    int assertion_error = assert_str_equals("Big int is incremented by one.", parsed_value, "some value");
    
    // Clean up
    deallocate_arg_map(sizeof(arg_desc)/sizeof(arg_t), arg_desc, args);

    // Return test status
    if (assertion_error) {
        return 1;
    }
    return 0;
}

char test_two_string_arguments_in_long_form_happy_path() {

    arg_t arg_desc[] = {
        {"t", "test-arg", STRING},
        {"o", "other-arg", STRING}
    };

    char *argv[] = {"test", "--test-arg", "some value", "--other-arg", "other value"};

    map *args = arg_parse(sizeof(arg_desc)/sizeof(arg_t), arg_desc, 3, argv);

    char *parsed_value = (char *)map_get(args, "test-arg");
    char *other_parsed_value = (char *)map_get(args, "other-arg");

    int assertion_error = assert_str_equals("Parsed value is the given value.", parsed_value, "some value")
            && assert_str_equals("Parsed value is the given value.", other_parsed_value, "other value");;
    
    // Clean up
    deallocate_arg_map(sizeof(arg_desc)/sizeof(arg_t), arg_desc, args);

    // Return test status
    if (assertion_error) {
        return 1;
    }
    return 0;
}

char test_one_integer_argument_in_long_form_happy_path() {

    arg_t arg_desc[] = {
        {"i", "int-arg", INTEGER}
    };

    char *argv[] = {"test", "--int-arg", "100"};

    map *args = arg_parse(sizeof(arg_desc)/sizeof(arg_t), arg_desc, 3, argv);

    int *parsed_value = (int *)map_get(args, "int-arg");
    int assertion_error = assert_int_equals("Parsed value is the given value.", *parsed_value, 100);
    
    // Clean up
    deallocate_arg_map(sizeof(arg_desc)/sizeof(arg_t), arg_desc, args);

    // Return test status
    if (assertion_error) {
        return 1;
    }
    return 0;
}

char test_one_double_argument_in_long_form_happy_path() {

    arg_t arg_desc[] = {
        {"i", "float-arg", FLOAT}
    };

    char *argv[] = {"test", "--float-arg", "100.99"};

    map *args = arg_parse(sizeof(arg_desc)/sizeof(arg_t), arg_desc, 3, argv);

    double *parsed_value = (double *)map_get(args, "float-arg");
    int assertion_error = assert_double_equals("Parsed value is the given value.", *parsed_value, 100.99);
    
    // Clean up
    deallocate_arg_map(sizeof(arg_desc)/sizeof(arg_t), arg_desc, args);

    // Return test status
    if (assertion_error) {
        return 1;
    }
    return 0;
}

char test_one_flag_argument_in_long_form_happy_path() {

    arg_t arg_desc[] = {
        {"i", "some-flag", FLAG}
    };

    char *argv[] = {"test", "--some-flag"};

    map *args = arg_parse(sizeof(arg_desc)/sizeof(arg_t), arg_desc, 2, argv);

    int assertion_error = 0;
    if (!map_get(args, "some-flag")) {
        printf("Expected the flag 'some-flag' to be set and evaluate to true.");
        assertion_error = 1;
    }
    
    // Clean up
    deallocate_arg_map(sizeof(arg_desc)/sizeof(arg_t), arg_desc, args);

    // Return test status
    if (assertion_error) {
        return 1;
    }
    return 0;
}

char test_all_argument_types_in_short_form_happy_path() {
    arg_t arg_desc[] = {
        {"i", "integer", INTEGER},
        {"f", "float", FLOAT},
        {"g", "flag", FLAG},
        {"s", "string", STRING}
    };

    char *argv[] = {
        "test", 
        "-i", "100",
        "-f", "10.10",
        "-g",
        "-s", "my string"
    };

    map *args = arg_parse(sizeof(arg_desc)/sizeof(arg_t), arg_desc, sizeof(argv) / sizeof(char *), argv);

    // Assert parsed values
    int assertion_error = 0;
    assertion_error |= assert_int_equals("Parsed value is the given value", *(int *)map_get(args, "integer"), 100);
    assertion_error |= assert_double_equals("Parsed value is the given value", *(double *)map_get(args, "float"), 10.10);
    if (!map_get(args, "flag")) {
        printf("Expected the flag 'some-flag' to be set and evaluate to true.");
        assertion_error |= 1;
    }
    assertion_error |= assert_str_equals("Parsed value is the given value", (char *)map_get(args, "string"), "my string");

    // Clean up
    deallocate_arg_map(sizeof(arg_desc)/sizeof(arg_t), arg_desc, args);

    // Return test status
    return assertion_error;
}

char test_parsing_same_argument_twice() {

    arg_t arg_desc[] = {
        {"i", "integer", INTEGER}
    };

    char *argv[] = {"test", "--integer", "100", "-i", "10", "--integer", "1000"};

    map *args = arg_parse(sizeof(arg_desc)/sizeof(arg_t), arg_desc, sizeof(argv)/sizeof(char *), argv);

    int assertion_error = assert_int_equals("Parsed argument is the last given argument", *(int *)map_get(args, "integer"), 1000);

    // Clean up
    deallocate_arg_map(sizeof(arg_desc)/sizeof(arg_t), arg_desc, args);

    // Return test status
    return assertion_error;
}

char test_parsing_non_existent_argument() {

    arg_t arg_desc[] = {
        {"i", "integer", INTEGER}
    };

    char *argv[] = {"test", "--nonexistent", "100"};

    map *args = arg_parse(sizeof(arg_desc)/sizeof(arg_t), arg_desc, sizeof(argv)/sizeof(char *), argv);

    // Clean up
    deallocate_arg_map(sizeof(arg_desc)/sizeof(arg_t), arg_desc, args);

    // Return test status
    return 0;
}

char test_parsing_string_as_integer_throws_error() {
    arg_t arg_desc[] = {
        {"i", "integer", INTEGER}
    };

    char *argv[] = {"test", "--integer", "my string"};

    map *args = arg_parse(sizeof(arg_desc)/sizeof(arg_t), arg_desc, sizeof(argv)/sizeof(char *), argv);

    // Clean up
    deallocate_arg_map(sizeof(arg_desc)/sizeof(arg_t), arg_desc, args);

    // Return test status
    return 0;
}


test test_suite[] = {
    {
        "Test single program argument, single argument given, happy path", test_single_string_argument_in_long_form_happy_path
    },
    {
        "Test two program arguments, two arguments given, happy path", test_two_string_arguments_in_long_form_happy_path
    },
    {
        "Test parsing an integer argument given in the long form", test_one_integer_argument_in_long_form_happy_path
    },
    {
        "Test parsing an float argument given in the long form", test_one_double_argument_in_long_form_happy_path
    },
    {
        "Test parsing a flag argument given in the long form", test_one_flag_argument_in_long_form_happy_path
    },
    {
        "Test parsing all argument types all in short form", test_all_argument_types_in_short_form_happy_path
    },
    {
        "Test parsing the same argument twice times", test_parsing_same_argument_twice
    },
    {
        "Test that trying to parse a non existing argument ends up in error", test_parsing_non_existent_argument, EXIT_FAILURE
    },
    {
        "Test that trying to parse a string as an integer ends in error", test_parsing_string_as_integer_throws_error, EXIT_FAILURE
    }
};

int main(int argc, char **argv) {
    suite_report report = run_test_suite(test_suite, sizeof(test_suite)/sizeof(test));

    if (report.failures > 0) {
        return -1;
    }

    return 0;
}
