#Generate a random number of $1 digits
Rand () 
{ 
    TMP=`mktemp --tmpdir=/dev/shm`;
    od -An < /dev/urandom | tr -d ' \n' | head -c $1 > $TMP;
    ARG=$1;
    A=`shuf -i 1-$((ARG-1)) -n1`;
    B=$((ARG-A));
    head -c $A $TMP;
    echo -n '.';
    tail -c $B < $TMP;
    rm $TMP
}

RandInt () 
{ 
    od -An < /dev/urandom | tr -d ' \n' | head -c $1 ;
    
}

OPS="+-*"

#Return a random symbol from OPS
Rop()
{ echo "${OPS:$((RANDOM%3)):1}"; }

#Generate 10 samples, 1–10 operations each (random), where numbers have 2–(${1}+1) digits (random) 
Rand10()
{
	A1=$1;
	for i in `seq 1 10`; do
		for j in `seq 1 $((1+RANDOM%10))`; do echo -n "`Rand $((2+RANDOM%A1))` `Rop` "; done
		echo "`Rand $((2+RANDOM%A1))`"
	done

}
#Generate 10 integer samples, 1–10 operations each (random), where numbers have 2–(${1}+1) digits (random) 
RandInt10()
{
	A1=$1;
	for i in `seq 1 10`; do
		for j in `seq 1 $((1+RANDOM%10))`; do echo -n "`RandInt $((2+RANDOM%A1))` `Rop` "; done
		echo "`RandInt $((2+RANDOM%A1))`"
	done

}
MAX_SCALE_FOR_BC=2147483647
#Filter for BC; makes sure it doesn't truncates nonintegral args too soon and that there are no continuation lines in the output
BC() {  cat <(echo "scale=$MAX_SCALE_FOR_BC;") - | bc | while read line; do sed 's/0*$//'<<<$line ; done  }

ComparisonTest10()
{
  A1=$1
  Rand10 $((A1+2)) > samples.txt
  BC < samples.txt > bc.txt
  ../skocipet < samples.txt > sp.txt
  paste -d "\n" bc.txt sp.txt > smush.txt 
  while read ONE; 
      do echo "bc=$ONE"; read TWO; echo "sp=$TWO"; 
      if [[ "$ONE" == "$TWO" ]]; then echo OK; else echo MISMATCH; fi
  done < smush.txt
  rm smush.txt
}

cat<<EOF
This is meant to be insourced, i.e.
$ . `basename $0`
This makes some simple random generators available. Check the comments in this source-file for more information.
EOF









