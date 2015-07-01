/*
 * This is a file that tests whether or not
 * we can call functions within functions
 * after we have modified the canary values
 * (according to the modified scheme). This
 * should be run through fix_canaries for
 * testing purposes. Otherwise, it will fail
 * miserably.
 */
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <alloca.h>

#define FORCE_STACK_PROTECTOR char buffer[400];

void walk_canaries() {
	void *rsp = NULL;
	uint64_t offset = 0;
	uint64_t canary = 0;
	int counter = 0;
	/*
	 * Without the nop here, fix_canaries
	 * believes that this is a push canary operation.
	 */
	asm volatile ("mov %%fs:0x28, %0\n"
	              "nop\n"
	     : "+r" (offset)
			 :
			 :);
	//rsp = (void*)(((uint64_t)&rsp) & 0xabcdef);
	rsp = &rsp;

	printf("rsp: %p\n", rsp);
	rsp = (void*)((uint64_t)rsp >> 16);
	rsp = (void*)((uint64_t)rsp << 16);
	printf("rsp: %p\n", rsp);

	printf("fs:0x28: %lx\n", offset);
	offset &= 0xFFFF;
	printf("fs:0x28: %lx\n", offset);

	rsp += offset;
	while (((uint64_t)rsp & 0xFFFF) != 0xFFFF) {
		printf(" rsp: %p\n", rsp);
		printf("*rsp: %lx\n", *((uint64_t*)(rsp)));
		rsp += *(uint64_t*)rsp & 0xFFFF;
		if (counter > 15) break;
		counter++;
	}
	printf(" rsp: %p (final)\n", rsp);
}

void set_canary64(uint64_t new_canary) {
	uint64_t old_canary;
	printf("Setting new canary as %lx \n", new_canary);
	asm volatile ("mov %%fs:0x28, %0\n"
	    "mov %1, %%fs:0x28\n"
	    : "+r" (old_canary)
	    : "r" (new_canary)
	    :);
}

uint64_t read_random64(void) {
	uint64_t random_value = -1;
	int fd = -1;
	fd = open("/dev/urandom", O_RDONLY);
	if (fd != -1) {
		read(fd, (void*)&random_value, sizeof(uint64_t));
		close(fd);
	}
	return random_value;
}

void set_canary32(uint32_t new_canary) {
	uint64_t old_canary;
	asm volatile ("mov %%fs:0x28, %0\n"
	    "mov %1, %%fs:0x28\n"
	    : "+r" (old_canary)
	    : "r" (new_canary)
	    :);
}

uint32_t read_random32(void) {
	uint32_t random_value = -1;
	int fd = -1;
	fd = open("/dev/urandom", O_RDONLY);
	if (fd != -1) {
		read(fd, (void*)&random_value, sizeof(uint32_t));
		close(fd);
	}
	return random_value;
}

int loop(int v) {
	FORCE_STACK_PROTECTOR;
	void *sp = NULL;

	printf("entering v: %d\n", v);

	sp = alloca(v*sizeof(char));

	if (v == 1) {
		/*
		 * Because this rewrites the entire 64 bit
		 * canary value, it will cause previously
		 * pushed canaries to fail to decode.
		 */
		//set_canary64(read_random64());
		walk_canaries();
	}
	else {
		//set_canary32(read_random32());
	}
	if (v>0)
		loop(--v);
	printf("exiting  v: %d\n", v+1);
	return 0;
}

int main() {
	set_canary64(0xFFFFFFFFFFFFFFFF);
	loop(10);
	return 0;
}
