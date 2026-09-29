# Functions
3DW + BF was compiled with a flag that left unused functions in. However, they are not quite "unused". They are simply inlined in the file, but the compiler kept its function in the file. So if there is a function that is not used anywhere in the file, there is a huge chance that it's inlined somewhere in the file. Look for the pattern and apply it.

# Weak Functions
There are a lot of weak functions in the game. The linker places them there because its the first instance of the game using it. Please keep them in their own file, we do not want to define weak function bodies in C++ files, only their source headers.