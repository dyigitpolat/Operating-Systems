TIMEFORMAT=%R
echo "test in progress.."
N=10+2
R=10

elapsed=0
for i in $(seq 1 10); 
do
	mycom=$( { time ./a.out 10 $R in1 in2 in3 in4 in5 in6 in7 in8 in9 in10 out 1920 1990 12000000 55000000 > log; } 2>&1)
	elapsed=$( { bc <<< $mycom+$elapsed; } 2>&1 )
done


echo $( { bc <<< $elapsed/10.0; } 2>&1 )


