#include "includes.cpp"
using namespace std;

int main(int argc, char *argv[])
{
	if(argc != 2 || !is_number(string(argv[1]))){
		cout << "Argumentos invalidos, tente novamente." << endl;
		return 0;
	}

	Config config;
	if(!config.initialize_config()) return 0;
	Msg msg;
	msg.nothing = REMOVE_PROC;
	msg.priority = stoi(string(argv[1]));
	config.push_msg(msg);
	return 0;
}
