all: build exec

build: solicita_execucao escalonador

solicita_execucao: sol_exec.cpp includes.cpp
	g++ -std=c++11 sol_exec.cpp -o solicita_execucao

escalonador: escalonador.cpp includes.cpp
	g++ -std=c++11 escalonador.cpp -o escalonador

exec:
	./solicita_execucao 00:10 3 1 hello_world

remove:
	rm solicita_execucao escalonador
