#include <bits/stdc++.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <unistd.h>

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
#define MSG_SIZE 100

typedef struct t_mensagem {
	long nothing;
	int copies;
	long interval;
	char content[MSG_SIZE];
} Msg;

class Config {
	public:
		int id_fila = -1;
		Config(){};
		void initialize_config(){
			if(id_fila < 0) start_queue();
		}

		void start_queue(){
			this->id_fila = msgget(MESG_QUEUE_NAME, IPC_CREAT|0666);
		}

		void destroy_queue(){
			msgctl(id_fila, IPC_RMID, NULL);
		}

		int get_queue(){
			return this->id_fila;
		}

		void push_msg(Msg msg){
			msgsnd(id_fila, &msg, sizeof(Msg)-sizeof(long), 0);
		}
		int pop_msg(Msg *msg){
			if(msgrcv(id_fila, msg, sizeof(Msg)-sizeof(long), 0, 0) != -1) return 1;
			else return 0;
		}
};

void display_msg(Msg *msg){
	cout << "intervalo: " << msg->interval << ", copias: " << msg->copies << ", mensagem: " << string(msg->content) << endl;
}

bool in_array(const std::string &value, const std::vector<string> &array)
{
	return std::find(array.begin(), array.end(), value) != array.end();
}

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
 * verifica se string eh um numero exadecimal
 * *****************************************************************************/
bool is_hex_string(std::string& s) {
	string str = s;
	to_uppercase(str);
	if(s[0] == '-')
		str = s.substr(1,s.length());
	vector<string> v;
	const char *c = "X";
	split(str, c, v);
	if(v.size() > 2 || v[0] != "0") return 0;

	for(auto &x:v[1]) {
		if(!isxdigit(x)) return 0;
	}

	return 1;
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

