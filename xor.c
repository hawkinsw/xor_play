/*
 * Very early prototype of what eventually went
 * into sxor.s and fix_canaries. Mostly useless
 * now.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

char random_data[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2F,0x6C,0x69,0x62,0x36,0x34,0x2F,0x6C,0x64,0x2D,0x6C,0x69,0x6E,0x75,0x78,0x2D,0x78,0x38,0x36,0x2D,0x36,0x34,0x2E,0x73,0x6F,0x2E,0x32,0x00,0x04,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x47,0x4E,0x55,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x04,0x00,0x00,0x00,0x14,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x47,0x4E,0x55,0x00,0xE9,0x39,};

int main() {
	int offset = 0;
	int random_data_len = sizeof(random_data);
	long int random_no = 0;
	int i = 0;
	uint64_t random_offset;
	uint64_t rega = -1;
	uint64_t regb = -1;

	random_no = 23; //random() % random_data_len;
	random_offset = &random_data[random_no];

	printf("address of random_data: 0x%016llx\n", random_data);
	printf("data at random_offset: 0x%016llx\n", *(uint64_t*)random_offset);
	rega = -1;
	regb = random_offset;

	/*
	 * rega: Contains 0xff..ff
	 * regb: Contains the random address.
	 */
	printf("Pre calculation:\n");
	printf("rega: 0x%016llx\n", rega);
	printf("regb: 0x%016llx\n", regb);

	/*
	 * Calculate into rega:
	 * Lower 32 bits are random address.
	 * Upper 32 bits are xor of upper and lower
	 *       32 bits of the 64 bits at the random address.
	 */
	asm volatile(
		"movl 0x4(%1), %k0\n"
		"xorl 0x0(%1), %k0\n"
		"shl $32, %0\n"
		"or %1, %0\n"
		: "+r" (rega)
		: "r" (regb)
		:);
	/*
	 * regb is dead.
	 */
	printf("Post calculation:\n");
	printf("rega: 0x%016llx\n", rega);
	printf("regb: 0x%016llx\n", regb);

	rega ^= 0x0;
	regb = 0ll;

	/*
	 * Check on our hurt values.
	 */
	printf("Hurt:\n");
	printf("rega: 0x%016llx\n", rega);
	printf("regb: 0x%016llx\n", regb);

	/*
	 * Check into regb
	 * Output should be regb == 0
	 * Lower 32 bits of regb are xor of upper and lower
	 *       32 bits of the 64 bits at the random address.
	 * Lower 32 bits of rega are shifted from upper 32 bits.
	 */
	asm volatile(
		"mov %0, %1\n"
		"rol $32, %1\n"
		"and $0x3F, %1\n"
		"add $0x601060, %k1\n"
		"rol $32, %1\n"
		"test %k0, %k1\n"
		"jne error\n"
		"movl 0x4(%k0), %k1\n"
		"xorl 0x0(%k0), %k1\n"
		"shr $32, %0\n"
		"error: xor %k0, %k1\n"
		: "+r" (rega), "+r" (regb)
		:
		:);
	printf("Post comparison:\n");
	printf("rega: 0x%016llx\n", rega);
	printf("regb: 0x%016llx\n", regb);
	return 0;
}
