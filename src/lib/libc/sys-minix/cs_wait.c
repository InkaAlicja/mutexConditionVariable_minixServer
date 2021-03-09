#include <sys/cdefs.h>
#include <lib.h>
#include<stdio.h>
#include <stdlib.h>
#include<string.h>

static int get_cs_endpt(endpoint_t *pt){
    return minix_rs_lookup("cs", pt);
}

int cs_wait(int cond_var_id, int mutex_id){
    message m;
    m.m_type=CS_WAIT;
    m.m1_i1 = mutex_id;
    m.m1_i2 = cond_var_id;

    endpoint_t CS_PROC_NR;
    if(get_cs_endpt(&CS_PROC_NR) == -1){
        errno = ENOSYS;
        return -1;
    }

    int res = sendrec(CS_PROC_NR, &m);
    while(res == 0 && m.m_type == EINTR){    //poniewaz cs_lock sam nie pozwala sie przerwac,
        res = cs_lock(mutex_id);        // spodziewamy sie ze to sie wykona najwyzej raz
    }

    if(m.m_type==0){
        return 0;
    }else{
        errno = m.m_type;
        return -1;
    }
}


