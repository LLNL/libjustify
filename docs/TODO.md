1.) Currently when create_atom is first called 1 row of dummy atoms are created and it gets extended.
    This could be improved to create the dummy rows after the first row of true atoms are created and it
    will speed it up proportionally to the size of the first row.

2. ) dummy_row_ds->top_row is in unnecessary and free_graph can be modified to remove it.

3. ) origin is used often as origin->up because we start at the dummy rows. This is pretty confusing

4. ) dummy rows could be extended by more than 1 to create anticaptory rows which could offer some speedup.

5. ) It would be nice to have a way to automatically insert | or some other delimiter to make tables.

6. ) It would be nice to have a way to alter previous cprintf() specifiers or update them.

7. ) Bunch of loops can be changed to do while.

8. ) Evaluate the %n callback. It's unsafe enough as is and I don't want to introduce any additional issues.