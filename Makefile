myshell: myshell.c myshell.h builtin_command.o signal_process.o
	gcc -o myshell myshell.c builtin_command.o signal_process.o

builtin_command.o: builtin_command.c builtin_command.h myshell.h
	gcc -c builtin_command.c

signal_process.o: signal_process.c signal_process.h myshell.h
	gcc -c signal_process.c

clean:
	rm-f *.o
