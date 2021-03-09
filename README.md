# Serwer mutexów i condition variables

### Mutexy
Funkcje:

* `cs_lock(int mutex_id)` - próbuje zarezerwować mutex o numerze przekazanym w argumencie.
Jeśli mutex nie jest w posiadaniu żadnego procesu zostaje przydzielony procesowi który wywołał funkcję.
W takim przypadku funkcja zwraca `0` (sukces).
Jeśli inny proces jest w posiadaniu mutexu bieżący proces zawiesza się aż do momentu kiedy mutex będzie mógł być mu przydzielony.
W przypadku kiedy proces otrzymuje mutex funkcja zwraca `0` (sukces).
* `cs_unlock(int mutex_id)` - zwalnia mutex o numerze przekazanym w argumencie.
Jeśli wołający proces jest w posiadaniu mutexu, funkcja zwraca `0` (sukces) a serwer mutexów przydziela mutex następnemu procesowi z kolejki procesów oczkujących na ten mutex.
Jeśli proces wołający nie jest w posiadaniu tego mutexu funkcja zwróci `-1` i ustawi `errno` na `EPERM`.

### Condition variables
Funkcje:

* `cs_wait(int cond_var_id, int mutex_id)` - zawiesza bieżący proces w oczekiwaniu na zdarzenie identyfikowane przez `cond_var_id`.
Proces wywołujący tę funkcję powinien być w posiadaniu mutexu identyfikowanego przez `mutex_id`.
Jeśli proces wołający nie posiada odpowiedniego mutexu funkcja zwraca `-1` i ustawia `errno` na `EINVAL`.
Jeśli proces wołający jest w posiadaniu mutexu serwer zwalnia mutex i zawiesza wołający proces aż do czasu gdy jakiś inny proces nie ogłosi zdarzenia `cond_var_id` za pomocą funkcji `cs_broadcast`.
Wówczas serwer ustawia proces w kolejce procesów oczekujących na  mutex `mutex_id` i po otrzymaniu mutexu zwraca `0` (sukces).
*	`cs_broadcast(int cond_var_id)` - ogłasza zdarzenie identyfikowane przez `cond_var_id`. Wszystkie procesy które zawiesiły się w oczekiwaniu na to zdarzenie zostają odblokowane. Każdy z nich po odzyskaniu swojego mutexu zostaje wznowiony.

## Sygnały
Jeśli proces oczekiwał na mutex, to po obsłudze sygnału wznawia oczekiwanie.
Jeśli oczekiwał na zdarzenie to odzyskuje mutex i zwraca sukces (tzw. spurious wakeup).

Mutex'y procesów które zostają zakończone są natychmiast zwalniane.

## Struktura repozytorium
Rozwiązanie przewidziane do działania w systemie MINIX 3.2.1.
- `src/` odzwierciedla fragment struktury katalogów źródeł systemu MINIX.
W celu przetestowania należy skopiować zawartość katalogu `src` z repozytorium do katalogu `/usr/src` w MINIX'e.
Wskutek tego część plików oryginalnych źródeł zostanie nadpisana.
Następnie należy zainstalować include'y, przekompilować i zainstalować bibliotekę standardową oraz nowy serwer.
Aby umożliwić zmiany w podstawowych serwerach systemowych (PM) przed testowaniem należy również przekompilowany obraz systemu.
Po restarcie systemu, nowa funkcjonalność powinna działać po uruchomieniu serwera poleceniem `service up /usr/sbin/cs`.


## ENG:
### Mutexes
Functions:
* `cs_lock(int mutex_id)` - tries to reserve the mutex with the given ID. 
If the mutex is not currently owned by another process, it is assigned to the caller, and the function returns `0` (success).
Otherwise the caller is suspended until the mutex can be assigned to it, and only then the function returns `0` (success).
* `cs_unlock(int mutex_id)` - releases the mutex with the given ID.
If the caller is the owner of the mutex, the function returns `0` (success) and the server assigns the mutex to the next process in the queue. 
Otherwise the function returns `-1` and sets `errno` to `EPERM`.

### Condition variables
Functions:
* `cs_wait(int cond_var_id, int mutex_id)` - suspends current process, awaiting event `cond_var_id`.
The calling process should be the owner of the mutex `mutex_id`, otherwise the function returns `-1` and sets `errno` to `EINVAL`.
If the calling process is the owner of `mutex_id` mutex, the server releases the mutex and suspends the caller until another process calls `cond_var_id` event. In such case, the server enqueues process as awaiting for `mutex_id` mutex and the function returns `0` once the mutex has been obtained.
*	`cs_broadcast(int cond_var_id)` - broadcasts `cond_var_id` event. All processes awaiting `cond_var_id` event are unsuspended and each of them resumes after regaining their mutex.

## Signals
If the process was waiting for a mutex when it received a signal, it resumes waiting after handling it.
If it was waiting for an event, it recovers the mutex and returns success (spurious wakeup).

Mutexes of terminated processes are immediately released.

## Repository structure
The solution is designed to run in MINIX 3.2.1.
- `src /` reflects a fragment of the MINIX source directory structure.
To test, copy the contents of the `src` directory from the repository to the` /usr/src` directory in MINIX.
As a result, some of the original source files will be overwritten.
Then install the includes, recompile and install the standard library and a new server.
To enable changes to the basic system servers (PM), the system image must also be recompiled before testing.
After rebooting the system, the new functionality should work after starting the server with the command `service up /usr/sbin/cs`.

