This directory contains files needed for the auto-testing of the system

The data sets are created on the assumption that the DEFAULT
parameters (in config.h) are used to compile the main programs.

  1 Shamir        (3,1)
  2 Replicated    (3,1) Maurer
  3 Replicated    (3,1) Reduced
  4 Q2 Shamir     (3,1)
  5 Q2 Replicated (3,1)
  6 Shamir        (4,1)
  7 Replicated    (4,1) Maurer
  8 Replicated    (4,1) Reduced
  9 Replicated    Unqual example  Maurer
 10 Replicated    Unqual example  Reduced
 11 Replicated    Qual example    Maurer
 12 Replicated    Qual example    Reduced
 13 Q2 Shamir     (4,1)
 14 Q2 Replicated (4,1)
 15 Q2 Main Example 5
 16 Q2 Main Example 6
 17 Q2 DNF        (4,1)
 18 FHE Based 2 party 128-bit prime
 19 FHE Based 3 party 128-bit prime
 20 FHE Based 2 party 32-bit prime
 21 FHE Based 3 party 32-bit prime
 22 Shamir        (5,2)
 23 Replicated    (5,2) Maurer
 24 Replicated    (5,2) Reduced
 25 Shamir        (10,4) 
 26 Shamir        (3,1) Small 32-bit prime
 27 Replicated    (3,1) Small 32-bit prime Maurer


To execute ALL the tests type
        ./run_tests.sh
in the main directory. And be prepared to wait ages and ages


One should also test...

Full Threshold 128 bit prime : 2 and 3 players
  - To test the main usage of two players
  - Programs/IO_demo          <- Tests IO
  - Programs/mult_demo        <- Run for an hour or so to test factory production

Full Threshold 16 bit prime : 2 player only
  - To test more than one MAC
  - Programs/IO_demo          <- Tests IO
  - Programs/mult_demo        <- Run for an hour or so to test factory production

The restart programs
  - See below for instructions


The specific LSSS are...

1) The unqualified example has maximal unqualified sets
        1 0 0 0
        0 1 1 0
        0 1 0 1
        0 0 1 1

2) The qualified example has minimal qualified sets
        1 1 0 0
        1 0 1 0
        1 0 0 1
        0 1 1 1

3) The Q2 examples are...

  - Q2 MSPs
     - Case 1: n=3  (Shamir by another name)
         1 1 1
         2
         
         1 2
         1 3
         1 4
     - Case 2: n=3  (Replicated threshold by another name)
         2 2 2
         3
         
         0 1 0
         0 0 1
         1 0 0
         0 0 1
         1 0 0
         0 1 0
    - Case 3: n=4  (Shamir by another name)
         1 1 1 1
         2

         1 2
         1 3
         1 4
         1 5
    - Case 4: n=4  (Replicated threshold by another name)
         3 3 3 3 
         4

         0 1 0 0
         0 0 1 0
         0 0 0 1
         1 0 0 0
         0 0 1 0
         0 0 0 1
         1 0 0 0
         0 1 0 0
         0 0 0 1
         1 0 0 0
         0 1 0 0
         0 0 1 0
     - Case 5: n=4  Delta^+ = 0,12,23; Gamma^- = 01,02,03,13
		   This test the assignment of channels in the online
                   phase as it is non-trivial in this case
         2 1 1 1
         3

         1  0  0
         0  1  0
         1  0  1
         0  1  1
         1 -1  1
     - Case 6: n=4  Delta^+ = 0,12,13,23; Gamma^- = 01,02,03,123   Not Mult
                   This tests the extension to multiplicative MSP
                   is working
         3 2 2 2
         6
         
         1 0 1 1 1 1
         1 1 0 1 1 1
         1 1 1 0 1 1
         1 1 1 1 0 0
         0 1 0 0 0 0
         0 0 1 0 0 0
         0 0 0 0 1 0
         0 0 0 1 0 0
         0 0 0 0 0 1
     - Case 7 : n=4 This is the DNF sharing for threshold (4,1)
         3 3 3 3 
         7 

         1 0 1 1 1 1 1
         1 1 0 1 1 1 1
         1 1 1 0 1 1 1
         0 1 0 0 0 0 0
         1 1 1 1 0 1 1
         1 1 1 1 1 0 1
         0 0 1 0 0 0 0
         0 0 0 0 1 0 0
         1 1 1 1 1 1 0
         0 0 0 1 0 0 0
         0 0 0 0 0 1 0
         0 0 0 0 0 0 1 


  

Restart Testing
---------------
Go to the main directory

   \cp Programs/restart_1/restart.mpc Programs/restart/
   ./compile.py Programs/restart

Run program as normal, using Programs/restart 

When finished all players should enter a number
  - Do not do this yet!

First get the next program ready to run

   \cp Programs/restart_2/restart.mpc Programs/restart/
   ./compile.py Programs/restart

Now enter a number on each players console

The second program should now run

