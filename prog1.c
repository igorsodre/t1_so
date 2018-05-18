#include <bits/stdc++.h>
#include <unistd.h>
#include <time.h>
#define endl std::endl
int main()
{
	srand(time(NULL));
	int times = (rand() % 36) + 40;
	std::cout << "executarei prog1 por: " << times << " sobre 2 segundos \n";
	for (int i = 0; i < times; ++i)
	{
			std::cout << "carregando em segundos " << i << "\r" << std::flush;
			int milisec = 500; // length of time to sleep, in miliseconds
			struct timespec req = {0};
			req.tv_sec = 0;
			req.tv_nsec = milisec * 1000000L;
			nanosleep(&req, (struct timespec *)NULL);
	}
	std::cout << endl;
	std::cout << "\n\n\n===================== Terminei de executar ============================\n\n\n";
	return 0;
}

