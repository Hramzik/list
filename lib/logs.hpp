#ifndef LOGS_HPP_INCLUDED
#define LOGS_HPP_INCLUDED









#include <assert.h>
#include <stdio.h>
#include <time.h>

#include "types/Return_code.hpp"



#define log_file_name "logs.txt"
#define LOG_ERROR(code)      _log_error   (code,    __FILE__, __PRETTY_FUNCTION__, __LINE__)
#define LOG_MESSAGE(message) _log_message (message, __FILE__, __PRETTY_FUNCTION__, __LINE__)


#define try(x) {\
\
    Return_code _return_code_try_special = x;\
    if (_return_code_try_special) { LOG_ERROR (_return_code_try_special); return _return_code_try_special; }\
}



static const size_t time_str_len  = 40;
static const bool   save_old_logs = false;



void       _log_error              (Return_code, const char*, const char*, int);
void       _log_message            (const char* message, const char* file, const char* func, const int line);
void        log_start              (void);
void        log_end                (void);
void        print_log_time         (void);
char*       tm_to_str              (struct tm* time_structure);



















void  _log_message  (const char* message, const char* file, const char* func, const int line) {

    FILE* log_file = fopen (log_file_name, "a");
    setvbuf                (log_file, NULL, _IONBF, 0);


    print_log_time();
    fprintf (log_file, "%s, %s (line %d): %s\n", file, func, line, message);


    fclose (log_file);
}


char*  tm_to_str  (struct tm* time_structure) {

    assert (time_structure != nullptr);


    static char time_str[time_str_len] = "";
    for (size_t i = 0; i < time_str_len; i++) time_str[i] = '\0';


    strftime (  time_str, time_str_len, "%d.%m.%Y %H:%M:%S| ", time_structure);


    return time_str;
}


void  print_log_time  (void) {

    FILE* log_file = fopen (log_file_name, "a");
    setvbuf                (log_file, NULL, _IONBF, 0);

    struct tm*   time_structure = nullptr;
    char*        time_str       = nullptr;
    const time_t cur_time       = time (nullptr);

    time_structure = localtime   (&cur_time);
    time_str       = tm_to_str (time_structure);
    fprintf (log_file, "%s", time_str);


    fclose (log_file);
}


void  log_start  (void) {

    FILE* log_file = fopen (log_file_name, "a");
    setvbuf                (log_file, NULL, _IONBF, 0);


    char log_starter[] = "-------STARTING THE PROGRAM...-------\n\n";

    fprintf (log_file, "%s", log_starter);


    fclose (log_file);


    atexit (log_end);
}


void  log_end  (void) {

    static bool logs_ended_flag = true;

    FILE*  log_file = fopen (log_file_name, "a");
    setvbuf                 (log_file, NULL, _IONBF, 0);


    if (logs_ended_flag) return;

    char log_ender[] = "-------ENDING   THE PROGRAM...-------\n\n";

    fprintf (log_file, "%s", log_ender); logs_ended_flag = true;


    fclose (log_file);
}


void  _log_error  (Return_code _code, const char* file, const char* func, const int line) {

    Return_code code     = _code;
    FILE*       log_file = fopen (log_file_name, "a");
    setvbuf                      (log_file, NULL, _IONBF, 0);

    switch (code) {

    case SUCCESS:
      /*fprintf (log_file, "everything ok!\n");*/                                                                                       break;

    case MEMORY_ERR:
        print_log_time();
        fprintf (log_file, "memory error in file %s in function %s (line %d)\n", file, func, line);                                     break;

    case BAD_ARGS:
        print_log_time();
        fprintf (log_file, "wrong parameters given to the function %s in file %s (line %d)\n", func, file, line);                       break;

    case FILE_ERR:
        print_log_time();
        fprintf (log_file, "file opening error in file %s in function %s (line %d)\n", file, func, line);                               break;

    default:
        print_log_time();
        fprintf (log_file, "wrong error code given to the LOG_ERROR function in file %s in function %s (line %d)\n", file, func, line); break;
    }


    fclose (log_file);
}









#endif