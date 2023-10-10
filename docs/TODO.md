1.) Currently when create_atom is first called 1 row of dummy atoms are created and it gets extended.
    This could be improved to create the dummy rows after the first row of true atoms are created and it
    will speed it up proportionally to the size of the first row.

2.) dummy rows could be extended by more than 1 to create anticaptory rows which could offer some speedup.

3.) It would be nice to have a way to alter previous cprintf() specifiers or update them.

4.) Bunch of loops can be changed to do while.
