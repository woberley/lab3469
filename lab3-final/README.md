ECE469
LAB03

ee469g55
AKASH LAVU (alavu)
WILLIAM OBERLEY (woberley)


Our gcc command has a flag for the type of scheduler that should be used. There are three macros that are defined: RR_SCHED, LT_SCHED_STATIC, and LT_SCHED_DYNAMIC. RR_SCHED is the round robin implementation of the scheduler whereas LT_SCHED_STATIC and LT_SCHED_DYNAMIC are the lottery ticket static and dynamic implementations. Only one type of scheduler can be selected. For example, if the dynamic lottery ticket scheduler wanted to be used, the command would be "gcc-dlx -mtraps -Wall -DLT_SCHED_DYNAMIC".