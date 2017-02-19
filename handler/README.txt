we have to call 
Signal(SIGCHLD,handle);
Signal(SIGPIPE,handle);
before infinite for loop in server


handler function
void handle(int signo)