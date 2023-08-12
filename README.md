# Jobin

Coroutine/Fiber based Job system. This project is an advanced job system written in C++. It can be used to execute functions in parallel with a programming model similar 
to async and await of Javascript. The system altomatically dispatch the functions to be executed in different threads in parallel using an efficient FIFO strategy. This project 
inspired by the Naighty Dog presentation in https://www.youtube.com/watch?v=HIVBhKj7gQU on how they parallelized the Naughty Dog Engine using Fibers. The project also makes heavy 
use template metaprogramming to give programmers the most easy integration process in their C++ codebases.
