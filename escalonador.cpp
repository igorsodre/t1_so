#include "includes.cpp"
using namespace std;
/** variaveis globais **/
int sem; // semaforo

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
	if(!(config.get_shared_memory() == SUCCESS)) return 0; //utilizada pra compartilhar o ambiente de escalonamento
	config.attach_memmory(CREATE_ENV);
	if((sem = sem_create(SEMAPHORE_KEY, 1)) == -1) return 0; // pega o semaforo, caso falhe, encerra o programa
	if((config.pid_runner = fork()) == 0) run_runner(config);
	else run_scheduler(config);

	wait(&v_wait);
	config.remove_shared_memory();
	sem_delete(sem);
	return 0;
}

/**
 * 1) trata as solicitacoes que chegarem na fila de mensagens,
 * e quando chegar o momento de serem executadas, avisa o runner
 * */
void run_scheduler(Config &config){
	Msg msg;
	signal(SIGALRM, dummy_proc);
	int ret;
	int delay;
	while(1){
		ret = config.pop_msg(&msg);
		if(ret == SUCCESS){
			alarm(0);
			build_process(&msg, config); // cria os processos e os adiciona para fila de espera
			delay = get_new_delay(config); // pega novo tempo de espera
			alarm(delay);
		}
		else if(ret == EINTR){ // caso for erro de interrupcao, entao o alarme foi disparado
			alarm(0);
			initialize_processes(config); // muda o processo de esperando para ready
			delay = get_new_delay(config); // pega novo tempo de espera
			alarm(delay);
		}
		else{
			alarm(0);
			cout << RED << "interrupcao desconhecida, possivel perda de mensagem" << RESET << endl;
			delay = get_new_delay(config); // pega novo tempo de espera
			alarm(delay);
		}
	}
	cout << "Sai do loop run scheduler" << endl;
}

/**
 * 2) realizado o escalonamento
 * */
void run_runner(Config &config){
	config.attach_memmory(NOOP);
	while(1){
		P(sem);
		if(!config.t_env->p2_queue.isEmpty()){
			TProcess proc = config.t_env->p2_queue.pop();
			cout << MAG << "Rodando proc: " << proc.id << ", file: " << string(proc.exec_file) << RESET << endl;
		}
		V(sem);
		sleep(10);
	}
	config.detatch_memory();
	exit(0);
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
		strcpy(proc.exec_file, msg->content);
		proc.priority = msg->priority;
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
	if(config.p_fila.size() > 0){
		TProcess proc = config.p_fila.top();
		int delay = difftime(mktime(&proc.when), now);
		return delay > 0 ? delay : 1; // se o horario do processo ja passou, retorna 1 segundo de delay
	}
	else return 0;
}

void display_proc(TProcess &proc){
	time_t now;
	t_time *well;
	time(&now);
	well = localtime(&now);
	cout << GRN << "Process: " << proc.id << ", file: " << string(proc.exec_file) << ", was put to run at: " << my_get_time(well) << RESET << endl;
}

/**
 * 6) move o processo para uma das filas ready;
 * */
void initialize_processes(Config &config){
	if(config.p_fila.size() > 0){
		TProcess proc = config.p_fila.top();
		P(sem);
		display_proc(proc);
		cout << "entrei no switch" << endl;
		switch(proc.priority){
			case 1:
				cout << "entrei no case 1" << endl;
				config.t_env->p1_queue.insert(proc);
				config.p_fila.pop();
				cout << "sai do case 1" << endl;
				break;
			case 2:
				cout << "entrei no case 2" << endl;
				config.t_env->p2_queue.insert(proc);
				config.p_fila.pop();
				cout << "sai do case 2" << endl;
				break;
			case 3:
				cout << "entrei no case 3" << endl;
				config.t_env->p3_queue.insert(proc);
				config.p_fila.pop();
				cout << "sai do case 3" << endl;
				break;
		}
		cout << "sai no switch" << endl;
		V(sem);
	}
}

