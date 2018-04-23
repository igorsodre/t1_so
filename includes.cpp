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

#define MESG_QUEUE_NAME 130114553
#define SHARED_MEMORY_KEY 130114
#define MSG_SIZE 100
#define SUCCESS -42
#define UP_QUEUE 1
#define DOWN_QUEUE 2

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
		int id;
		string exec_file;
		int priotity;
		short current_action;
		t_time when;

		bool operator<(TProcess &b){
			time_t now;
			time(&now);
			return difftime(now, mktime(&when)) < difftime(now, mktime(&b.when));
		}
};

/**
 * ambiente de execucao, com as 3 filas de prioridades round robin e variaveis de controle
 * */
class TEnvironment {
	public:
		int pid_seq = 1;
		queue<TProcess> p1_queue;
		queue<TProcess> p2_queue;
		queue<TProcess> p3_queue;
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

		void initialize_config(){ //inicializa configuracoes iniciais
			ios::sync_with_stdio(false);
			if(id_fila < 0) start_queue();
		}

		void get_shared_memory(){ // aloca um segmento de memoria compartilhada
			mem_id = shmget(SHARED_MEMORY_KEY, sizeof(TEnvironment), IPC_CREAT|0666);
		}

		void attach_memmory(){ // fixa o segmento de memoria compartilhada
			t_env = (TEnvironment *) shmat(mem_id, (char *)0, 0);
		}

		void detatch_memory(){ // desfixa o segmento de memoria compartilhada
			shmdt((void*)t_env);
		}

		void remove_shared_memory(){ // libera o segmento de memoria compartilhada alocado anteriormente
			shmctl(mem_id, IPC_RMID, NULL);
		}

		void start_queue(){ // cria uma fila de mensagem
			this->id_fila = msgget(MESG_QUEUE_NAME, IPC_CREAT|0666);
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
			if(msgrcv(id_fila, msg, sizeof(Msg)-sizeof(long), 0, 0) < 0) return errno;
			else return SUCCESS;
		}

		/* void f_teste(int display){ // nome autoexplicativo, qualquer codigo para procedimento de teste eh colocado aqui */
		/* } */
};

// mostra o conteudo da estrutura de mensagens
void display_msg(Msg *msg){
	cout << "horario de execucao: " << msg->horario.tm_hour << " horas, " << msg->horario.tm_min << " minutos, " << msg->horario.tm_sec << " segundos" << endl;
	cout << "copias: " << msg->copies << ", arquivo a executar: " << string(msg->content) << ", prioridade: " <<msg->priority << endl;
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
#endif

