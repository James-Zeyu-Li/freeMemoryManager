# freeMemoryManager
This is a simulator of a free space manager which simulate malloc memory and the free heap memory situation with different memory allocation policy and free space sorting policy.

`g++ -std=c++20 freeSpace.cpp -o free`
#### Policy
```
What’s the policy
1: BEST
2: WORST
3: FIRST
4: NEXTFIT
```
#### Flags
```
<< "-S,         size of the heap\n"
<< "-B,         base address of the heap\n"
<< "-H,         size of the header\n"
<< "-P,         list search policy (BEST, WORST, FIRST)\n"
<< "-l,         list order (ADDRSORT, SIZESORT+, SIZESORT-)\n"
<< "-C,         coalesce the free list?\n"
<< "-A,         list of operations (+10,-0,etc)\n"
<< "-h,         show this help message and exit\n";

```




#### Test cases
1. `./free -p BEST -l ADDRSORT -C -A +40,+30,+20,-1,+50`
2. `./free -p NEXTFIT -A +40,-0,+40,-1,+10,-2,+5,+10,+20`
3. `./free -p NEXT -l ADDRSORT -C -A +10,+30,+40,+50,-1,+15,+35` 
    - should fail