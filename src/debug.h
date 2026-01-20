#ifdef MYALLOC_DEBUG
    #define DBG(...) fprintf(stderr, __VA_ARGS__)
#else
    #define DBG(...) do { } while (0)
#endif