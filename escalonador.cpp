#include "includes.cpp"
using namespace std;

int main(int argc, char *argv[])
{
	Config config;
	Msg msg;
	config.initialize_config();
	if(config.pop_msg(&msg)) display_msg(&msg);
	return 0;
}
