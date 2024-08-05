http://www.wowotech.net/kernel_synchronization/per-cpu.html

每一个 per-cpu 变量在每一个 CPU 都有一份副本


            +-------------+
            |             |
            |             |
            |             |
            +-------------+
     +----> |             | <-- cpu3 per-cpu value address3
     |      +-------------+
     +----> |             | <-- cpu2 per-cpu value address2
     |      +-------------+
     +----> |             | <-- cpu1 per-cpu value address1
     |      +-------------+
copy +----- |             | <-- define per-cpu value address
            +-------------+
            |             |
            |             |
            |             |
            +-------------+


offset_array[cpu_id] =  {(define per-cpu value address) - (cpux per-cpu value addressx), ...}

get_cpu_xxx(define per-cpu value address)
	__get_cpu_xxx(define per-cpu value address, cpu_id)

