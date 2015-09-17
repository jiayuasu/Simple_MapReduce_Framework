# Simple_MapReduce_Framework
This is a very simple thread based MapReduce framework implementation which counts the letters in one document.

## Main idea
This implementation follows the basic idea of MapReduce. It makes use of multi-thread to count the letters in one document.
### Input document
One input document is loaded into a mmap which is shared between different threads in Read-Only mode.
###Mapper
Each mapper takes a part of the mmap as the input. Each mapper is assigned to one reducer using "mod" operation.
###Reducer
Each reducer aggregates the outputs from many mappers. Once all the mappers assigned to one reducer finish their own computation, this reducer will start its computation right away without waiting for unrelated mappers.
###Result
The main process has to wait the completion of all the reducers and aggregates the outputs.

## How to test
1. Put the source code under a folder in Linux.
2. Change the FILEPATH with your target document path.
3. Change M and R to your target numbers of Mappers and Reducers. Note that: M and R which are larger than one only work on multi-core processors.
```
gcc -pthread Count_The_Letters.c -o ExecuteMe
```
