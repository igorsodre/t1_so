all: build exec

build: solicita_execucao escalonador compila_testes

solicita_execucao: sol_exec.cpp includes.cpp
	g++ -std=c++11 sol_exec.cpp -o solicita_execucao

escalonador: escalonador.cpp includes.cpp
	g++ -std=c++11 escalonador.cpp -o escalonador

exec:
	./solicita_execucao 00:00 2 1 prog1 && ./solicita_execucao 00:01 1 prog2 && ./solicita_execucao 00:00 2 2 prog2
	# ./solicita_execucao 00:00 1 1 prog1

compila_testes: prog1.c prog2.c
	g++ prog1.c -o prog1 && g++ prog2.c -o prog2
remove:
	rm solicita_execucao escalonador
