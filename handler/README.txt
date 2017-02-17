we have to call

Signal(SIGCHLD,sig_chld);

in server after listen() call and before infinite for loop. This will clean the zombie child server processes.





handler function::

void sig_chld(int signo)