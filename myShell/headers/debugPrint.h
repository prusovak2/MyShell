//#include <stdio.h>
#define COLOR_BLUE    "\x1b[36m"
#define COLOR_RESET   "\x1b[0m"
#define COLOR_RED     "\x1B[31m"
#define COLOR_GREEN   "\x1B[32m"

#define DEBUG 

#ifdef DEBUG
# define DEBUG_PRINT(toPrint...)\
        do {\
            printf (COLOR_BLUE);\
            printf (toPrint);\
            printf (COLOR_RESET);\
        } while (0)
# define UNEXPECTED_PRINT(toPrint...)\
        do {\
            printf (COLOR_RED);\
            printf (toPrint);\
            printf (COLOR_RESET);\
        } while (0)
# define DEBUG_PRINT_GREEN(toPrint...)\
        do {\
            printf (COLOR_GREEN);\
            printf (toPrint);\
            printf (COLOR_RESET);\
        } while (0)

#else
# define DEBUG_PRINT(toPrint...) do {} while (0)
# define UNEXPECTED_PRINT(toPrint...) do {} while(0)
# define UNEXPECTED_PRINT_GREEN(toPrint...) do {} while(0)
#endif