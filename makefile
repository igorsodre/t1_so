all: build exec

build: solicita_execucao escalonador lista_postergados remove_postergado shutdown_postergado compila_testes

solicita_execucao: sol_exec.cpp includes.cpp
	g++ -std=c++11 sol_exec.cpp -o solicita_execucao

escalonador: escalonador.cpp includes.cpp
	g++ -std=c++11 escalonador.cpp -o escalonador

lista_postergados: includes.cpp lista_postergados.cpp
	g++ -std=c++11 lista_postergados.cpp -o lista_postergados

remove_postergado: remove_postergado.cpp includes.cpp
	g++ -std=c++11 remove_postergado.cpp -o remove_postergado

shutdown_postergado: shutdown_postergado.cpp includes.cpp
	g++ -std=c++11 shutdown_postergado.cpp -o shutdown_postergado

compila_testes: prog1.c prog2.c
	g++ prog1.c -o prog1 && g++ prog2.c -o prog2

exec:
	./solicita_execucao 00:00 3 1 prog1 && ./solicita_execucao 00:00 1 prog2 && ./solicita_execucao 00:00 2 2 prog2
	# ./solicita_execucao 00:00 1 1 prog1
lista:
	./lista_postergados

desliga:
	./shutdown_postergado

remove:
	rm solicita_execucao escalonador
