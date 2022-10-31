#ifndef ONEGIN_HPP_INCLUDED
#define ONEGIN_HPP_INCLUDED








#include <ctype.h>
#include <sys/stat.h>

#include "logs.hpp"
#include "types/Text.hpp"


#define CHAR_SIZE sizeof (char)
#define LINE_SIZE sizeof (Line)


static Line* const lines_freed = 0;
static char* const   str_freed = 0;


Return_code readfile_into_Text     (const char* file_name, Text* ptrtext);
char*       delete_slash_r         (char* str);
size_t      get_file_len           (FILE* file);
size_t      get_num_rows           (char* str);
char*       slash_n_to_slash_zero  (char* str);
Return_code initialize_lines       (Text* ptrtext);
Text*       initialize_text        (const char* file_name);

size_t      get_lines_len          (Line* lines);

Return_code sort_lines_from_start  (Text* ptrtext);
Return_code sort_lines_from_end    (Text* ptrtext);
Return_code sort_lines_original    (Text* ptrtext);
int         _l_linecmp             (const void* first, const void* second);
int         _r_linecmp             (const void* first, const void* second);
int         _original_linecmp      (const void* first, const void* second);
int         _l_strcmp              (char* first, char* second);
int         _r_strcmp              (char* first, char* second);

Return_code print_lines            (Text* ptrtext);
Return_code print_lines_spaceless  (Text* ptrtext);
Return_code fprint_lines           (Text* ptrtext, const char* file_name, const char* file_mode);
Return_code fprint_lines_spaceless (Text* ptrtext, const char* file_name, const char* file_mode);
bool        isblank                (char* str);
bool        is_no_commands         (const char* str);
bool        is_split               (const char* str);


Return_code cleanmemory            (Text* ptrtext);


void        _mysort                (void* _list, size_t n, size_t size, int ( * comparator ) (const void* first, const void* second));
void        _swap                  (void* first, void* second, size_t size);
int         comp                   (const void* a, const void* b);



















Return_code  readfile_into_Text  (const char* file_name, Text* ptrtext) {

    if (file_name == nullptr) {
        LOG_ERROR   (BAD_ARGS);
        LOG_MESSAGE ("pointer to file_name is leading nowhere\n");
        return      BAD_ARGS;
    }

    if (ptrtext == nullptr) {
        LOG_ERROR   (BAD_ARGS);
        LOG_MESSAGE ("pointer to text is leading nowhere\n");
        return      BAD_ARGS;
    }


    FILE*  file = fopen (file_name, "rb");

    if (file == nullptr) {

        LOG_ERROR (FILE_ERR);
        return     FILE_ERR;
    }


    (*ptrtext).buffer_len = get_file_len (file);


    (*ptrtext).buffer           = (char*) calloc ((*ptrtext).buffer_len + 1, CHAR_SIZE);

    if ((*ptrtext).buffer == nullptr) {
        LOG_ERROR (MEMORY_ERR);
        return     MEMORY_ERR;
    }


    fread                      ((*ptrtext).buffer, CHAR_SIZE, (*ptrtext).buffer_len, file);

    if ( * ((*ptrtext).buffer + (*ptrtext).buffer_len - 1) == '\n') { //func

        * ((*ptrtext).buffer  + (*ptrtext).buffer_len)     = '\n';
        * ((*ptrtext).buffer  + (*ptrtext).buffer_len + 1) = '\0';
    }
    else {

        * ((*ptrtext).buffer + (*ptrtext).buffer_len) = '\0'; // null-terminator
    }
    delete_slash_r            ((*ptrtext).buffer);


    fclose                    (file);


    return SUCCESS;
}


