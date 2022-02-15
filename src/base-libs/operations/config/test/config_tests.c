
#include "config.h"
#include "map.h"
#include "test.h"
#include "assert.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct _config_file {
    FILE *ptr;
    char name[32];
} config_file;

config_file write_tmp_config_file(char *config) {
    // Write the config to a file
    int fd;
    char config_file_name[] = "/tmp/config_tests_XXXXXX";
    fd = mkstemp(config_file_name);
    FILE *config_file_ptr = fdopen(fd, "w+");
    fwrite(config, sizeof(char), strlen(config), config_file_ptr);
    fflush(config_file_ptr);

    printf("Wrote temporary config to file %s\n", config_file_name);

    config_file tmp_file;
    tmp_file.ptr = config_file_ptr;
    sprintf(tmp_file.name, "%s", config_file_name);

    return tmp_file;
}

char test_load_config_from_file() {
    arg_t arg_desc[] = { { "i", "integer", INTEGER }, { "f", "float", FLOAT }, { "g", "flag", FLAG }, { "s", "string", STRING } };

    // Create a configuration file
    char *config = "# Some comment here =)\n"
                   "integer 100\n"
                   "\n"
                   "#Another comment after an empty line\n"
                   "float 10.10\n"
                   "\n"
                   "flag\n"
                   "\n"
                   "string some string \n" // Make sure the string is not trimmed
                   "integer 10\n"; // Make sure the value is overriden

    config_file f = write_tmp_config_file(config);
    
    // Get the name of the temporary file created
    char *argv[] = { "test", "-c", f.name };

    map *args = load_config(sizeof(arg_desc) / sizeof(arg_t), arg_desc, sizeof(argv) / sizeof(char *), argv);

    // Close config file descriptor, this may cause deletion of the file.
    fclose(f.ptr);

    // Assert parsed values
    int assertion_error = 0;
    assertion_error += assert_int_equals("Parsed value is the given value", *(int *)map_get(args, "integer"), 10);
    assertion_error += assert_double_equals("Parsed value is the given value", *(double *)map_get(args, "float"), 10.10);
    if (!map_get(args, "flag")) {
        printf("Expected the flag 'some-flag' to be set and evaluate to true.");
        assertion_error += 1;
    }
    assertion_error += assert_str_equals("Parsed value is the given value", (char *)map_get(args, "string"), "some string ");

    // Clean up
    deallocate_arg_map(sizeof(arg_desc) / sizeof(arg_t), arg_desc, args);

    // Return test status
    return assertion_error;
}

char test_unparseable_float_in_config_file() {

    arg_t arg_desc[] = { { "i", "integer", INTEGER }, { "f", "float", FLOAT }, { "g", "flag", FLAG }, { "s", "string", STRING } };

    // Create a configuration file
    char *config = "float 10;10\n";

    // Write the config to a file
    config_file f = write_tmp_config_file(config);

    // Get the name of the temporary file created
    char *argv[] = { "test", "-c", f.name };

    map *args = load_config(sizeof(arg_desc) / sizeof(arg_t), arg_desc, sizeof(argv) / sizeof(char *), argv);

    // Close config file descriptor, this may cause deletion of the file.
    fclose(f.ptr);
}

char test_unparseable_int_in_config_file() {

    arg_t arg_desc[] = { { "i", "integer", INTEGER }, { "f", "float", FLOAT }, { "g", "flag", FLAG }, { "s", "string", STRING } };

    // Create a configuration file
    char *config = "integer a10;\n";

    // Write the config to a file
    config_file f = write_tmp_config_file(config);

    // Get the name of the temporary file created
    char *argv[] = { "test", "-c", f.name };

    map *args = load_config(sizeof(arg_desc) / sizeof(arg_t), arg_desc, sizeof(argv) / sizeof(char *), argv);

    // Close config file descriptor, this may cause deletion of the file.
    fclose(f.ptr);
}

