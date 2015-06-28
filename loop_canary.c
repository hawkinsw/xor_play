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

#define FORCE_STACK_PROTECTOR char buffer[400];

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
	printf("entering v: %d\n", v);
	if (v == 1) {
		/*
		 * Because this rewrites the entire 64 bit
		 * canary value, it will cause previously
		 * pushed canaries to fail to decode. 
		 */
		set_canary64(read_random64());
	}
	else {
		set_canary32(read_random32());
	}		
	if (v>0)
		loop(--v);
	printf("exiting  v: %d\n", v+1);
	return 0;
}

int main() {
	//set_canary64(read_random64());
	loop(10);
	return 0;
}
