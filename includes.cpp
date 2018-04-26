#include <bits/stdc++.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <signal.h>
#include <unistd.h>
#include <sys/sem.h>
#include <time.h>

using namespace std;

#ifndef INCLUDES_HPP
#define INCLUDES_HPP

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

#define MESG_QUEUE_KEY 130114553
#define SHARED_MEMORY_KEY 130114
#define SEMAPHORE_KEY 1301145
#define MSG_SIZE 100
#define MAX_SIZE_QUEUE 20
#define SUCCESS -42
#define UP_QUEUE 1
#define DOWN_QUEUE 2
#define CREATE_ENV 1
#define NOOP 0

/***************************************************************************************
 **************************** Implementacao de uma fila de tamanho estatico  ************
 ***************************************************************************************/
/* fonte: https://www.sanfoundry.com/cpp-program-implement-circular-queue/ */
template<typename T>
class PQueue
{
	private:
		T cqueue_arr[MAX_SIZE_QUEUE];
		int front = -1;
		int	rear = -1;
	public:
		PQueue<T>() { }
		void start(){
			front = -1;
			rear = -1;
		}

		bool isFull(){
			return (front == 0 && rear == MAX_SIZE_QUEUE-1) || (front == rear+1);
		}

		bool isEmpty(){
			return front == -1;
		}

		/*
		 * Insert into Circular Queue
		 */
		int insert(T item)
		{
			if (isFull()) return 0;
			if (front == -1)
			{
				front = 0;
				rear = 0;
			}
			else
			{
				if (rear == MAX_SIZE_QUEUE - 1)
					rear = 0;
				else
					rear = rear + 1;
			}
			cqueue_arr[rear] = item;
			return 1;
		}
		/*
		 * Delete from Circular Queue
		 */
		T pop()
		{
			T element;
			if (isEmpty()) return element;

			element = cqueue_arr[front];
			if (front == rear)
			{
				front = -1;
				rear = -1;
			}
			else
			{
				if (front == MAX_SIZE_QUEUE - 1)
					front = 0;
				else
					front = front + 1;
			}
			return element;
		}
};

/********************************************************************************************/

/* estrutura utilizada para tranmissao das mensagens do solicitador para o executor*/
typedef struct tm t_time;
typedef struct t_mensagem {
	long nothing;
	int copies;
	t_time horario;
	int priority;
	char content[MSG_SIZE];
} Msg;

/**
 * abstracao dos processos a serem escalonados,
 * com arquivo de execucao, prioridade atual e flag que diz se a prioridade
 * vai aumentar ou diminiur apos a proxima execucao
 * */
class TProcess {
	public:
		int id = -1;
		char exec_file[MSG_SIZE];
		int priority;
		short current_action;
		t_time when;

};
/* definindo o operador < (menor que) para utiilizacao da fila de prioridade */
bool operator<(TProcess a, TProcess b){
	time_t now;
	time(&now);
	return difftime(mktime(&a.when), now) > difftime(mktime(&b.when), now);
}

/**
 * ambiente de execucao, com as 3 filas de prioridades round robin e variaveis de controle
 * */
class TEnvironment {
	public:
		int pid_seq = 1;
		PQueue<TProcess> p1_queue;
		PQueue<TProcess> p2_queue;
		PQueue<TProcess> p3_queue;
};

/**
 * estrutura de controle programas,
 * utilizada para unificar valores e procedimentos utilizados por todos os processos
 * */
class Config {
	public:
		int id_fila = -1;
		int pid_runner;
		int mem_id; // indentificador da memoria compartilahda
		TEnvironment *t_env;
		priority_queue<TProcess> p_fila; //fila de espera de processos ordenada por hora de execucao mais proxima
		Config(){}; //constructor

		int initialize_config(){ //inicializa configuracoes iniciais
			ios::sync_with_stdio(false);
			if(id_fila < 0) return start_queue();
			else return 1;
		}

		int get_shared_memory(){ // aloca um segmento de memoria compartilhada
			if( (mem_id = shmget(SHARED_MEMORY_KEY, sizeof(TEnvironment), IPC_CREAT|0666)) == -1) {
				cout << RED << "Falha ao alocar memoria compartilhada" << RESET << endl;
				return errno;
			}
			else return SUCCESS;
		}

		void attach_memmory(int operation){ // fixa o segmento de memoria compartilhada
			t_env = (TEnvironment *) shmat(mem_id, (char *)0, 0);
			if(operation == CREATE_ENV){
				t_env->p1_queue.start();
				t_env->p2_queue.start();
				t_env->p3_queue.start();
			}
		}

		void detatch_memory(){ // desfixa o segmento de memoria compartilhada
			shmdt((void*)t_env);
		}

		void remove_shared_memory(){ // libera o segmento de memoria compartilhada alocado anteriormente
			shmctl(mem_id, IPC_RMID, NULL);
		}

		int start_queue(){ // cria uma fila de mensagem
			this->id_fila = msgget(MESG_QUEUE_KEY, IPC_CREAT|0666);
			if(id_fila < 0) {
				cout << RED << "Falha ao criar fila de mensagem" << RESET << endl;
				return 0;
			}
			return 1;
		}

		void destroy_queue(){ // libera a fila de mensagens
			msgctl(id_fila, IPC_RMID, NULL);
		}

		int get_queue(){ // retorna o id da fila de mensagens
			return this->id_fila;
		}

		void push_msg(Msg msg){ // insere uma mensagem na fila de mensagens
			msgsnd(id_fila, &msg, sizeof(Msg)-sizeof(long), 0);
		}

