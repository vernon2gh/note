#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/kvm.h>

const uint8_t code[] = {
	0xba, 0xf8, 0x03, /* mov $0x3f8, %dx */
	0x00, 0xd8,       /* add %bl, %al */
	0x04, '0',        /* add $'0', %al */
	0xee,             /* out %al, (%dx) */
	0xb0, '\n',       /* mov $'\n', %al */
	0xee,             /* out %al, (%dx) */
	0xf4,             /* hlt */
};

int main(int argc, char *argv[])
{
	int kvm, vmfd, vcpufd;
	struct kvm_run *run;
	char *mem;
	int mmap_size;
	int ret;

	kvm = open("/dev/kvm", O_RDWR | O_CLOEXEC);

	ret = ioctl(kvm, KVM_GET_API_VERSION, NULL);
	if (ret == -1)
		printf("KVM_GET_API_VERSION failed.");
	if (ret != 12)
		printf("KVM_GET_API_VERSION %d, expected 12", ret);

	vmfd = ioctl(kvm, KVM_CREATE_VM, (unsigned long)0);
	mem = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	memcpy(mem, code, sizeof(code));

	struct kvm_userspace_memory_region region = {
		.slot = 0,
		.guest_phys_addr = 0x1000,
		.memory_size = 0x1000,
		.userspace_addr = (uint64_t)mem,
	};
	ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &region);

	vcpufd = ioctl(vmfd, KVM_CREATE_VCPU, (unsigned long)0);

	mmap_size = ioctl(kvm, KVM_GET_VCPU_MMAP_SIZE, NULL);
	run = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, vcpufd, 0);

	struct kvm_sregs sregs;
	ioctl(vcpufd, KVM_GET_SREGS, &sregs);
	sregs.cs.base = 0;
	sregs.cs.selector = 0;
	ioctl(vcpufd, KVM_SET_SREGS, &sregs);

	struct kvm_regs regs = {
		.rip = 0x1000,
		.rax = 2,
		.rbx = 2,
		.rflags = 0x2,
	};
	ioctl(vcpufd, KVM_SET_REGS, &regs);

	while(1) {
		ioctl(vcpufd, KVM_RUN, NULL);

		switch (run->exit_reason) {
		case KVM_EXIT_HLT:
			printf("KVM_EXIT_HLT\n");
			return 0;

		case KVM_EXIT_IO:
			if (run->io.direction == KVM_EXIT_IO_OUT && run->io.size == 1
			    && run->io.port == 0x3f8 && run->io.count == 1)
				putchar(*(((char *)run) + run->io.data_offset));
			else
				printf("unhandled KVM_EXIT_IO\n");

			break;

		case KVM_EXIT_FAIL_ENTRY:
			printf("KVM_EXIT_FAIL_ENTRY: hardware_entry_failure_reason = 0x%llx\n",
				(unsigned long long)run->fail_entry.hardware_entry_failure_reason);
			break;

		case KVM_EXIT_INTERNAL_ERROR:
			printf("KVM_EXIT_INTERNAL_ERROR: suberror = 0x%x\n",
				run->internal.suberror);
			break;
		}
	}
}