char test_unrecognized_arguments_in_config_file() {

    arg_t arg_desc[] = { { "i", "integer", INTEGER }, { "f", "float", FLOAT }, { "g", "flag", FLAG }, { "s", "string", STRING } };

    // Create a configuration file
    char *config = "unrecognized_arg a10;\n";

    // Write the config to a file
    config_file f = write_tmp_config_file(config);

    // Get the name of the temporary file created
    char *argv[] = { "test", "-c", f.name };

    map *args = load_config(sizeof(arg_desc) / sizeof(arg_t), arg_desc, sizeof(argv) / sizeof(char *), argv);

    // Close config file descriptor, this may cause deletion of the file.
    fclose(f.ptr);
}

char test_command_line_arguments_take_precedence_over_config_file() {
    arg_t arg_desc[] = { { "i", "integer", INTEGER }, { "f", "float", FLOAT }, { "g", "flag", FLAG }, { "s", "string", STRING } };

    // Create a configuration file
    char *config = "string file\n";

    // Write the config to a file
    config_file f = write_tmp_config_file(config);

    // Get the name of the temporary file created
    char *argv[] = { "test", "--string", "command line arg", "-c", f.name };

    map *args = load_config(sizeof(arg_desc) / sizeof(arg_t), arg_desc, sizeof(argv) / sizeof(char *), argv);

    // Close config file descriptor, this may cause deletion of the file.
    fclose(f.ptr);

    // Assert parsed values
    int assertion_error = 0;
    assertion_error = assert_str_equals("Command line argument takes precendence over config file", (char *)map_get(args, "string"), "command line arg"); 

    // Clean up
    deallocate_arg_map(sizeof(arg_desc) / sizeof(arg_t), arg_desc, args);

    // Return test status
    return assertion_error;
}

char test_help_message_and_clean_exit_on_provided_flag() {
    arg_t arg_desc[] = { { "t", "test-arg", STRING } };

    char *argv[] = { "test", "--help" };

    map *args = load_config(sizeof(arg_desc) / sizeof(arg_t), arg_desc, sizeof(argv) / sizeof(char *), argv);

    // Always return 1 since --help should make the program exit.
    return 1;
}

char test_not_providing_mandatory_arguments() {

    arg_t arg_desc[] = { { "i", "integer", INTEGER, MANDATORY }, { "f", "float", FLOAT }, { "g", "flag", FLAG }, { "s", "string", STRING } };

    // Create a configuration file
    char *config = "float 10;\n";

    // Write the config to a file
    config_file f = write_tmp_config_file(config);

    // Get the name of the temporary file created
    char *argv[] = { "test", "-c", f.name };

    map *args = load_config(sizeof(arg_desc) / sizeof(arg_t), arg_desc, sizeof(argv) / sizeof(char *), argv);

    // Close config file descriptor, this may cause deletion of the file.
    fclose(f.ptr);
}

char test_single_string_argument_in_long_form_happy_path() {
    arg_t arg_desc[] = { { "t", "test-arg", STRING } };

    char *argv[] = { "test", "--test-arg", "some value" };

    map *args = load_config(sizeof(arg_desc) / sizeof(arg_t), arg_desc, sizeof(argv) / sizeof(char *), argv);

    char *parsed_value = (char *)map_get(args, "test-arg");
    int assertion_error = assert_str_equals("Big int is incremented by one.", parsed_value, "some value");

    // Clean up
    deallocate_arg_map(sizeof(arg_desc) / sizeof(arg_t), arg_desc, args);

    // Return test status
    if (assertion_error) {
        return 1;
    }
    return 0;
}

char test_two_string_arguments_in_long_form_happy_path() {
    arg_t arg_desc[] = { { "t", "test-arg", STRING }, { "o", "other-arg", STRING } };

    char *argv[] = { "test", "--test-arg", "some value", "--other-arg", "other value" };

    map *args = load_config(sizeof(arg_desc) / sizeof(arg_t), arg_desc, sizeof(argv) / sizeof(char *), argv);

    char *parsed_value = (char *)map_get(args, "test-arg");
    char *other_parsed_value = (char *)map_get(args, "other-arg");

    int assertion_error = assert_str_equals("Parsed value is the given value.", parsed_value, "some value") &&
                          assert_str_equals("Parsed value is the given value.", other_parsed_value, "other value");
    ;

    // Clean up
    deallocate_arg_map(sizeof(arg_desc) / sizeof(arg_t), arg_desc, args);

    // Return test status
    if (assertion_error) {
        return 1;
    }
    return 0;
}

