#define STL_LOG_PRINT_FILE_NAME

#include "stl_log.h"

int log_callback(void *arg, stl_log_level level,
                 const char *fmt, va_list va)
{
    const char *my_app = arg;
    const char *level_str = stl_log_levels[level].str;

    fprintf(stdout, " %s received log : level = [%s] ", my_app, level_str);
    vfprintf(stdout, fmt, va);

    return 0;
}

int main()
{
    const char *my_app_name = "my app";

    stl_log_init(); // Call once when your app starts.

    // Default log-level is 'info' and default destination is 'stdout'
    stl_log_info("Hello world!\n");

    // Enable logging to file.
    stl_log_set_file("log.0.txt", "log-latest.txt");

    // stdout and file will get the log line
    stl_log_info("to stdout and file!\n");

    // Enable callback
    stl_log_set_callback((void *)my_app_name, log_callback);

    // stdout, file and callback will get the log line
    stl_log_info("to all!\n");

    stl_log_term(); // Call once on shutdown.

    return 0;
}
