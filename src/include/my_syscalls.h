#ifndef CONDITIONVARIABLESSERVER_INKAALICJA_MY_SYSCALLS_H
#define CONDITIONVARIABLESSERVER_INKAALICJA_MY_SYSCALLS_H

int cs_lock(int mutex_id);

int cs_unlock(int mutex_id);

int cs_wait(int cond_var_id, int mutex_id);

int cs_broadcast(int cond_var_id);

#endif //CONDITIONVARIABLESSERVER_INKAALICJA_MY_SYSCALLS_H
