#include <minix/callnr.h>
#include <signal.h>
#include <sys/svrctl.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <minix/com.h>
#include <minix/config.h>
#include <minix/sysinfo.h>
#include <minix/type.h>
#include <minix/vm.h>
#include <string.h>
#include <machine/archtypes.h>
#include <lib.h>
#include <assert.h>
#include "globals.h"

/*===========================================================================*
 *                            get_mutex_own                                  *
 *===========================================================================*/
int get_mutex_own(int mutex_num){
    for(int i=0;i<last_mutex;i++){
        if(mutex[i].num==mutex_num)
            return i;
    }
    return -1;
}
/*===========================================================================*
 *                            mutex_functions                                *
 *===========================================================================*/
int do_lock(message *m, endpoint_t who_e){
    int mutex_num = m->m1_i1;
    int r=get_mutex_own(mutex_num);

    /* jesli ten sam proces poprosi dwa razy o ten sam mutex to zwracamy sukces */
    if(r!=-1 && mutex[r].proc == who_e)
        return 0;

    /* jesli mutex zajety to dodaj do kolejki */
    if(r!=-1 && mutex[r].proc!=-1){
        mutex[r].queue[mutex[r].queue_end]=who_e;
        mutex[r].queue_end = (mutex[r].queue_end+1)%NR_PROCS;
        return EBUSY;
    }

    int set=0;//bool czy nam sie udalo czy musimy dokleic na koniec w mutex[]
    /* uzupelnij w mutex[] pierwsze miejsce gdzie nie ma wlasciciela(bo inaczej skoncza sie kiedys miejsca) */
    for(int i=0;i<last_mutex;i++){
        if(mutex[i].proc==-1){
            mutex[i].num = mutex_num;
            mutex[i].proc = who_e;
            /* kolejka najwyrazniej byla pusta, wiec mozemy dla porzadku przestawic znaczniki */
            mutex[i].queue_beg=0;
            mutex[i].queue_end=0;
            set=1;
            break;
        }
    }
    /* jesli nam sie nie udalo po dorodze znalezc wolnego miejsca to doklejamy na koniec w mutex[] */
    if(!set){
        mutex[last_mutex].num = mutex_num;
        mutex[last_mutex].proc = who_e;
        mutex[last_mutex].queue_beg=0;
        mutex[last_mutex].queue_end=0;
        last_mutex++;
    }
    return 0;
}

int do_unlock(message *m, endpoint_t who_e){
    int mutex_num = m->m1_i1;
    int r=get_mutex_own(mutex_num);

    if(r==-1 || mutex[r].proc!=who_e)
        return EPERM;

    for(int i=0;i<last_mutex;i++){
        if(mutex[i].num==mutex_num){
            if(mutex[i].queue_beg == mutex[i].queue_end){
                mutex[i].proc = -1;
                break;
            }
            mutex[i].proc = mutex[i].queue[mutex[i].queue_beg];
            mutex[i].queue_beg = (mutex[i].queue_beg+1)%NR_PROCS;
            //message m_for_new_owner;
            m->m_type = 0;
            m->m1_i1 = mutex[i].num;
            if ((r = sendnb(mutex[i].proc, m)) != 0) {
                printf("CS send(from do_unlock) error %d.\n", r);
                printf("proces: %d\n",mutex[i].proc);
            }
            break;
        }
    }
    return 0;
}

/*===========================================================================*
 *                      condition_variables_functions                        *
 *===========================================================================*/

int do_wait(message *m, endpoint_t who_e){
    int mutex_num = m->m1_i1;
    int cv_num = m->m1_i2;

    /* sprawdzenie czy ma ten mutex */
    int r = get_mutex_own(mutex_num);
    if(r==-1 || mutex[r].proc!=who_e)
        return EINVAL;

    /* zwolnienie mutexu */
    do_unlock(m,who_e);

    for(int i=0;i<last_cv;i++){
        if(conVar[i].num == cv_num) {
            conVar[i].list[conVar[i].last] = who_e;
            conVar[i].mutex_list[conVar[i].last] = mutex_num;
            conVar[i].last++;
            return EBUSY;
        }
    }
    for(int i=0;i<last_cv;i++){
        if(conVar[i].num == -1){ //zakladam ze condition variables sa dodatnie
            conVar[i].num = cv_num;
            conVar[i].list[conVar[i].last] = who_e;
            conVar[i].mutex_list[conVar[i].last] = mutex_num;
            conVar[i].last++;
            return EBUSY;
        }
    }
    /* nie bylo takiego sygnalu ani wolnego miejsca po drodze */
    conVar[last_cv].last = 0;
    conVar[last_cv].num = cv_num;
    conVar[last_cv].list[conVar[last_cv].last] = who_e;
    conVar[last_cv].mutex_list[conVar[last_cv].last] = mutex_num;
    conVar[last_cv].last++;
    last_cv++;

    return EBUSY;
}

int do_broadcast(message *m, endpoint_t who_e){
    int cv_num = m->m1_i2;

    for(int i=0;i<last_cv;i++) {
        if (conVar[i].num == cv_num) {
            int n = conVar[i].last;
            /* wszystkim ktorzy czekali oglos zdarzenie */
            for(int j=0;j<n;j++){
                m->m1_i1 = conVar[i].mutex_list[j];
                int r = do_lock(m,conVar[i].list[j]);
                if(r==0) {
                    m->m_type = 0;
                    if ((r = sendnb(conVar[i].list[j], m)) != 0) {
                        printf("CS send(from do_broadcast) error %d.\n", r);
                        printf("proces: %d\n", mutex[i].proc);
                    }
                }
            }
            conVar[i].last=0;
            conVar[i].num = -1;
        }
    }
    return 0;
}