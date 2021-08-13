# rbc-plus-plus
 
A C++ bot for the JHUAPL reconaissance blind chess competition

## Installation
You need to install OpenSSL first before compiling. Right now the makefile is only implemented for Windows.

## Engines
**UniformExpectimax** chooses moves and senses via the Expectimax algorithm with a uniform distribution over all potential boards. Board evaluation is done with a pure monte carlo method.  
**Expectimax** chooses moves and senses via the Expectimax algorithm over the given probability distribution. Board evaluation is done with a pure monte carlo method.  
**ShannonExpectimax** chooses moves and senses via the Expectimax algorithm over the given probability distribution. Board evaluation is done using Shannon's evaluation function.  
**MonteShannonExpectimax** chooses moves and senses via the Expectimax algorithm over the given probability distribution. Board evaluation is done with a monte carlo method using Shannon's evaluation function at the max depth to give a score.  
**RandomEngine** Chooses moves and senses randomly  
