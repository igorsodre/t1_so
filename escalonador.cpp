#include "includes.cpp"
using namespace std;
/** variaveis globais **/
int sem; // semaforo
Config *dismiss;

/* 1)*/
void run_scheduler(Config &config);

/* 2) */
void run_runner(Config &config);

/* 3) */
void dummy_proc(int i);
void dummy_proc2(int i);

/* 4) */
void build_process(Msg *msg, Config &config);

/* 5) */
int get_new_delay(Config &config);

/* 6) */
void initialize_processes(Config &config);

/* 7) */
void run_process(Config &config, TProcess proc);

/* 8) */
void list_proc(Config &config);

/* 9) */
void display_proc(TProcess &proc);

/* 10) */
void remove_proc(Config &config, int proc_id);

/* 11) */
void shut_down_scheduler(Config config);

/* 12) */
void shut_down_runner(int i);

/* 13) */
void display_proc_run_information(Config *config);

int main()
{
	Config config;
	config.initialize_config();
	if(!(config.get_shared_memory() == SUCCESS)) return 0; //utilizada pra compartilhar o ambiente de escalonamento
	config.attach_memmory(CREATE_ENV);
	if((sem = sem_create(SEMAPHORE_KEY, 1)) == -1) return 0; // pega o semaforo, caso falhe, encerra o programa
	if((config.pid_runner = fork()) == 0) run_runner(config);
	else run_scheduler(config);

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
			switch(msg.nothing){
				case CREATE_PROC:
					build_process(&msg, config); // cria os processos e os adiciona para fila de espera
					break;
				case LISTA_PROC:
					list_proc(config); // lista os processos da fila de espera
					break;
				case REMOVE_PROC:
					remove_proc(config, msg.priority); // remove o processo passado como argumento
					break;
				case SHUTDOWN:
					cout << "recebi um SHUTDOWN" << endl;
					shut_down_scheduler(config); // encerra o programa
					break;
			}
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
}

/**
 * 2) realizado o escalonamento
 * */
void run_runner(Config &config){
	config.attach_memmory(NOOP);
	dismiss = &config; // seta variavel global para rotina de encerramento
	siginterrupt(SIGALRM, 1);
	siginterrupt(SIGTERM, 1);
	signal(SIGALRM, dummy_proc2);
	signal(SIGTERM, shut_down_runner);

	while(1){
		P(sem);
		if(!config.t_env->pqueue[1].isEmpty()){ // round robin com fila 1 > fila 2 > fila 3
			TProcess proc = config.t_env->pqueue[1].pop();
			V(sem);
			run_process(config, proc);
		}else if(!config.t_env->pqueue[2].isEmpty()){
			TProcess proc = config.t_env->pqueue[2].pop();
			V(sem);
			run_process(config, proc);
		}else if(!config.t_env->pqueue[3].isEmpty()){
			TProcess proc = config.t_env->pqueue[3].pop();
			V(sem);
			run_process(config, proc);
		}else {
			V(sem);
			sleep(2); // se nenhuma fila tem processos, dorme um pouco
		}
	}
	exit(0);
}

/**
 * 3) procedimento dummy.
 * */
void dummy_proc(int i){ i++;}
void dummy_proc2(int i){ i++; }

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
		display_proc(proc);
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

/**
 * 9) mostra informacao do processo
 * */
void display_proc(TProcess &proc){
	time_t now;
	t_time *well;
	time(&now);
	well = localtime(&now);
	cout << GRN << "ID: " << proc.id << "\t | arquivo executavel: " << string(proc.exec_file) << "\t| horario de execucao: " << my_get_time(&proc.when) << RESET << endl;
}

/**
 * 6) move o processo para uma das filas ready;
 * */
void initialize_processes(Config &config){
	if(config.p_fila.size() > 0){
		TProcess proc = config.p_fila.top();
		P(sem);
		switch(proc.priority){ // seta o momento do fluxo circular de transito na filas de prioridade que o processo vai comecar
			case 1:
				proc.current_action = FILA_1_1;
				break;
			case 2:
				proc.current_action = FILA_2_1;
				break;
			case 3:
				proc.current_action = FILA_3_1;
				break;
		}
		config.t_env->pqueue[proc.priority].insert(proc);
		config.p_fila.pop();
		V(sem);
	}
}

/**
 * 7) roda o processo ate que ele termine ou por 5 segundos
 * se nao terminou deolve-o para uma das filas round robin
 * */
