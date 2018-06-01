#include "includes.cpp"
using namespace std;

/* 1) */
int validate_arguments(int argc, char *argv[], Config &config);
/* 2) */
void schedule_process(Config &config, char *argv[], int argc);
/* 3) */
Msg build_message(char *argv[], int argc);
/* 4) */
t_time get_interval(string str);


int main(int argc, char *argv[])
{
	Config config;
	if(!config.initialize_config()) return 0;
	if(!validate_arguments(argc, argv, config)) return 0;
	schedule_process(config, argv, argc);

	return 0;
}

/**
 * 1) verifica se os argumentos sao validados
 */
int validate_arguments(int argc, char *argv[], Config &config){
	/* destroy a fila de mensagens */
	if(argc == 2 && string(argv[1]) == "-d"){
		config.destroy_queue();
		return 0;
	}
	if(argc == 4){
		vector<string> time_args;
		const char *delimiter = ":";
		split(string(argv[1]), delimiter, time_args);
		if(time_args.size() != 2   ||
				!is_number(time_args[0])   ||
				!is_number(time_args[1])   ||
				!is_number(string(argv[2]))
		  ){
			cout << RED << "Chamada com argumentos invalidos, favor utilizar o seguinte formato: \n\t$ solicita_execucao 00:00 0 nome_programa" << RESET << endl;
			return 0;
		}
		return 1;
	}
	if(argc != 5){
		cout << "Numero incorreto de argumentos\n";
		return 0;
	}
	vector<string> time_args;
	const char *delimiter = ":";
	split(string(argv[1]), delimiter, time_args);
	if(time_args.size() != 2   ||
			!is_number(time_args[0])   ||
			!is_number(time_args[1])   ||
			!is_number(string(argv[2]))||
			!is_number(string(argv[3]))
	  ){
		cout << RED << "Chamada com argumentos invalidos, favor utilizar o seguinte formato: \n\t$ solicita_execucao 00:00 0 0 nome_programa" << RESET << endl;
		return 0;
	}
	return 1;
}


/**
 * 2) insere a nova solicitacao de proecesso na pilha de mensagens
 * */
void schedule_process(Config &config, char *argv[], int argc){
	Msg msg = build_message(argv, argc);
	config.push_msg(msg);
}

/**
 * 3) gera a mensagem formatada corretamente para a fila de mensagens
 * com o instante de inicio de execucao, o numero de copias e o arquivo executavel
 * */
Msg build_message(char *argv[], int argc){
	int offset = argc == 5 ? 0 : 1;
	Msg msg;
	msg.copies = stoi(string(argv[2]));
	msg.horario = get_interval(string(argv[1]));
	msg.priority = argc == 5 ? stoi(string(argv[3])) : 1;
	strcpy(msg.content, argv[4 - offset]);
	return msg;
}

/**
 * 4) calcula o intervalo em segundos de uma entrada em horas e minutos
 * */
t_time get_interval(string str){
	vector<string> time_args;
	const char *delimiter = ":";
	split(str, delimiter, time_args);
	int horas = stoi(time_args[0]);
	int minutos = stoi(time_args[1]);

	time_t now;
	t_time when;
	time(&now);
	when = *localtime(&now); // pega o horario atual

	// adiciona o delay ao horario atual
	when.tm_hour += (horas + (minutos / 60));
	when.tm_min += (minutos % 60);

	return when;
}
