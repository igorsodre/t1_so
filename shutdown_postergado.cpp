#include "includes.cpp"
using namespace std;

int main()
{
	Config config;
	if(!config.initialize_config()) return 0;
	Msg msg;
	msg.nothing = SHUTDOWN;
	config.push_msg(msg);
	return 0;
}
