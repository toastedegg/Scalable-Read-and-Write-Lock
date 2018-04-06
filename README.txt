Team members:

Haozhe Jiang 68249954
Jinsheng Zhu 33882845





The outer directory contain the deliverable with the performance test case, a cds.tar.gz contains the “user_main” version of the program that accommodate the testing with cdscheck. Use makefile to compile each version of program, but the cds one need to be extract under cds_checker/benchmarks/.. 
The correctness test is inside the rwlock.cc of the “user_main” version, with store32 in writer and load32 in reader to detect potential data race. The performance test is inside the “main”version program.
Other files such as description.txt, performance_measurement.pdf and statement.txt is inside the outer directory.




performance test:

commands: 
performance measurement: $./rwlock number_of_thread  
correctness check: ./run.sh the_executable_under_rwlock_cds (this directory is inside the tarball called cds.tar.gz)


There is no input format check.

The writer and reader rato is 1:19. The writer and reader will constantly try to acquire read and write lock each iterates 1000 times. The main will print out the average time(in 10 run) used for linuxrwlock and our lock.
