# Threads_Atermis
A Threads system using C++

20 threads são criadas no main(), cada uma executa a função registrarLogs() e gera 10 entradas no arquivo — totalizando 200 linhas de log.

Para compilar:

g++ -std=c++17 -pthread -o artemis artemis_log.cpp
./artemis
