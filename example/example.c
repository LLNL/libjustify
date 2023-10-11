// Copyright 2023 Lawrence Livermore National Security, LLC and other
// libjustify Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include "cprintf.h"

void test_cprintf(void)
{
    /*printf("fprintf():\n");
    fprintf( stderr, "%d %d %d\n", 1, 2, 3);
    fprintf( stderr, "%d %d %d\n", 10, 20, 30);
    fprintf( stderr, "%d %d %d\n", 100, 200, 300);
    printf("\n");

    printf("cfprintf():\n");
    cfprintf( stderr, "%d %d %d\n", 1, 2, 3);
    cfprintf( stderr, "%d %d %d\n", 10, 20, 30);
    cfprintf( stderr, "%d %d %d\n", 100, 200, 300);
    cflush();

    printf("\n\n============\n\n");

    printf("printf():\n");
    printf("a=%07.4f b= %07.5Lf\n", 1.2, 1.2L );
    printf("a=%07.4lf b= %07.5Lf\n", 10.22, 10.22L );
    printf("a=%07.4lf b= %07.5Lf\n", 100.222, 100.222L );
    printf("a=%07.4lf b= %07.5Lf\n", 1000.2222, 1000.2222L );
    printf("\n");

    printf("cprintf():\n");
    cprintf("a=%07.4f b= %07.5Lf\n", 1.2, 1.2L );
    cprintf("a=%07.4lf b= %07.5Lf\n", 10.22, 10.22L );
    cprintf("a=%07.4lf b= %07.5Lf\n", 100.222, 100.222L );
    cprintf("a=%07.4lf b= %07.5Lf\n", 1000.2222, 1000.2222L );
    cflush();

    printf("\n\n============\n\n");

    printf("printf():\n");
    printf("%s | %s | %s | %-s \n", "tiny", "longer", "pretty long", "downright wordy");
    printf("%0d | %.2f | %p | %c \n", 0, 3.14, main, 'w');
    printf("%0d | %.2f | %p | %-c \n", 0, 3.14, main, 'x');
    printf("%0d | %.2f | %p | %c \n", 20000, 300000.14, cprintf, 'y');
    printf("%0d | %.2f | %p | %-c \n", 20, 30.14, printf, 'z');
    printf("\n");

    printf("cprintf():\n");
    cprintf("%s | %s | %s | %-s \n", "tiny", "longer", "pretty long", "downright wordy");
    cprintf("%0d | %.2f | %p | %c \n", 0, 3.14, main, 'x');
    cprintf("%0d | %.2f | %p | %-c \n", 0, 3.14, main, 'x');
    cprintf("%0d | %.2f | %p | %c \n", 20000, 300000.14, cprintf, 'y');
    cprintf("%0d | %.2f | %p | %-c \n", 20, 30.14, printf, 'z');
    cflush();*/

    //cprintf("%0d | %d | %p |\n", 0, 1024, main);
    //cprintf("%0d | %.4f | %hx | %c \n", 0, 3.1415, 256, 'x');
    //cflush();

    /*printf("\ncprintf()\n");
    int n = 0;
    int s = 0;

    cprintf("")
    cprintf("%0d | %d | %n \n", 128, 2, &n);
    cprintf("%x | %.2f | %hx | %c | %n\n", 0xC0FFEE, 3.14, 256, 'x', &s);
    cflush();
    printf("n = %d\n", n);
    printf("s = %d\n", s);
    */
}

void print_children(unsigned int i)
{
    unsigned ARR_SIZE = 32;

    if (i == 0)
    {
        cprintf("%s %s %s %s\n", "Thread", "HWThread", "Core", "Socket");
    }

    int socket = (i > ARR_SIZE / 2) % 2;
    cfprintf(stdout, "%d %d %d %d\n", i, i, i, socket);

    i = i + 1;
    if (i < ARR_SIZE)
    {
        print_children(i);
    }
    else
    {
        cflush();
    }

}

void print_power(int SIZE)
{
    //cfprintf(stdout, "%-d | %f |\n", i, 3.14f);
    //cfprintf(stdout, "_AMD_GPU_CLOCKS Host: %s, Socket: %d,")

    for (int i = 0; i < SIZE; i++)
    {
        if (i == 0)
        {
            cfprintf(stdout, "%s | %s | \n", "Core", "Energy (J)");
        }
        else
        {
            cfprintf(stdout, "Core: %i | %-s %f |\n", i * 100, "Socket:", 3.14f);
        }
    }
    dump_graph();
    cflush();
}

void variorum_print_topology(void)
{
    char *hostname = "quartz1234";
    int num_sockets = 2;
    int num_cores_per_socket = 18;
    int total_cores = num_sockets * num_cores_per_socket;
    int total_Threads = 36;
    int threads_per_core = 2;

    cfprintf(stdout, "=================\n");
    cfprintf(stdout, "Platform Topology\n");
    cfprintf(stdout, "=================\n");
    cfprintf(stdout, "  %-s: %-s\n", "Hostname", hostname);
    cfprintf(stdout, "  %-s: %-d\n", "Num Sockets", num_sockets);
    cfprintf(stdout, "  %-s: %-d\n", "Num Cores per Socket", num_cores_per_socket);

    if (threads_per_core == 1)
    {
        cfprintf(stdout, "  %-s: %-s\n", "  Hyperthreading", "No");
    }
    else
    {
        cfprintf(stdout, "  %-s: %-s\n", "  Hyperthreading", "Yes");
    }

    cfprintf(stdout, "\n");
    cfprintf(stdout, "  %-s: %-d\n", "Total Num of Cores", total_cores);
    cfprintf(stdout, "  %-s: %-d\n", "Total Num of Threads", total_Threads);
    cfprintf(stdout, "\n");
    cfprintf(stdout, "Layout:\n");
    cfprintf(stdout, "-------\n");
    cflush();
    print_children(0);
    //fprintf(stdout, "Thread HWThread Core Socket\n");
}

void test_space_before_format_string(void)
{
    cprintf("  %d %d %d\n", 100, 200, 300);
    cprintf("  %d %d %d\n", 100, 200, 300);
    cprintf("  %d %d %d\n", 100, 200, 300);
    cprintf("  %d %d %d\n", 100, 200, 300);
    cprintf("  %d %d %d\n", 100, 200, 300);
    cprintf("  %d %d %d\n", 100, 200, 300);
    cprintf("  %d %d %d\n", 100, 200, 300);
    cprintf("  %d %d %d\n", 100, 200, 300);
    cprintf("  %d %d %d\n", 100, 200, 300);
    cprintf("  %d %d %d\n", 100, 200, 300);

    cflush();
}

void test_hyperthreading(void)
{
    cfprintf(stdout, "  %-s %s\n", "Hyperthreading:", "Enabled");
    cfprintf(stdout, "  %-s %-d\n", "Num Thread Per Core: ", 2);
    cflush();
}

int main()
{
    //variorum_print_topology();
    //test_space_before_format_string();
    //test_hyperthreading();
    //fprintf(stdout, "  Hyperthreading:       Enabled\n");
    //fprintf(stdout, "  Num Thread Per Core:  %d\n", 1);
    //cflush();
    //cfprintf(stdout, "Core: %-d | %-s %f |\n", 22, "Socket:", 3.14f);
    //cflush();
    print_power(2);
    return 0;
}
