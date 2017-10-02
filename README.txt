Program PMan (Process Manager) is able to create a process (and run it in background), present a list of process, terminate/stop/start a process and show stat of a process.

How to use PMan
Step 1: compile test files that you want to use
Step 2: execute "make" in terminal to compile PMan
Step 3: execute "./PMan" in terminal to run PMan
Step 4: input supported commands in PMan

Supported commands of PMan:
1. bg <cmd>: start program <cmd> in the background
2. bglist: display a list of all the programs currently executing in the background
3. bgkill <pid>: terminate process <pid>
4. bgstop <pid>: temporarily stop process <pid>
5. bgstart <pid>: restart process <pid> which has been previously stopped
6. pstat <pid>: list comm, state, utime, stime, rss, voluntary_ctxt_switches and nonvoluntary_ctxt_switches of process <pid>

Important notes for bg:
If you want to run inf.c using bg, compile inf.c using "gcc inf.c -o inf" before you execute "./PMan".
Compile and run PMan and input "bg ./inf a 1" where "a" and 1 are the parameters required by inf.c. Note that you should use ./inf if inf is not in PATH.
