#/bin/bash

echo "-k 6 -c 50 -e 3 -t 200 -p 100 -s 20 -f ./log.txt -x 2 -y 10 -m 2 -l 5 -z 50" > config.txt
valgrind -s --leak-check=full ./exesm.out ./config.txt &
pid=$!
sleep 25
kill -SIGHUP $pid
wait $pid
bash analisi.sh
