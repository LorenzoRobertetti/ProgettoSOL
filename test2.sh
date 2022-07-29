#/bin/bash

echo "-k 2 -c 20 -e 5 -t 500 -p 80 -s 30 -f ./log.txt -x 2 -y 10 -m 2 -l 5 -z 50" > config.txt
valgrind -s --leak-check=full ./exesm.out ./config.txt &
pid=$!
sleep 15
kill -SIGQUIT $pid
wait $pid
bash analisi.sh