char test_one_integer_argument_in_long_form_happy_path() {
    arg_t arg_desc[] = { { "i", "int-arg", INTEGER } };

    char *argv[] = { "test", "--int-arg", "100" };

    map *args = load_config(sizeof(arg_desc) / sizeof(arg_t), arg_desc, sizeof(argv) / sizeof(char *), argv);

    int *parsed_value = (int *)map_get(args, "int-arg");
    int assertion_error = assert_int_equals("Parsed value is the given value.", *parsed_value, 100);

    // Clean up
    deallocate_arg_map(sizeof(arg_desc) / sizeof(arg_t), arg_desc, args);

    // Return test status
    if (assertion_error) {
        return 1;
    }
    return 0;
}

char test_one_double_argument_in_long_form_happy_path() {
    arg_t arg_desc[] = { { "i", "float-arg", FLOAT } };

    char *argv[] = { "test", "--float-arg", "100.99" };

    map *args = load_config(sizeof(arg_desc) / sizeof(arg_t), arg_desc, sizeof(argv) / sizeof(char *), argv);

    double *parsed_value = (double *)map_get(args, "float-arg");
    int assertion_error = assert_double_equals("Parsed value is the given value.", *parsed_value, 100.99);

    // Clean up
    deallocate_arg_map(sizeof(arg_desc) / sizeof(arg_t), arg_desc, args);

    // Return test status
    if (assertion_error) {
        return 1;
    }
    return 0;
}

char test_one_flag_argument_in_long_form_happy_path() {
    arg_t arg_desc[] = { { "i", "some-flag", FLAG } };

    char *argv[] = { "test", "--some-flag" };

    map *args = load_config(sizeof(arg_desc) / sizeof(arg_t), arg_desc, sizeof(argv) / sizeof(char *), argv);

    int assertion_error = 0;
    if (!map_get(args, "some-flag")) {
        printf("Expected the flag 'some-flag' to be set and evaluate to true.");
        assertion_error = 1;
    }

    // Clean up
    deallocate_arg_map(sizeof(arg_desc) / sizeof(arg_t), arg_desc, args);

    // Return test status
    if (assertion_error) {
        return 1;
    }
    return 0;
}

char test_all_argument_types_in_short_form_happy_path() {
    arg_t arg_desc[] = { { "i", "integer", INTEGER }, { "f", "float", FLOAT }, { "g", "flag", FLAG }, { "s", "string", STRING } };

    char *argv[] = { "test", "-i", "100", "-f", "10.10", "-g", "-s", "my string" };

    map *args = load_config(sizeof(arg_desc) / sizeof(arg_t), arg_desc, sizeof(argv) / sizeof(char *), argv);

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
    deallocate_arg_map(sizeof(arg_desc) / sizeof(arg_t), arg_desc, args);

    // Return test status
    return assertion_error;
}

char test_parsing_same_argument_twice() {
    arg_t arg_desc[] = { { "i", "integer", INTEGER } };

    char *argv[] = { "test", "--integer", "100", "-i", "10", "--integer", "1000" };

    map *args = load_config(sizeof(arg_desc) / sizeof(arg_t), arg_desc, sizeof(argv) / sizeof(char *), argv);

    int assertion_error = assert_int_equals("Parsed argument is the last given argument", *(int *)map_get(args, "integer"), 1000);

    // Clean up
    deallocate_arg_map(sizeof(arg_desc) / sizeof(arg_t), arg_desc, args);

    // Return test status
    return assertion_error;
}