Text*  initialize_text  (const char* file_name) {

    if (file_name == nullptr) {
        LOG_ERROR   (BAD_ARGS);
        LOG_MESSAGE ("pointer to file_name is leading nowhere\n");
        return      nullptr;
    }


    static Text text = {}; // вернуть значение текст


    Return_code return_code = readfile_into_Text (file_name, &text);
    if (return_code) {
        LOG_MESSAGE ("error while reading file into text structure\n");
        return      nullptr;
    }


    text.num_lines     =       get_num_rows (text.buffer);
    if (text.num_lines == 0) { LOG_MESSAGE ("buffer error occured\n"); return nullptr; }


    text.lines     =                     (Line*) calloc (text.num_lines + 1, LINE_SIZE);

    if (text.lines == nullptr) {
        LOG_ERROR (MEMORY_ERR);
        return nullptr;
    }


    initialize_lines                     (&text);
    text.lines[text.num_lines].ptr        = nullptr; // null-terminator

    slash_n_to_slash_zero                (text.buffer);


    return &text;
}


Return_code  sort_lines_from_start  (Text* ptrtext) {

    if (ptrtext        == nullptr) { LOG_ERROR (BAD_ARGS); LOG_MESSAGE ("pointer to text is leading nowhere!\n"); return BAD_ARGS; }
    if (ptrtext->lines == nullptr) { LOG_ERROR (BAD_ARGS); LOG_MESSAGE ("can't sort uninitialized lines!\n");     return BAD_ARGS; }


    _mysort ((*ptrtext).lines, (*ptrtext).num_lines, LINE_SIZE, _l_linecmp);
    return SUCCESS;
}


Return_code  sort_lines_from_end  (Text* ptrtext) {

    if (ptrtext        == nullptr) { LOG_ERROR (BAD_ARGS); LOG_MESSAGE ("pointer to text is leading nowhere!\n"); return BAD_ARGS; }
    if (ptrtext->lines == nullptr) { LOG_ERROR (BAD_ARGS); LOG_MESSAGE ("can't sort uninitialized lines!\n");     return BAD_ARGS; }


    qsort ((*ptrtext).lines, (*ptrtext).num_lines, LINE_SIZE, _r_linecmp);
    return SUCCESS;
}


Return_code  sort_lines_original  (Text* ptrtext) {

    if (ptrtext        == nullptr) { LOG_ERROR (BAD_ARGS); LOG_MESSAGE ("pointer to text is leading nowhere!\n"); return BAD_ARGS; }
    if (ptrtext->lines == nullptr) { LOG_ERROR (BAD_ARGS); LOG_MESSAGE ("can't sort uninitialized lines!\n");     return BAD_ARGS; }


    qsort ((*ptrtext).lines, (*ptrtext).num_lines, LINE_SIZE, _original_linecmp);
    return SUCCESS;
}


int  _l_linecmp  (const void* first, const void* second) {

    assert (first  != nullptr);
    assert (second != nullptr);


    const Line*  first_line  = (const Line*) first;
    const Line*  second_line = (const Line*) second;

    return _l_strcmp (first_line->ptr, second_line->ptr);
}


int  _r_linecmp  (const void* first, const void* second) {

    assert (first  != nullptr);
    assert (second != nullptr);


    const Line* first_line  = (const Line*) first;
    const Line* second_line = (const Line*) second;

    return _r_strcmp (first_line->ptr, second_line->ptr);
}


int  _original_linecmp  (const void* first, const void* second) {

    assert (first  != nullptr);
    assert (second != nullptr);


    const Line* first_line  = (const Line*) first;
    const Line* second_line = (const Line*) second;

    if (first_line->start_index > second_line->start_index)
        return 1;

    if (first_line->start_index < second_line->start_index)
        return -1;

  //if (first_line->start_index > second_line->start_index)
        return 0;
}


