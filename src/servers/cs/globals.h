#ifndef CONDITIONVARIABLESSERVER_INKAALICJA_GLOBALS_H
#define CONDITIONVARIABLESSERVER_INKAALICJA_GLOBALS_H

#define MUTEX_NUM 1024

extern struct m_mutex{
    int num;
    endpoint_t proc;
    endpoint_t queue[NR_PROCS];
    int queue_beg;
    int queue_end;
} mutex[MUTEX_NUM];

extern int last_mutex;

extern struct m_con_var{
    int num;
    endpoint_t list[NR_PROCS];
    int mutex_list[NR_PROCS];
    int last;
}conVar[NR_PROCS];

extern int last_cv;

#endif //CONDITIONVARIABLESSERVER_INKAALICJA_GLOBALS_H
