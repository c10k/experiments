#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <stdexcept>
#include <unistd.h>
#include <string.h>

typedef void Sigfunc(int);

Sigfunc *signal(int signo, Sigfunc *func)
{
	struct sigaction act,oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if(signo == SIGALRM){
#ifdef SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;
#endif
	} else{
#ifdef SA_RESTART
		act.sa_flags |= SA_RESTART;
#endif
	}
	if(sigaction(signo, &act, &oact) < 0){
		return (SIG_ERR);
	}
	return (oact.sa_handler);

}

Sigfunc *
Signal(int signo, Sigfunc *func)
{
	Sigfunc	*sigfunc;

	if ( (sigfunc = signal(signo, func)) == SIG_ERR){
		const char buf[] = "signal error\n";
		write(STDOUT_FILENO, buf, strlen(buf));
		exit(1);
	}
	return(sigfunc);
}

void handle(int signo)
{
	pid_t pid;
	int stat;
	switch(signo){
		case SIGCHLD:
		while( (pid = waitpid(-1, &stat, WNOHANG)) > 0);
		break;
		case SIGPIPE:
		throw std::runtime_error("Broken pipe.");
		break;
		case SIGINT:
		const char buf[] = "\nExiting now...\n";
		write(STDOUT_FILENO, buf, strlen(buf));
		exit(0);
		break;

		//more signals to be added here
	}
		return;
}
