#include "inc.h"
#include "globals.h"
endpoint_t who_e;
int call_type;
endpoint_t SELF_E;

static struct {
    int type;
    int (*func)(message *,endpoint_t);
} cs_calls[] = {
        { LOCK, do_lock},
        { UNLOCK, do_unlock},
        { CS_WAIT, do_wait},
        { BROADCAST, do_broadcast},
};

#define SIZE(a) (sizeof(a)/sizeof(a[0]))

/* SEF functions and variables. */
static void sef_local_startup(void);
static int sef_cb_init_fresh(int type, sef_init_info_t *info);
static void sef_cb_signal_handler(int signo);
static void remove_process(endpoint_t who);
static int unblock_process(endpoint_t who);

int last_mutex = 0;
struct m_mutex mutex[MUTEX_NUM];

int last_cv = 0;
struct m_con_var conVar[NR_PROCS];

int main(int argc, char *argv[])
{
    message m;
    /* SEF local startup. */
    env_setargs(argc, argv);
    sef_local_startup();

    while (TRUE) {
        int r;

        if ((r = sef_receive(ANY, &m)) != OK)
            printf("sef_receive failed %d.\n", r);
        who_e = m.m_source;
        call_type = m.m_type;

        /* dispatch message */
        int result=-EBUSY;
        if (call_type == LOCK){
            result = cs_calls[0].func(&m,who_e);
        }else if(call_type == UNLOCK) {
            result = cs_calls[1].func(&m,who_e);
        }else if(call_type == CS_WAIT){
            result = cs_calls[2].func(&m,who_e);
        }else if(call_type == BROADCAST){
            result = cs_calls[3].func(&m,who_e);
        }
        if(who_e == PM_PROC_NR){
            who_e = m.PM_PROC;
            if(call_type == PM_UNPAUSE){
                /* unpause process */
                result = unblock_process(who_e);
            }else{
                /* process died */
                remove_process(who_e);
                result = -EBUSY;
                continue;
            }
        }

        /* Send reply, but only if succeeded or EPERM */
        if(result == -EBUSY)
            continue;
        m.m_type = result;
        if ((r = sendnb(who_e, &m)) != OK)
            printf("CS send error %d.\n", r);

    }
    /* no way to get here */
    return -1;
}
/*===========================================================================*
 *                             sef_local_startup                             *
 *===========================================================================*/
static void sef_local_startup()
{
    /* Register init callbacks. */
    sef_setcb_init_fresh(sef_cb_init_fresh);
    sef_setcb_init_restart(sef_cb_init_fresh);
    /* No live update support for now. */
    /* Register signal callbacks. */
    //sef_setcb_signal_handler(sef_cb_signal_handler);
    /* Let SEF perform startup. */
    sef_startup();
}
/*===========================================================================*
 *                          sef_cb_init_fresh                                *
 *===========================================================================*/
static int sef_cb_init_fresh(int UNUSED(type), sef_init_info_t *UNUSED(info))
{
/* Initialize the cs server. */
    SELF_E = getprocnr();
//if(verbose)
    printf("CS: self: %d\n", SELF_E);

    return(0);
}
/*===========================================================================*
 *                         unblock_process                                   *
 *===========================================================================*/
static int unblock_process(endpoint_t who){
    /* co jest istotne to ze kazdy proces czeka na dany mutex nie wiecej niz raz i na nie wiecej niz jeden mutex */
    /* nie moze tez czekac i na mutex i na zdarzenie */

    /* jesli czeka na mutex - usunac go z kolejki */
    for(int i=0;i<last_mutex;i++){
        for(int j=0;j<NR_PROCS;j++){
            /* nalezy rozwazyc czy dany indeks wgl jest w kolejce */
            if((mutex[i].queue_beg < mutex[i].queue_end && j >= mutex[i].queue_beg && j < mutex[i].queue_end)){
                if(mutex[i].queue[j] == who){
                    int k=j;
                    while(k < mutex[i].queue_end-1){
                        mutex[i].queue[k] = mutex[i].queue[k+1];
                        k++;
                    }
                    mutex[i].queue_end = (mutex[i].queue_end-1)%NR_PROCS;
                    return EINTR;
                }
            }
            if((mutex[i].queue_beg > mutex[i].queue_end && (j < mutex[i].queue_end || j >= mutex[i].queue_beg))){
                if(mutex[i].queue[j] == who){
                    int k=j;
                    while((k < mutex[i].queue_end || k >= mutex[i].queue_beg)){
                        mutex[i].queue[k] = mutex[i].queue[k+1];
                        k++;
                    }
                    mutex[i].queue_end = (mutex[i].queue_end-1)%NR_PROCS;
                    return EINTR;
                }
            }
        }
    }
    /*jesli czeka na zdarzenie - usunac go z kolejki */
    for(int i=0;i<last_cv;i++){
        for(int j=0;j<conVar[i].last;j++){
            if(conVar[i].list[j] == who){
                for(int k=j;k<conVar[i].last-1;k++){
                    conVar[i].list[k] = conVar[i].list[k+1];
                    conVar[i].mutex_list[k] = conVar[i].mutex_list[k+1];
                }
                conVar[i].last--;
                return EINTR;
            }
        }
    }
    return -EBUSY;
}
/*===========================================================================*
 *                          remove_process                                   *
 *===========================================================================*/
static void remove_process(endpoint_t who){
    /* odblokowac - usunac ze wszystkich kolejek */
    unblock_process(who);
    /* jesli ma mutex - oddac go */
    for(int i=0;i<last_mutex;i++){
        if(mutex[i].proc == who){
            message mes;
            mes.m1_i1 = mutex[i].num;
            do_unlock(&mes, who);
        }
    }
}
