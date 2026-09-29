# Code Conventions

- Use `nullptr` and `override` (we have defines for these)
- Use C++ casts (`static_cast< T >(expr)`) instead of C-style casts
- Use `default` when possible
- Use `auto` when it makes sense
- Prefix pointers with `p` and references with `r`. 
- Use {} all the time, even with if / else blocks with a single line
- Write Doxygen comments on every matching function: `@brief`, one `@param` per parameter describing its purpose and any known constraints or unused status, and `@return` when the function returns a value.
- Try to avoid comments unless it is not obvious at all what code is doing