Return_code  print_lines  (Text* ptrtext) {

    if (ptrtext == nullptr) { LOG_ERROR (BAD_ARGS); LOG_MESSAGE ("pointer to text is leading nowhere!\n"); return BAD_ARGS; }


    setvbuf (stdout, nullptr, _IOFBF, (*ptrtext).buffer_len);


    size_t ind = 0;
    while ((*ptrtext).lines[ind].ptr != nullptr) {

        printf ("%s\n", (*ptrtext).lines[ind].ptr);
        ind++;
    }

    printf ("--------------------\n");
    return SUCCESS;
}


Return_code  print_lines_spaceless  (Text* ptrtext) {

    if (ptrtext == nullptr) { LOG_ERROR (BAD_ARGS); LOG_MESSAGE ("pointer to text is leading nowhere!\n"); return BAD_ARGS; }


    setvbuf (stdout, nullptr, _IOFBF, (*ptrtext).buffer_len);


    size_t ind = 0;
    while ((*ptrtext).lines[ind].ptr != nullptr) {

        if ( *((*ptrtext).lines[ind].ptr) != '\0')
            printf ("%s\n", (*ptrtext).lines[ind].ptr);
        
        ind++;
    }

    printf ("--------------------\n");
    return SUCCESS;
}


Return_code  fprint_lines  (Text* ptrtext, const char* file_name, const char* file_mode) {

    if (ptrtext   == nullptr)                                { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (file_name == nullptr)                                { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (file_mode == nullptr)                                { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if ( strcmp (file_mode, "a") && strcmp (file_mode, "w")) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    FILE*  destination = fopen  (file_name, file_mode);
    if (destination == nullptr) { LOG_ERROR (FILE_ERR); return FILE_ERR; }


    setvbuf (destination, nullptr, _IOFBF, (*ptrtext).buffer_len);


    size_t ind = 0;
    while ((*ptrtext).lines[ind].ptr != nullptr) {

        fprintf (destination, "%s\n", (*ptrtext).lines[ind].ptr);
        ind++;
    }

    fprintf (destination, "--------------------\n");


    fclose (destination);
    return SUCCESS;
}


Return_code  fprint_lines_spaceless  (Text* ptrtext, const char* file_name, const char* file_mode) {

    if (ptrtext   == nullptr)                                { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (file_name == nullptr)                                { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if (file_mode == nullptr)                                { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }
    if ( strcmp (file_mode, "a") && strcmp (file_mode, "w")) { LOG_ERROR (BAD_ARGS); return BAD_ARGS; }


    FILE*  destination = fopen  (file_name, file_mode);
    if (destination == nullptr) { LOG_ERROR (FILE_ERR); return FILE_ERR; }

    setvbuf (destination, nullptr, _IOFBF, (*ptrtext).buffer_len);


    size_t ind = 0;
    while ((*ptrtext).lines[ind].ptr != nullptr) {

        if ( *((*ptrtext).lines[ind].ptr) != '\0' and !isblank ((*ptrtext).lines[ind].ptr) )
            fprintf (destination, "%s\n", (*ptrtext).lines[ind].ptr);
        
        ind++;
    }


    fprintf (destination, "--------------------\n");


    fclose (destination);
    return SUCCESS;
}


int  _l_strcmp  (char* first, char* second) {

    assert (first  != nullptr);
    assert (second != nullptr);


    size_t ind_first  = 0;
    size_t ind_second = 0;
    while ( !((first[ind_first] == '\0'  &&  second[ind_second] == '\0' ) ||
              (first[ind_first] == '\0'  &&  isalpha(second[ind_second])) ||
              (isalpha(first[ind_first]) &&  second[ind_second] == '\0' )) ) {

        if (!isalpha (first[ind_first]) && first[ind_first]     != '\0') {

            ind_first++;
            continue;
        }

        if (!isalpha (second[ind_second]) && second[ind_second] != '\0') {

            ind_second++;
            continue;
        }


        if (first[ind_first] != second[ind_second])
            return (int) first[ind_first] - second[ind_second];
        ind_first ++;
        ind_second++;
    }


    return (int) first[ind_first] - second[ind_second];
}


int  _r_strcmp  (char* first, char* second) {

    assert (first  != nullptr);
    assert (second != nullptr);


    size_t len_first  = strlen (first);
    size_t len_second = strlen (second);

    size_t ind_first_plus_one  = len_first;
    size_t ind_second_plus_one = len_second;

    while (ind_first_plus_one >= 1 && ind_second_plus_one >= 1) {

        if (!isalpha (first[ind_first_plus_one - 1])) {

            ind_first_plus_one--;
            continue;
        }

        if (!isalpha (second[ind_second_plus_one - 1])) {

            ind_second_plus_one--;
            continue;
        }


        if ( first[ind_first_plus_one - 1] != second[ind_second_plus_one - 1])
            return (int) first[ind_first_plus_one - 1] - second[ind_second_plus_one - 1];
        ind_first_plus_one --;
        ind_second_plus_one--;
    }

    if (ind_first_plus_one == 0 && ind_second_plus_one == 0)
        return  0;
    if (ind_first_plus_one == 0)
        return  1;
  //if (ind_second_plus_one == 0)
        return -1;
}


char*  delete_slash_r  (char* str) {//scientific interest

    assert (str != nullptr);


    for (size_t read = 0, write = 0; str[read-1] != '\0'; ) {

        if (str[read] != '\r') {

            str[write] = str[read];
            read ++;
            write++;
        }

        else {

            read++;
        }
    }


    return str;
}


char*  slash_n_to_slash_zero  (char* str) {

    assert (str != nullptr);


    for (size_t ind = 0; str[ind] != '\0'; ind++) {

        if (str[ind] == '\n') { str[ind] = '\0'; }
    }


    return str;
}


size_t  get_num_rows  (char* str) {

    if (str == nullptr) { LOG_ERROR (BAD_ARGS); LOG_MESSAGE ("pointer to string is leading nowhere!\n"); return 0; }


    size_t num_rows = 1;


    size_t ind = 0;
    for (; str[ind] != '\0'; ind++) {

        if (str[ind] == '\n') { num_rows++; }
    }


    if (ind >= 2) {

        if (str[ind-1] == '\n' && str[ind-2] == '\n') num_rows--;
    }


    return num_rows;
}


Return_code  initialize_lines  (Text* ptrtext) {

    if (ptrtext == nullptr) { LOG_ERROR (BAD_ARGS); LOG_MESSAGE ("pointer to text is leading nowhere!\n"); return BAD_ARGS; }


    bool   addnext  = true;
    size_t line_ind = 0;

    for (size_t source_ind = 0; (*ptrtext).buffer[source_ind] != '\0'; source_ind++) {
        
        if (addnext) {

            (*ptrtext).lines[line_ind].ptr         = &(*ptrtext).buffer[source_ind];
            (*ptrtext).lines[line_ind].start_index =  line_ind;
            (*ptrtext).lines[line_ind].isblank     =  isblank ((*ptrtext).lines[line_ind].ptr);
            addnext                                =  false;
            line_ind                              +=  1;
        }

        if ((*ptrtext).buffer[source_ind] == '\n') { addnext = true; }

    }


    return SUCCESS;
}


Return_code  cleanmemory  (Text* ptrtext) {

    if (ptrtext == nullptr) { LOG_ERROR (BAD_ARGS); LOG_MESSAGE ("pointer to text is leading nowhere!\n"); return BAD_ARGS; }


    free ((*ptrtext).lines);
    free ((*ptrtext).buffer);

    (*ptrtext).lines  = lines_freed;
    (*ptrtext).buffer = str_freed;


    return SUCCESS;
}


void  _mysort  (void* _list, size_t n, size_t size, int ( * comparator ) (const void* first, const void* second)) {

    assert (_list      != nullptr);
    assert (comparator != nullptr);
    assert (size       != 0);

    if (n <= 1) return;


    char* list = (char*) _list;


    size_t  pivot_ind = n/2; // берем pivot
    _swap   ( list + (n - 1) * size, list + pivot_ind * size, size );
    pivot_ind = n-1;


    size_t  left_ind = 0;
    size_t right_ind = n - 1;

    while (left_ind <= right_ind && left_ind  < n - 1 && right_ind > 0) {

        while ( comparator (list + left_ind * size,  list + pivot_ind * size) <= 0 && left_ind  < n - 1) left_ind++;
        while ( comparator (list + right_ind * size, list + pivot_ind * size) >= 0 && right_ind >     0) right_ind--;


        if (left_ind  < right_ind) _swap (list + left_ind * size, list + right_ind * size, size);
    }

    _swap (list + left_ind * size, list + pivot_ind * size, size);

    _mysort                 (list,                            right_ind + 1, size, comparator);
    _mysort                 (list + (right_ind + 1) * size, n-right_ind - 1, size, comparator);
}


void  _swap  (void* first, void* second, size_t size) {

    assert (first  != nullptr);
    assert (second != nullptr);


    long long*  first_8bite = (long long*) first;
    long long* second_8bite = (long long*) second;
    long long _buffer_8bite = 0;

    size_t copied = 0;

    while ((size - copied) >= 8) {

        _buffer_8bite = *second_8bite; 
        *second_8bite = * first_8bite;
        * first_8bite = _buffer_8bite;
        
        first_8bite++; second_8bite++;
        copied += 8;
    }

    long*  first_4bite = (long*)  first_8bite;
    long* second_4bite = (long*) second_8bite;
    long _buffer_4bite = 0;

    while ((size - copied) >= 4) {

        _buffer_4bite = *second_4bite; 
        *second_4bite = * first_4bite;
        * first_4bite = _buffer_4bite;

        first_4bite++; second_4bite++;
        copied += 4;
    }

    char*  first_1bite = (char*)  first_4bite;
    char* second_1bite = (char*) second_4bite;
    char _buffer_1bite = 0;

    while ((size - copied) >= 1) {

        _buffer_1bite = *second_1bite; 
        *second_1bite = * first_1bite;
        * first_1bite = _buffer_1bite;

        first_1bite++; second_1bite++;
        copied += 1;
    }
}


size_t  get_file_len  (FILE* file) {

    if ( file == nullptr ) {
        LOG_ERROR (BAD_ARGS);
        return 0;
    }


    struct stat buffer;
    fstat ( fileno (file), &buffer );


    return buffer.st_size;
}


bool  isblank  (char* str) {

    for (size_t i = 0; str[i] != '\0'; i++) {

        if (isalpha (str[i])) return false;
    }

    return true;
}


bool  is_no_commands  (const char* str) {

    char next = '{';


    for (size_t i = 0; str[i] != '\0'; i++) {

        switch (str[i]) {

            case ' ':  break;
            case '\n': break;
            case '\t': break;
            case '{':  if (next == '{') { next = ';';  break; } else { return false; }
            case ';':  if (next == ';') { next = '}';  break; } else { return false; }
            case '}':  if (next == '}') { next = '\0'; break; } else { return false; }
            default:   return false;
        }
    }


    return true;
}


bool  is_split  (const char* str) {

    char next = 'S';


    for (size_t i = 0; str[i] != '\0'; i++) {

        switch (toupper(str[i])) {

            case ' ':  break;
            case '\n': break;
            case '\t': break;
            case 'S':  if (next == 'S') { next = 'P';  break; } else { return false; }
            case 'P':  if (next == 'P') { next = 'L';  break; } else { return false; }
            case 'L':  if (next == 'L') { next = 'I';  break; } else { return false; }
            case 'I':  if (next == 'I') { next = 'T';  break; } else { return false; }
            case 'T':  if (next == 'T') { next = '\0'; break; } else { return false; }
            default:   return false;
        }
    }


    if (next != '\0') { return false; }


    return true;
}









#endif