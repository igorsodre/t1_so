#include "includes.cpp"
using namespace std;


/* 1)*/
void run_scheduler(Config &config);

/* 2) */
void run_runner(Config &config);


int main()
{
	int v_wait;
	Config config;
	config.initialize_config();
	config.get_shared_memory(); //utilizada pra compartilhar o ambiente de escalonamento

	if((config.pid_runner = fork()) == 0) run_scheduler(config);
	else run_runner(config);

	wait(&v_wait);
	config.remove_shared_memory();
	return 0;
}


/**
 * 1) trata as solicitacoes que chegarem na fila de mensagens,
 * e quando chegar o momento de serem executadas, avisa o runner
 * */
void run_scheduler(Config &config){
	/* Msg msg; */
	config.attach_memmory();
	/* int ret; */
	/* while(1){ */
	/* 	ret = config.pop_msg(&msg); */
	/* 	if(ret == SUCCESS){ */
	/* 		build_process(&msg); */
	/* 	} */
	/* } */
	config.f_teste(1);
	config.detatch_memory();
	exit(0);
}

/**
 * 2) realizado o escalonamento
 * */
void run_runner(Config &config){
	config.attach_memmory();
	config.f_teste(2);
}
