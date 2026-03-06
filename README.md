chapter summaries:

chap1
    add makefile
    add linker script
    add boot.S
        sets ".section .text.boot" which makes everything defined in the .S file included in the .text.boot section (this is used by the linker script)
        
        defines a _start function
            reads from special reg mpidr_el1 into x0, which gets processor id
            logical and the value in x0 with #0xFF to check the processor id
            cbz (compare, branch if zero) x0 and go to master if zero
            go to proc_hang

        defines a proc_hang, which just branches to itself (b proc_hang)
        
        defines a master function
            we read bss_begin and bss_end into x0 and x1 (this uses adr, which the arm docs do a terrible job explaining)
            we subtract bss_begin from bss_end and write into x1
            we branch with link to memzero
        
        defines memzero
            moves the stack pointer to #LOW_MEMORY, which i think is the start of available memory?
            branch with link to kernel_main -- i think that a branch with link
            
            
    add uart initialization
        set special registers to u32 values
        
    sending data to the mini uart
        enter an infinite loop that breaks when AUX_MU_LSR_REG is free 0x20 (if we're sending)
        or ready 0x01 (if we're receiving)
        once ready, we write to AUX_MU_IO_REG our char, or we
        read from AUX_MU_IO_REG (we logical & this value with 0xFF to get the bottom 8 bits. we're only sending/receiving 8bit chars but we're writing/reading 32bits)

there's some other shit in here about the linux project structure and kernel build systems


chap2
    introduces exception levels
    need to add printf functionality so that we can print the exception level the kernel is currently running on
    a program can't raise its own exception level. current el can only be changed if an exception is generated.
    an exception is generated if a program tries to execute an instruction that its current EL doesn't allow, or if it runs an "svc" instruction intentionally
    hardware can also generate interrupts, which are a special type on exception.
    
    whenever an exception is generated, the following sequence happens:
        the address of the current instruction is saved to ELR_EL<n> (the exception link reg)
        current processor's state is stored in SPSR_EL<n> (the saved program status reg)
        an exception handler handles the exception
        the exception handler calls eret, restoring processor state from SPSR_EL<n> and jumping to ELR_EN<n>
        the processor state contains:
            condition flags (negative N, zero A, unsigned overflow C, signed overflow V). these are used in branch instrs
            interrupt disable bits. used to enable/disable different types of interrupts
            some other stuff required to fully restore processor state
    
    the exception handler can actually modify both ELR_EL<n> and SPSR_EL<n>, so the exception handler doesn't necessarily return to the exact location
    this functionality is used for switching between EL3 and EL1
    
    we switch our OS from EL0 to EL1.
    to do this, we:
    
    ldr x0, =SCTLR_VALUE_MMU_DISABLED
    msr sctlr_el1, x0                   // system control reg (EL1). setting different values here configures the behaviour of the processor while it is in EL1. importantly, whether the MMU is turned on or not
    
    ldr x0, =HCR_VALUE
    msr hcr_el2, x0                     // hypervisor config reg (EL2). not entirely sure what this register does. guide says it "controls the execution state at EL1"
    
    ldr x0, =SCR_VALUE
    msr scr_el3, x0                     // secure config reg (EL3). configures security settings. for example, controls whether lower levels are executed in "secure" or "nonsecure" state
    
    ldr x0, =SPSR_VALUE
    msr spsr_el3, x0                    // saved program status reg (EL3). should go back in and get gpt to explain the reasoning behind manually saving this
    
    adr x0, el1_entry
    msr elr_el3, x0                     // exception link register (EL3). holds the address we return to when the eret instruction is executed. we overwrite this with the el1_entry label
    
    eret

    we're basically just setting some sys regs using predefined values
    
    
    
    there's some additional exploration of the linux kernel in here as well.
    some stuff about stext, preserve_boot_args (there's some weird shit going on with "cache line maintenance" in this function)
        preserve boot args:
            when the kernel boots, the hardware puts some important information (the addr of the device tree blob) into x0.
            the kernel is about to overwrite x0, so we save it out
            still don't understand this cache line invalidation stuff   
    el2_setup
        kernel can be booted in either el1 or el2. el2 gives us access to virtualization
        and we can act as a host os.
        
    interestingly, linux places the kernel at a random space in layout for security
    
chap3
    so our kernel should now be in el1
    
    the way we communicate with hardware is asynchronous. we send some command to a device, and it notifies us after some interval when the work is completed.
    these async notifications are called "interrupts" and our processor has to issue an interrupt handler to manage them.
    
    we can use a timer to issue interrupts at some predefined interval. we can use this in our scheduler
    
    ## there's some stuff about the vector base address register that i should probably go
    back and look at. from memory, its a register address of the first item of an array meant for handling interrupts or something
    
    
    
    in arm, interrupts are a subcategory of exceptions. there are four types of exceptions
        synchronous:
            generated by the currently executed software instruction.
            
        interrupt request:
            standard async interrupt. always generated by hardware.
            
        fast interrupt request:
            this is a high priority exception 
            
        system error:
            async exception generated by hardware, but this always indicates some error cond
            
    
    each exception type needs its own handler, and a separate handler should be defined for each execution state
    execution states:
        EL1t:
            an exception occurs at EL1 while the SP was shared with EL0. happens when SPSel reg holds value 0
        EL1h:
            exception occurs at EL1 while SP was in EL1. SPSel reg holds 1.
        EL0_64:
            exception taken from EL0 in 64bit mode
        EL0_32:
            exception taken from EL0 in 32bit mode
            
    explanation of the "SP shared with EL0 stuff":
        
        // i want more clarification/confirmation that there are distinct stacks and stack pointers for each exception level
        // and how these are used
        in AArch64, each exception level has its own stack and stack pointer
        if SPSel = 0, the processor uses the stack pointer SP_EL0 regardless of the actual exception level
        if SPSel = 1, the processor uses the stack pointer associated with its current EL, (i.e if EL = 1, we use SP_EL1)
        
        having a separate stack for the os to use means it has a "clean" stack to work with. what are the implications of this?
        
        if we're using SP_EL0 when an exception is generated, we know that the exception occurred while user-space code was being executed. this means either the code either generated an exception (like a malloc request), or a hardware interrupt was generated. 
        because we know that we were in the middle of some user space code executing, we know that we'll need to save out our stack state so that we can restore it later.
        in contrast, i dont think we need to do this if we're using SP_EL1.
        
        
    
    there are 16 total execution states. the four above are repeated for EL3, 2, and 0
    
    we can mask interrupts (prevent them from actually interrupting a process).
    this is important because if we get interrupted in the middle of saving processor state, the original state becomes unrecovereable
    
    daifset and daifclr are used to set/clear interrupt masks
        d: masks debug exceptions, which are a type of synchronous exception
        a: masks SErrors, which are also called asynchronous aborts
        i: masks IRQs
        f: masks FIQs
        
    a device doesn't interrupt the processor directly. it communicates with the interrupt handler, which manages it.
    the interrupt handler can also determine which piece of hardware generated the interrupt
    the pi has its own interrupt controller
    
    the guide mentions that you can have several exceptions occur at once.
    to handle this, you should wrap your handler's handle match statement 
    in a while loop
    
    guide creates a timer-based interrupt init. this is done by getting the current
    timer value from TIMER_CLO, incrementing it by some interval and then
    writing it back to TIMER_C1. i assume that the hardware has some built-in
    functionality that allows it to check whether TIMER_CLO & TIMER_C1 = 0x0 (they
    match), and then issue an interrupt.
    
    also creates a timer_interrupt_handler. this just increments the current timer
    val and then writes into TIMER_C1 like the init above. we also have to ack
    the timer interrupt by writing 1 into the TIMER_CS reg. we then printf that
    we received a timer interrupt so we know its happened.
    ^^^
    the timer uses curVal to track the current value of the timer. for some reason
    this is a global value, and the function signature of our timer irq handler
    doesn't take in any args? maybe worth clarifying why this is?
     
    
chap4
    scheduler
    we have some schedule(), process() and copy_process() fns

    schedule:
        loop            
            iterate through tasks from 0..NR_TASKS
            if *p = tasks[i] exists and p.state == running and p.counter > -1 set next to i and c to p.counter
            if c is set (not -1, which is what it is initialized to), then break
            iterator through tasks from 0..NR_TASKS again
            if *p = task[i] exists, then p.counter = (p.counter >> 1) + p.priority
                                                       ^^^^^^^^^^^^^
                                                       why lshift this? div by 2
                                                       we're basically updating the counters of all of our tasks by dividing them by 2
        call switch_to(task[next])
        call preempt_enable() 
    
    process:
        just a placeholder process. i think this ends up being some dynamic function in reality?
        
        take in an array
        loop
            send array[i]
            delay
    
    
    copy_process:
        takes in a function and an argument to the function
        
        preempt_disable()                        // have a feeling this is probably masking interrupts
        
        task_struct *p;
        
        p = (task_struct *) get_free_page()
        if not p return err val
        page.priority = current.priority        // need to figure out where current is coming from, not defined in fork.c
        page.state = running
        page.counter = page.priority
        p.preempt_count = 1
        p.cpu_context.x19 = input function
        p.cpu_context.x20 = ret_from_fork       // this isn't defined in fork.c either
        p.cpu_context.sp = p + THREAD_SIZE      // this isn't defined 
        increment NR_TASKS and assign to pid    // not sure if this is actually the same NR_TASKS as above, but its definitely not defind in fork.c
        tasks[pid] = p
        
        preempt_enable
        
        // what is actually going on here, semantically?
        // we get some page/task_struct from get_free_page()
        // we set the properties of the task_struct to some values
        // we append the tasks to tasks[] and then reenable preempt 
        
    switch_to:
        takes in task_struct *next
        if current is next do nothing               // again, not sure where current is coming from
        move global task_struct *prev to current    // prev will be in the same place as current
        update current = next
        call cpu_switch_to(prev, next)              // cpu switch to saves registers and then loads back in, but not entirely sure what is happening here
    
    cpu_switch_to:
        this is an asm function
        asm functions can take up to 8 args, which fill x0 - x7
        x8 
        
    
chap5
    syscalls and user space processes
    implement process isolation

chap6