void run_process(Config &config, TProcess proc){
	int v_pid;
	int ret, v_wait, counter;
	int what_queue[8] = {1, 1, 2, 2, 3, 3, 2, 2};

	if(proc.active){ // se o processo esta ativo, so precisa continuar sua execucao
		v_pid = proc.pid;
		proc.current_action = (proc.current_action + 1) % STATES_SIZE;
		counter = QUANTUM;
		config.current_running_process = proc;
		kill(v_pid, SIGCONT);
		while(counter--){
			sleep(1);
			ret = waitpid(v_pid, &v_wait, WNOHANG);
			if(ret > 0) break;
		}
		if(ret ==  0){
			kill(v_pid, SIGSTOP); // para o processo
			P(sem);
			// recoloca o processo na fila correta
			config.t_env->pqueue[what_queue[proc.current_action]].insert(proc);
			V(sem);
		} else { // se nao teve erro entao o processo encerrou
			cout << GRN << "Processo: " << proc.id << " terminou execucao." << RESET << endl;
			// TODO: procedimento de encerramento de processo
		}
	}
	else { // se o processo nao esta ativo entao ele ainda nao foi criado
		int v_pid = fork();
		if(v_pid == 0){ // codigo para colocar o programa para executar
			string right_path = "./" + string(proc.exec_file);
			if(execl(right_path.c_str(), proc.exec_file, (char *) 0) < 0){
				cout << "Nao foi possivel executar o arquivo: " << right_path << ", programa deve ser reagendado." << endl;
				exit(0);
			}
		}
		else{
			proc.pid = v_pid;
			proc.active = true; // marca o processo como rodando
			proc.current_action = (proc.current_action + 1) % STATES_SIZE;
			counter = QUANTUM;
			config.current_running_process = proc;
			while(counter--){
				sleep(1);
				ret = waitpid(v_pid, &v_wait, WNOHANG);
				if(ret > 0) break;
			}
			if(ret == 0){
				kill(v_pid, SIGSTOP); // para o processo
				P(sem);
				// recoloca o processo na fila correta
				config.t_env->pqueue[what_queue[proc.current_action]].insert(proc);
				V(sem);
			} else { // se nao teve erro entao o processo encerrou
				cout << GRN << "Processo: " << proc.id << " terminou execucao." << RESET << endl;
				// TODO: procedimento de encerramento de processo
			}
		}
	}
}

/**
 * 8) lista todos os processos que estao na fila de espera para serem executados
 * */
void list_proc(Config &config){
	vector<TProcess> v;
	TProcess proc;
	cout << "\tjob_id\t\t| arq_exec\t\t|hh:mm\t\t|pri" << endl;
	while(!config.p_fila.empty()){
		proc = config.p_fila.top();
		cout << "\t" << proc.id << "\t\t|" << string(proc.exec_file) << "\t\t\t|" << my_get_time(&proc.when) << "\t\t|" << proc.priority << endl;
		v.push_back(proc);
		config.p_fila.pop();
	}
	while(!v.empty()){
		config.p_fila.push(v.back());
		v.pop_back();
	}
}

/**
 * 10) remove o processo da fila de espera
 * */
void remove_proc(Config &config, int proc_id){
	vector<TProcess> v;
	TProcess proc;
	while(!config.p_fila.empty()){
		proc = config.p_fila.top();
		if(proc.id != proc_id) v.push_back(proc); // nao insere no arrai o processo que quer remover
		config.p_fila.pop();
	}
	while(!v.empty()){
		config.p_fila.push(v.back());
		v.pop_back();
	}
}

/**
 * 11) remove os macanismos IPC e encerra o programa
 * */
void shut_down_scheduler(Config config){
	int v_wait;
	kill(config.pid_runner, SIGTERM);
	wait(&v_wait);
	config.detatch_memory();
	config.remove_shared_memory();
	config.destroy_queue();
	sem_delete(sem);
	exit(0);
}

/**
 * 12) rotina de encerramento do processo runner
 * */
void shut_down_runner(int i){
	i++;
	int v_wait, ret;
	TProcess proc;
	kill(dismiss->current_running_process.pid, SIGKILL);
	for(int index = 1; index < 4; index++){ // mata todos os processos em todas as filas
		while(dismiss->t_env->pqueue[index].isEmpty()){
			proc = dismiss->t_env->pqueue[index].pop();
			kill(proc.pid, SIGKILL);
		}
	}
	while(1){ // da wait em todos os processos matados anteriormente
		ret = wait(&v_wait);
		if(ret == -1 && errno == ECHILD) break;
	}
	display_proc_run_information(dismiss);
	dismiss->detatch_memory();
	exit(0); // finaliza execucao
}
/**
 * 13) mostra as informacoes de todos os procesos que foram efetivamente executados
 * */
void display_proc_run_information(Config *config){
	cout << endl << "printei informacao dos processos" << endl;
}
