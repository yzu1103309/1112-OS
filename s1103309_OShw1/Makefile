CC = g++
CFLAGS = -Wall -g -O

all:
	@echo "\e[1;34m"
	@cat ./.signature.txt

	@echo "\e[1;37m[\e[1;32m info \e[1;37m] Compile progress started... \e[1;33m"
	$(CC) $(CFLAGS) ./master.cpp -o ./master.out
	$(CC) $(CFLAGS) ./mmv.cpp -o ./mmv.out
	@echo "\e[1;37m[\e[1;32m info \e[1;37m] Compile progress completed! \e[1;37m"

	@echo "\n[\e[1;32m info \e[1;37m] Listing files in current folder using 'ls'... \e[1;33m"
	@ls
	@echo "\e[1;37m"

test:
	@echo "\e[1;34m"
	@cat ./.signature.txt

	@echo "\e[1;37m[\e[1;32m info \e[1;37m] Test progress started... \e[1;33m"
	@echo "\n\e[1;37m[\e[1;32m info \e[1;37m] Show Content of 'blake.txt' using 'cat'... \e[1;33m"
	@cat ./blake.txt

	@echo "\n\e[1;37m[\e[1;32m info \e[1;37m] Listing files in current folder using 'ls'... \e[1;33m"
	@ls

	@echo "\n\n\e[1;37m[\e[1;32m info \e[1;37m] Start testing with default output file (happy.tmp) \e[1;33m"
	@echo "\e[1;37m---------------------------------------------------------------------------"
	@echo "\e[1;37m[\e[1;32m info \e[1;37m] Executing program 'master.out' \e[1;37m \n"
	./master.out
	@echo "\n\e[1;37m[\e[1;32m info \e[1;37m] Exited program 'master.out' \e[1;33m"
	@echo "\e[1;37m---------------------------------------------------------------------------"

	@echo "\n\e[1;37m[\e[1;32m info \e[1;37m] Show Content of 'happy.tmp' using 'cat'... \e[1;33m"
	@cat ./happy.tmp

	@echo "\n\e[1;37m[\e[1;32m info \e[1;37m] Listing files in current folder using 'ls'... \e[1;33m"
	@ls

	@echo "\n\e[1;37m*******************************************************************************"


	@echo "\n\e[1;37m[\e[1;32m info \e[1;37m] Start testing with user-defined output file \e[1;33m"
	@echo "\e[1;37m---------------------------------------------------------------------------"
	@echo "\e[1;37m[\e[1;32m info \e[1;37m] Executing program 'master.out' with argv \e[1;37m \n"
	./master.out ./blake.txt ./happy2.tmp
	@echo "\n\e[1;37m[\e[1;32m info \e[1;37m] Exited program 'master.out' \e[1;33m"
	@echo "\e[1;37m---------------------------------------------------------------------------"
	@echo "\n\e[1;37m[\e[1;32m info \e[1;37m] Show Content of 'happy2.tmp' using 'cat'... \e[1;33m"
	@cat ./happy2.tmp
	@echo "\n\e[1;37m[\e[1;32m info \e[1;37m] Test progress completed! \e[1;37m\n"

clean:
	@echo "\e[1;34m"
	@cat ./.signature.txt

	@echo "\e[1;37m[\e[1;32m info \e[1;37m] Clean progress started... \e[1;37m\n"
	rm -v ./*.tmp ./*.out
	@echo "\n\e[1;37m[\e[1;32m info \e[1;37m] Clean progress completed! \e[1;37m\n"








