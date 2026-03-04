.globl irq_vector_init
irq_vector_init:
    adr x0, vectors     // load VBAR_EL1 with virtual
    msr vbar_el1, x0    // vector table address
    ret


// arm processor state has 4 bits for storing the mask status for different interrupts
// d - masks debug exc
// a - masks SErrors (called A because SErrors are sometimes called asynchronous aborts)
// i - masks irqs
// f - masks fiqs
// hence the daifclr and daifset below
    
.globl enable_irq
enable_irq:
    msr daifclr, #2     // this and teh function below use the #2 because we only want to set/clear the second bit (1 << 2)
    ret


.globl disable_irq
disable_irq:
    msr daifset, #2
    ret