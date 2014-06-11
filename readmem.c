/* Read memory
   Copyright (c) 2014 Wouter Cloetens <wouter@e2big.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#define _GNU_SOURCE_
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

static void hexdump(const unsigned char *buffer, off_t offset, size_t len);

int main(int argc, char *argv[])
{
	void *p = NULL;
	char *endp = NULL;
	unsigned long offset, mapoffset;
	size_t len, pagesize;
	size_t maplen;
	int success = 0;

	pagesize = sysconf(_SC_PAGESIZE);

	if (argc >= 2)
	{
		offset = strtoul(argv[1], &endp, 0);	
		if (endp != argv[1])
		{
			len = strtoul(argv[2], &endp, 0);	
			if (endp != argv[2])
				success = 1;
		}
	}
	if (!success)
	{
		fprintf(stderr, "Usage: %s <physical address> <length>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	printf("offset: 0x%lx len: 0x%x\n", offset, len);

        int fd = open("/dev/mem", O_RDONLY|O_SYNC);
        if (fd < 0)
        {
                fprintf(stderr, "Can't open /dev/mem: %m\n");
		exit(EXIT_FAILURE);
        }
	mapoffset = (offset / pagesize) * pagesize;
	maplen = ((offset - mapoffset + len + pagesize - 1) / pagesize) * pagesize;
        p = mmap(NULL, maplen, PROT_READ, MAP_SHARED, fd, mapoffset);
        if (p == NULL)
        {
                fprintf(stderr, "Can't mmap /dev/mem, offset 0x%lx, size 0x%x: %m\n", mapoffset, maplen);
		exit(EXIT_FAILURE);
        }
        if (((unsigned long)p / pagesize) * pagesize != (unsigned long)p)
        {
                fprintf(stderr, "mmap /dev/mem, offset 0x%lx, size 0x%x resulted in non-aligned address 0x%lx\n",
			mapoffset, maplen, (unsigned long)p);
		munmap(p, maplen);
		close(fd);
		exit(EXIT_FAILURE);
        }
        printf("mmap /dev/mem, offset 0x%lx, size 0x%x at address 0x%lx\n", mapoffset, maplen, (unsigned long)p);
	fflush(stdout);

/*
 	printf("hexdump(0x%lx + 0x%lx - 0x%lx = 0x%lx, 0x%lx, 0x%lx\n", (unsigned long)p, offset, mapoffset, (unsigned long)(p + (offset - mapoffset)), offset, (unsigned long)len);
	fflush(stdout);
*/
	hexdump(p + (offset - mapoffset), offset, len);

	munmap(p, maplen);
	close(fd);
        return EXIT_SUCCESS;
}

static void hexdump(const unsigned char *buffer, off_t offset, size_t len)
{
    unsigned int i;
    int j, k;

    printf("data length: %d = 0x%x\n", (int)len, (int)len);

    for (i = 0, j = 0; i < len; i++, j++)
    {
        if (i % 16 == 0)
            printf("%08lx  ", offset + i);
        printf("%02x ", ((int)buffer[i] & 0xff));
        if (j >= 15)
        {
            for ( ; j >= 0; j--)
                printf("%c",
                       ((buffer[i - j] >= ' ') && (buffer[i - j] <= '~')) ?
                       buffer[i - j] : '.');
            printf("\n");
        }
    }
    if (i % 16 != 0)
    {
        for (k = j; k <= 15; k++)
            printf("   ");
        for (k = j ; k > 0; k--)
            printf("%c",
                   ((buffer[i - k] >= ' ') && (buffer[i - k] <= '~')) ?
                   buffer[i - k] : '.');
        for (k = j ; k <= 15; k++)
            printf(" ");
    }
    printf("\n");
    fflush(stdout);
}
