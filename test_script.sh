echo "Test 1"
./test 8 1 &
./test 8 2 &

read x
echo "Test 2: Cannot be scripted"

read x
echo "Test 3"
./test 5 1 5 &
./test 5 2 4 
./test 5 2 1

read x
echo "Test 4"
./test 5 1 &
./test 5 1 &
./test 5 2 &

read x
echo "Test 5"
./test 5 1 4 1 &
./test 5 1 4 1 &
./test 5 1 4 1 &
./test 5 2 4 

read x
echo "Test 6"
./test 5 1 3 1 &
./test 5 2 1 2 
./test 5 1 &
./test 5 2

read x
echo "Test 7"
./test 5 1 6 1 &
./test 5 2 3  
./test 5 2 1
./test 5 2 2

read x
echo "Test 8"
./test 5 1 6 1 &
./test 5 1 6 1 &
./test 5 1 6 1 &
./test 5 2 2
./test 5 2 1
./test 5 2 3

read x
echo "Test 9"
./test 6 1 4 1 &
./test 6 1 4 1 &
./test 7 1 4 1 &
./test 7 1 4 1 &
./test 6 2 2 1 
./test 7 2 2 1
./test 6 2 2 1
./test 7 2 2 1

read x
echo "Test 10-16: Cannot be scripted"





