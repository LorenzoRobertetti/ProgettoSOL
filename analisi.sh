#/bin/bash

path=log.txt

clienticasse=$(grep "clientecassa" $path)
clientipuri=$(grep "clientepuro" $path)
cassechiusure=$(grep "cassachiusura" $path)

echo "$clientipuri" | while read -r line ; do
    id=$(echo $line | awk '{print $2}')
    numprod=$(echo $line | awk '{print $(NF-2)}')
    tottime=$(echo $line | awk '{print $(NF-1)}')
    queuetime=$(echo $line | awk '{print $(NF-3)}')
    numcasse=$(echo $line | awk '{print $(NF)}')
    echo "Cliente:| id cliente $id | n.prodotti acquistati $numprod | tempo totale nel super. $tottime | tempo tot. speso in coda $queuetime | n. di code visitate $numcasse |"
done

grep "cassafinale" $path | while read -r line ; do
    id=$(echo $line | awk '{print $(NF-2)}')
    numcl=$(echo $line | awk '{print $(NF)}')
    numchiusure=$(echo $line | awk '{print $(NF-1)}')

    # avg tempo servizio
    total=0
    times=$(echo "$clienticasse" | awk -v id="$id" '$3 == id' | awk '{print $(NF)}')
    for time in $times; do
        total=$(echo $total+$time | bc )
    done

    if [ $numcl -eq 0 ]
    then
        avgservtime=0
    else
        avgservtime=$(echo "scale=3; $total / $numcl" | bc)
    fi

    # num tot prodotti
    totalprod=0
    numprods=$(echo "$clientipuri" | awk -v id="$id" '$3 == id' | awk '{print $(NF-2)}')
    for numprod in $numprods; do
        totalprod=$(echo $totalprod+$numprod | bc )
    done

    # tot tempo apertura
    totaltime=0
    times=$(echo "$cassechiusure" | awk -v id="$id" '$2 == id' | awk '{print $(NF)}')
    for time in $times; do
        totaltime=$(echo $totaltime+$time | bc )
    done

    echo "Cassa:| id cassa $id | n. prodotti elaborati $totalprod | n. di clienti $numcl | tempo tot. di apertura $totaltime | tempo medio di servizio $avgservtime | n. di chiusure $numchiusure |"
done
