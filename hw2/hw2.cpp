#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include "hw2.h"

using namespace std;


int pagefault(lookaside* main_lookaside, int access_id, int mem_size, char *algorithm, unsigned *timestamp, char *prepage, int* clk)
{
    int replace = main_lookaside[0].timestamp;
    int k = *clk;
    if (string(algorithm) == "clock") {
        while (main_lookaside[k].clk == 1) {
             
            main_lookaside[k].clk = 0;
            if (k = 255)
                k = 0;
            else 
                k++;
        }
        replace = k;
        main_lookaside[replace].clk = 1;
        *clk = replace+1;
    }
    else { //fifo/lru 
        replace = 0;
        for (k=1;k<mem_size;k++) {
            if (main_lookaside[k].timestamp < main_lookaside[replace].timestamp)
                replace = k;
        }

        main_lookaside[replace].timestamp = *timestamp;
    }

    main_lookaside[replace].mem_id = access_id;
    if (prepage[0] == '1') {
        *timestamp += 1;
        replace += 1;
        *clk += 1;
        
        if ((replace < mem_size)) {
            main_lookaside[replace].mem_id = access_id + 1;
            main_lookaside[replace].timestamp = *timestamp;
            main_lookaside[replace].clk = 1;
        }
    }
    
    //either demand or prepaging
    return 0;
}

int check (lookaside* reference, int mem_size, int page_size, int access_id, char* algorithm, unsigned *timestamp)
{
    int hit = -1;
    int frame = (access_id / page_size) ;
    for (int k=0;k<mem_size;k++) {
        if ((reference[k].mem_id / page_size) == frame) {
            hit = k;
            if (string(algorithm) == "lru")
                reference[k].timestamp = *timestamp;
            else if (string(algorithm) == "clock") {
                reference[k].clk = 1;
            }
        }
    }
    
    return hit;
}

int lookup(pagetable** ptables, int pid, int offset)
{
    int abs;
    if (ptables[pid]->prog_size >= offset) {
        abs = int(ptables[pid]->page_begin + offset);
    }
    else {
        abs = -1;
    }
    return abs;
}

pagetable* gen_ptable(unsigned pid, unsigned mem_used, unsigned total_allocated, unsigned page_sz){
    pagetable *temp;
    temp = new pagetable;
    temp->prog_id = pid;
    temp->prog_size = mem_used;
    temp->page_size = page_sz;
    temp->pages_total = mem_used / page_sz;
    temp->page_begin = total_allocated;
    temp->page_end = total_allocated + mem_used;
    
/*    cout << "id: " << temp->prog_id << endl
        << "prog_size: " << temp->prog_size <<endl
        << "page_size: " << temp->page_size <<endl
        << "pages_total: " << temp->pages_total <<endl
        << "page_begin: " << temp->page_begin <<endl
        << "page_end: " << temp->page_end <<endl;
*/
    return temp;
}

int main(int argc, char *argv[])
{
    unsigned MEM_SZ = 512;
    int pagefaults = 0;
    unsigned current_timestamp = 1;
    
    if (argc > 6) {
        //iamerror();
        cout << "HOLY SHIT BRO" << endl;
        return 0;
    }

    char* procs_fname = argv[1];
    char* access_fname = argv[2];
    unsigned page_size = atoi(argv[3]); 
    char *algorithm = argv[4];
    char *prepage = argv[5];
    int mem_pages = MEM_SZ/page_size;    

    std::ifstream procinfo (procs_fname, std::ifstream::in);
    std::ifstream procaccess (access_fname, std::ifstream::in);

    int ** mem = NULL; 
    int ** old_mem = NULL;
    pagetable **ptables = NULL;
    pagetable **old_ptables = NULL;
    unsigned pid = 0;
    unsigned mem_used = 0;
    unsigned total_procs = 0;
    unsigned total_allocated = 0;

    cout << "Page size: " << page_size <<endl
        << "Algorithm: " << algorithm << endl
        << "Paging Policy: "<< prepage << endl;

    cout << (string(algorithm) == "lru" ) << endl;
    while (!procinfo.eof()) {
        //cout << total_procs << endl;
        procinfo >> pid;
        procinfo >> mem_used;

        old_mem = mem;
        old_ptables = ptables;

        mem = new int*[total_procs+1];
        ptables = new pagetable*[total_procs+1];

        if (total_procs > 0) {
            for (int i=0;i<total_procs;i++) {
                mem[i] = old_mem[i];
                ptables[i] = old_ptables[i];
            }
            delete [] old_mem;
            delete [] old_ptables;
        }   

        mem[total_procs] = new int[mem_used];

        ptables[total_procs] = gen_ptable(pid, mem_used, total_allocated, page_size);

        total_procs++;
        total_allocated += mem_used; 

    }
    
    lookaside* main_lookaside = new lookaside[mem_pages];
    for (int k=0;k<mem_pages;k++) {
        main_lookaside[k].mem_id = 0;
        main_lookaside[k].timestamp = 0;
        main_lookaside[k].modify = 0;
        main_lookaside[k].clk = 0;
    }
    
    int offset;
    int status; //0 = = good, other = fault
    int access_id;
    int clock_ptr = 0;
    string derp; 
    while (!procaccess.eof()) {
        procaccess >> pid;
        procaccess >> offset;
        procaccess.ignore(64, '\n');
        procaccess.sync();

        access_id = lookup(ptables, pid, offset);
      
        status = check(main_lookaside, mem_pages, page_size, access_id, algorithm, &current_timestamp);
        if (status == -1) {
            pagefaults += 1;
            pagefault(main_lookaside, access_id, mem_pages, algorithm, &current_timestamp, prepage, &clock_ptr); 
        }
        current_timestamp ++;
    }
    cout << "Page faults: " << pagefaults << endl;
    // Otherwise, we are good to go!

    for (int j=0;j<total_procs;j++) {
        delete [] mem[j];
        delete ptables[j];
    }
    delete [] mem;
    delete [] ptables;
    delete [] main_lookaside;
    return 0;
}
