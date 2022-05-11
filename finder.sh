for var in "$@"
do
  test=$(grep -o "$var" ./out_files/*out | wc -l)
  echo "$var" "$test"
done
