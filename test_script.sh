echo "Test Case 1: Simple Send and receive "
./test 8 1 &
./test 8 2 

read x
echo "Test Case 2: QFULL Sender Blocked"

read x
echo "TEST Case 3: RECIEVER BLOCK "
./test 5 1 5 &
./test 5 2 4 
./test 5 2 1

read x
echo "Test Case 4: Single sender and multiple receivers "
./test 5 1 &
./test 5 1 &
./test 5 2 

read x
echo "TEST Case 5: Multiple receiver multiple messages"
./test 5 1 4 1 &
./test 5 1 4 1 &
./test 5 1 4 1 &
./test 5 2 4 

read x
echo "Test Case 6: Deferred Registration"
./test 5 1 3 1 &
./test 5 2 1 2 
./test 5 1 &
./test 5 2

read x
echo "Test Case 7: MULTIPLE WRITERS TO QUEUE"
./test 5 1 6 1 &
./test 5 2 3  
./test 5 2 1
./test 5 2 2

read x
echo "Test Case 8: MANY WRITE, MANY READ SAME Q"
./test 5 1 6 1 &
./test 5 1 6 1 &
./test 5 1 6 1 &
./test 5 2 2
./test 5 2 1
./test 5 2 3

read x
echo "Test Case 9: MANY WRITE, MANY READ DIFFERENT Q"
./test 6 1 4 1 &
./test 6 1 4 1 &
./test 7 1 4 1 &
./test 7 1 4 1 &
./test 6 2 2 1 
./test 7 2 2 1
./test 6 2 2 1
./test 7 2 2 1

read x
echo "Test Case 10-17: Are negative test cases which cannot be scripted. These can be run manually to verify.."





