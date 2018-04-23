#include "includes.cpp"
using namespace std;


/* 1)*/
void run_scheduler(Config &config);

/* 2) */
void run_runner(Config &config);

/* 3) */
void dummy_proc(int i);

/* 4) */
void build_process(Msg *msg, Config &config);

/* 5) */
int get_new_delay(Config &config);

/* 6) */
void initialize_processes(Config &config);

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
	Msg msg;
	signal(SIGALRM, dummy_proc);
	config.attach_memmory();
	int ret;
	int delay;
	while(1){
		ret = config.pop_msg(&msg);
		if(ret == SUCCESS){
			build_process(&msg, config); // cria os processos e os adiciona para fila de espera
			delay = get_new_delay(config); // pega novo tempo de espera
			alarm(delay);
		}
		else if(ret == EINTR){ // caso for erro de interrupcao, entao o alarme foi disparado
			initialize_processes(config);
			delay = get_new_delay(config);
			alarm(delay);
		}
	}
	config.detatch_memory();
	exit(0);
}

/**
 * 2) realizado o escalonamento
 * */
void run_runner(Config &config){
	config.attach_memmory();
}

/**
 * 3) procedimento dummy.
 * */
void dummy_proc(int i){}

/**
 * 4) monta um processo a partir da mensagem recebida
 * */
void build_process(Msg *msg, Config &config){
	for(int i = 0; i < msg->copies; i++){
		TProcess proc;
		proc.id = config.t_env->pid_seq++; // seta um processo id e incrementa o contador de processos
		proc.exec_file = string(msg->content);
		proc.priotity = msg->priority;
		proc.when = msg->horario;
		config.p_fila.push(proc);
	}

}

/**
 * 5) recupera o novo delay do alarm a partir do proximo processo na fila de espera
 * */
int get_new_delay(Config &config){
	time_t now;
	time(&now);
	TProcess proc = config.p_fila.top();
	int delay = difftime(now, mktime(&proc.when));
	return delay > 0 ? delay : 1; // se o horario do processo ja passou, retorna 1 segundo de delay
}

/**
 * 6) move o processo para uma das filas ready;
 * */
void initialize_processes(Config &config){
	alarm(0);
	// perguntar sobre o semaforo aqui
}
