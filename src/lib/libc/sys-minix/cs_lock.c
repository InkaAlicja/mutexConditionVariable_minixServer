#include <sys/cdefs.h>
#include <lib.h>
#include<stdio.h>
#include <stdlib.h>
#include<string.h>

static int get_cs_endpt(endpoint_t *pt){
    return minix_rs_lookup("cs", pt);
}

int cs_lock(int mutex_id){
    message m;
    m.m_type=LOCK;
    m.m1_i1=mutex_id;

    endpoint_t CS_PROC_NR;
    if(get_cs_endpt(&CS_PROC_NR) == -1){
        errno = ENOSYS;
        return -1;
    }

    int res;
    do{
        m.m_type=LOCK;
        m.m1_i1=mutex_id;
        res = sendrec(CS_PROC_NR,&m);
    }while(res == 0 && m.m_type == EINTR);

    if(m.m_type==0){
        return 0;
    }else{
        errno = m.m_type;
        return -1;
    }
}