char test_parsing_non_existent_argument() {
    arg_t arg_desc[] = { { "i", "integer", INTEGER } };

    char *argv[] = { "test", "--nonexistent", "100" };

    map *args = load_config(sizeof(arg_desc) / sizeof(arg_t), arg_desc, sizeof(argv) / sizeof(char *), argv);

    // Clean up
    deallocate_arg_map(sizeof(arg_desc) / sizeof(arg_t), arg_desc, args);

    // Return test status
    return 0;
}

char test_parsing_string_as_integer_throws_error() {
    arg_t arg_desc[] = { { "i", "integer", INTEGER } };

    char *argv[] = { "test", "--integer", "my string" };

    map *args = load_config(sizeof(arg_desc) / sizeof(arg_t), arg_desc, sizeof(argv) / sizeof(char *), argv);

    // Clean up
    deallocate_arg_map(sizeof(arg_desc) / sizeof(arg_t), arg_desc, args);

    // Return test status
    return 0;
}

char test_parsing_float_as_integer_throws_error() {
    arg_t arg_desc[] = { { "i", "integer", INTEGER } };

    char *argv[] = { "test", "--integer", "100.01" };

    map *args = load_config(sizeof(arg_desc) / sizeof(arg_t), arg_desc, sizeof(argv) / sizeof(char *), argv);

    // Clean up
    deallocate_arg_map(sizeof(arg_desc) / sizeof(arg_t), arg_desc, args);

    // Return test status
    return 0;
}

char test_parsing_string_as_float_throws_error() {
    arg_t arg_desc[] = { { "f", "float", FLOAT } };

    char *argv[] = { "test", "--float", "my string" };

    map *args = load_config(sizeof(arg_desc) / sizeof(arg_t), arg_desc, sizeof(argv) / sizeof(char *), argv);

    // Clean up
    deallocate_arg_map(sizeof(arg_desc) / sizeof(arg_t), arg_desc, args);

    // Return test status
    return 0;
}

test test_suite[] = { { "Test single program argument, single argument given, happy path", test_single_string_argument_in_long_form_happy_path },
                      { "Test two program arguments, two arguments given, happy path", test_two_string_arguments_in_long_form_happy_path },
                      { "Test parsing an integer argument given in the long form", test_one_integer_argument_in_long_form_happy_path },
                      { "Test parsing an float argument given in the long form", test_one_double_argument_in_long_form_happy_path },
                      { "Test parsing a flag argument given in the long form", test_one_flag_argument_in_long_form_happy_path },
                      { "Test parsing all argument types all in short form", test_all_argument_types_in_short_form_happy_path },
                      { "Test parsing the same argument twice times", test_parsing_same_argument_twice },
                      { "Test that trying to parse a non existing argument ends up in error", test_parsing_non_existent_argument, EXIT_FAILURE },
                      { "Test that trying to parse a string as an integer ends in error", test_parsing_string_as_integer_throws_error, EXIT_FAILURE },
                      { "Test that trying to parse a float as an integer ends in error", test_parsing_float_as_integer_throws_error, EXIT_FAILURE },
                      { "Test that trying to parse a string as a float ends in error", test_parsing_string_as_float_throws_error, EXIT_FAILURE },
                      { "Test that a help message is printed when -h or --help are passed as arguments", test_help_message_and_clean_exit_on_provided_flag, EXIT_SUCCESS },
                      { "Test that not providing a value for a mandatory argument results in error", test_not_providing_mandatory_arguments, EXIT_FAILURE },
                      { "Test that it is possible to parse configuration from file", test_load_config_from_file },
                      { "Test that providing a malformated float value in config file results in error", test_unparseable_float_in_config_file, EXIT_FAILURE },
                      { "Test that providing a malformated integer value in config file results in error", test_unparseable_int_in_config_file, EXIT_FAILURE },
                      { "Test that providing invalid arguments in config file results in error", test_unrecognized_arguments_in_config_file, EXIT_FAILURE },
                      { "Test that command line arguments take precedence over config file ones",
                        test_command_line_arguments_take_precedence_over_config_file } };

int main(int argc, char **argv) {
    suite_report report = run_test_suite(test_suite, sizeof(test_suite) / sizeof(test));

    if (report.failures > 0) {
        return -1;
    }

    return 0;
}
