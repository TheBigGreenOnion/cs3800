#ifndef HW2_H
#define HW2_h
#endif

typedef struct 
{
    unsigned timestamp;
    bool used;
} meminfo;

typedef struct {
    unsigned prog_id;
    unsigned prog_size;
    unsigned page_size;
    unsigned pages_total;
    unsigned page_begin;
    unsigned page_end;
} pagetable;


typedef struct {
    unsigned mem_id;
    unsigned timestamp;
    bool modify;
    bool clk;
} lookaside;
    
