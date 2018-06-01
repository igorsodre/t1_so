#include <bits/stdc++.h>
#include <unistd.h>
#include <time.h>
#define endl std::endl

void dummy(int i){
	i++;
	std::cout << endl;
	std::cout << "\n\n\n===================== Terminei de executar pid: " << getpid() <<" ============================\n\n\n";
	exit(0);
}

int main()
{
	srand(time(NULL));
	int times = (rand() % 10) + 5;
	std::cout << "executarei prog1 por: " << times << " sobre 2 segundos \n";
	signal(SIGALRM, dummy);
	alarm(times);
	while(1)
	{
	}
	return 0;
}

