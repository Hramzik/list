#ifndef TEXT_TYPE_HPP_INCLUDED
#define TEXT_TYPE_HPP_INCLUDED









struct Line_structure {

    char*  ptr = nullptr;
    size_t len = 0;
    size_t start_index = 0;
    bool   isblank = false;
};


typedef struct Line_structure Line;


struct Text_structure {

    Line*  lines = nullptr;
    size_t num_lines = 0;
    char*  buffer = nullptr;
    size_t buffer_len = 0;
};


typedef struct Text_structure Text;









#endif