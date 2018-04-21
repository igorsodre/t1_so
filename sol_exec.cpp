#include "includes.cpp"
using namespace std;

/* 1) */
int validate_arguments(int argc, char *argv[]);
/* 2) */
void schedule_process(Config &config, char *argv[]);
/* 3) */
Msg build_message(char *argv[]);
/* 4) */
int get_interval(string str);


int main(int argc, char *argv[])
{
	if(!validate_arguments(argc, argv)) return 0;
	Config config;
	config.initialize_config();
	/* config.destroy_queue(); */
	schedule_process(config, argv);
	return 0;
}

/**
 * 1) verifies if the arguments are valid. If not, it terminates the execution
 */
int validate_arguments(int argc, char *argv[]){
	if(argc != 4){
		cout << "Numero incorreto de argumentos\n";
		return 0;
	}
	vector<string> time_args;
	const char *delimiter = ":";
	split(string(argv[1]), delimiter, time_args);
	if(time_args.size() != 2   ||
			!is_number(time_args[0]) ||
			!is_number(time_args[1]) ||
			!is_number(string(argv[2]))
	  ){
		cout << RED << "Chamada com argumentos invalidos, favor utilizar o seguinte formato: \n\t$ solicita_execucao 00:00 0 nome_programa" << RESET << endl;
		return 0;
	}
	return 1;
}


/**
 * 2) insere a nova solicitacao de proecesso na pilha de mensagens
 * */
void schedule_process(Config &config, char *argv[]){
	Msg msg = build_message(argv);
	display_msg(msg);
	config.push_msg(msg);
}

/**
 * 3) gera a mensagem formatada corretamente para a fila de mensagens
 * com o instante de inicio de execucao, o numero de copias e o arquivo executavel
 * */
Msg build_message(char *argv[]){
	Msg msg;
	msg.copies = stoi(string(argv[2]));
	msg.interval = get_interval(string(argv[1]));
	strcpy(msg.content, argv[3]);
	return msg;
}

/**
 * 4) calcula o intervalo em segundos de uma entrada em horas e minutos
 * */
int get_interval(string str){
	vector<string> time_args;
	const char *delimiter = ":";
	split(str, delimiter, time_args);
	int horas = stoi(time_args[0]);
	int minutos = stoi(time_args[1]);
	int tempo_total = (minutos * 60) + (horas * 3600);
	return tempo_total;
}