		int pop_msg(Msg *msg){ // busca uma mensagm na fila
			cout << BLU << "esperando mensagem..." << RESET << endl;
			if(msgrcv(id_fila, msg, sizeof(Msg)-sizeof(long), 0, 0) < 0) {
				cout << RED << "Falha na espera de mensagem" << RESET << endl;
				return errno;
			}
			else {
				cout << GRN << "Mensagem recebida" << RESET << endl;
				return SUCCESS;
			}
		}

		/* void f_teste(int display){ // nome autoexplicativo, qualquer codigo para procedimento de teste eh colocado aqui */
		/* } */
};

// mostra o conteudo da estrutura de mensagens
void display_msg(Msg *msg){
	cout << "horario de execucao: " << msg->horario.tm_hour << " horas, " << msg->horario.tm_min << " minutos, " << msg->horario.tm_sec << " segundos" << endl;
	cout << "copias: " << msg->copies << ", arquivo a executar: " << string(msg->content) << ", prioridade: " <<msg->priority << endl;
}

string my_get_time(t_time *horario){
	string retorno = to_string(horario->tm_hour) + ":" + to_string(horario->tm_min) + ":" + to_string(horario->tm_sec);
	return retorno;
}
// verifica se o valor passado esta contido no vetor passado
bool in_array(const std::string &value, const std::vector<string> &array)
{
	return std::find(array.begin(), array.end(), value) != array.end();
}

// Retira os espacoes em branco de strings
std::string trim(const std::string& str,
		const std::string& whitespace = " \t")
{
	const auto strBegin = str.find_first_not_of(whitespace);
	if (strBegin == std::string::npos)
		return ""; // no content

	const auto strEnd = str.find_last_not_of(whitespace);
	const auto strRange = strEnd - strBegin + 1;

	return str.substr(strBegin, strRange);
}

std::string reduce(const std::string& str,
		const std::string& fill = " ",
		const std::string& whitespace = " \t")
{
	// trim first
	auto result = trim(str, whitespace);

	// replace sub ranges
	auto beginSpace = result.find_first_of(whitespace);
	while (beginSpace != std::string::npos)
	{
		const auto endSpace = result.find_first_not_of(whitespace, beginSpace);
		const auto range = endSpace - beginSpace;

		result.replace(beginSpace, range, fill);

		const auto newStart = beginSpace + fill.length();
		beginSpace = result.find_first_of(whitespace, newStart);
	}
	return result;
}

string join(const vector<string>& vec, const char* delim)
{
	stringstream res;
	copy(vec.begin(), vec.end(), ostream_iterator<string>(res, delim));
	return res.str();
}

/*******************************************************************************
 * separa a string em um array de strings com separacao definida pelo
 * delimitador passado como argumento
 * *****************************************************************************/
void split(const string &s, const char* delim, vector<string> & v){
	// to avoid modifying original string
	// first duplicate the original string and return a char pointer then free the memory
	v.clear();
	char * dup = strdup(s.c_str());
	char * token = strtok(dup, delim);
	while(token != NULL){
		v.push_back(string(token));
		// the call is treated as a subsequent calls to strtok:
		// the function continues from where it left in previous invocation
		token = strtok(NULL, delim);
	}
	free(dup);
}

/**************************************************************************
 * passa string para sua uppercase version
 * *************************************************************************/
void to_uppercase(string &str){
	std::transform(str.begin(), str.end(),str.begin(), ::toupper);
}

/*******************************************************************************
 * verifica se string eh um numero Inteiro
 * *****************************************************************************/
bool is_number(const std::string& s)
{
	string str;
	str = s;
	if(s[0] == '-')
		str = s.substr(1, s.length()+1);
	return !str.empty() && std::find_if(str.begin(),
			str.end(), [](char c) { return !std::isdigit(c); }) == str.end();
}


/**
 * 0) obtem o semaforo, caso falha avisa na tela
 * */
int sem_create(key_t key, int initval)  
{
	int semid ;

	union semun {
		int val ;
		struct semid_ds *buf ;
		ushort array[1] ;
	} arg_ctl ;

	semid = semget(ftok("includes.cpp",key),1,IPC_CREAT|IPC_EXCL|0666) ;
	if (semid == -1) {
		semid = semget(ftok("includes.cpp",key),1,0666) ; 
		if (semid == -1) {
			perror("Erro semget()") ;
			return -1;
		}
	}

	arg_ctl.val = initval ;
	if (semctl(semid,0,SETVAL,arg_ctl) == -1) {
		perror("Erro inicializacao semaforo") ;
		return -1;
	}
	return(semid) ;
}
/* 0) */
void P(int semid)
{
	struct sembuf sempar[1];
	sempar[0].sem_num = 0 ;
	sempar[0].sem_op = -1 ;
	sempar[0].sem_flg = SEM_UNDO ;
	if (semop(semid, sempar, 1) == -1)
		perror("Erro operacao P") ;
}

/* 0) */
void V(int semid)
{
	struct sembuf sempar[1];
	sempar[0].sem_num = 0 ;
	sempar[0].sem_op =  1 ;
	sempar[0].sem_flg = SEM_UNDO ;
	if (semop(semid, sempar, 1) == -1)
		perror("Erro operacao V") ;
}

/* 0) */
void sem_delete(int semid) 
{
	if (semctl(semid,0,IPC_RMID,0) == -1)
		perror("Erro na destruicao do semaforo");
}
#endif

