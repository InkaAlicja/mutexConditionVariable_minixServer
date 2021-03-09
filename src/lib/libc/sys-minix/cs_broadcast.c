#include <sys/cdefs.h>
#include <lib.h>
#include<stdio.h>
#include <stdlib.h>
#include<string.h>

static int get_cs_endpt(endpoint_t *pt){
    return minix_rs_lookup("cs", pt);
}

int cs_broadcast(int cond_var_id){
    message m;
    m.m_type=BROADCAST;
    m.m1_i2 = cond_var_id;

    endpoint_t CS_PROC_NR;
    if(get_cs_endpt(&CS_PROC_NR) == -1){
        errno = ENOSYS;
        return -1;
    }

    int res = sendrec(CS_PROC_NR,&m);

    if(m.m_type==0){
        return 0;
    }else{
        errno = m.m_type;
        return -1;
    }
}

