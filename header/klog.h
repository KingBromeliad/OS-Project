#define KLOG_LINES     64     // Number of lines in the buffer. Adjustable, only limited by available memory
#define KLOG_LINE_SIZE 42     // Length of a single line in characters

void klog_print_hex(unsigned int num);
void klog_print(char *str);
void next_char(void);
void next_line(void);