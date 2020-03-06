# 1 "device/ide.c"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 32 "<command-line>" 2
# 1 "device/ide.c"
# 1 "device/ide.h" 1


# 1 "./lib/stdio.h" 1


# 1 "./kernel/global.h" 1



# 1 "./lib/kernel/stdint.h" 1



typedef signed char int8_t;
typedef signed short int int16_t;
typedef signed int int32_t;
typedef signed long long int int64_t;
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;
# 5 "./kernel/global.h" 2
# 1 "./lib/kernel/stdbool.h" 1


# 1 "./lib/kernel/stdint.h" 1
# 4 "./lib/kernel/stdbool.h" 2

typedef uint8_t bool;
# 6 "./kernel/global.h" 2
# 122 "./kernel/global.h"
typedef struct gdt_desc
{

    uint16_t limit_low_word;


    uint16_t base_low_word;


    uint8_t base_mid_byte;


    uint8_t attr_low_byte;


    uint8_t limit_high_attr_high;


    uint8_t base_high_byte;
}gdt_desc;
# 4 "./lib/stdio.h" 2




typedef char* va_list;
# 21 "./lib/stdio.h"
uint32_t vsprintf(char* str, const char* format, va_list ap);


uint32_t printf(const char* format, ...);


void sprintf(char* str, const char* format, ...);
# 4 "device/ide.h" 2
# 1 "./lib/kernel/bitmap.h" 1



# 1 "./lib/kernel/stdbool.h" 1
# 5 "./lib/kernel/bitmap.h" 2


typedef struct bitmap
{
    uint32_t btmp_bytes_len;

    uint8_t* bits;
}bitmap;






 void bitmap_init(bitmap* btmp);
# 28 "./lib/kernel/bitmap.h"
 bool bitmap_scan_test(const bitmap* btmp, const uint32_t bit_idx);
# 37 "./lib/kernel/bitmap.h"
 int bitmap_scan(const bitmap* btmp, const uint32_t cnt);
# 47 "./lib/kernel/bitmap.h"
 void bitmap_set(bitmap* btmp, const uint32_t bit_idx, const int8_t value);
# 5 "device/ide.h" 2
# 1 "./lib/kernel/list.h" 1
# 18 "./lib/kernel/list.h"
typedef struct list_elem
{

    struct list_elem* prev;


    struct list_elem* next;
}list_elem;


typedef struct list
{

    list_elem head;


    list_elem tail;
}list;


typedef bool function(list_elem*, int arg);
# 47 "./lib/kernel/list.h"
void list_init(list* _list);
# 56 "./lib/kernel/list.h"
void list_insert_before(list_elem* before, list_elem* elem);







void list_push_head(list* pList, list_elem* elem);
# 73 "./lib/kernel/list.h"
void list_push_back(list* pList, list_elem* elem);







void list_remove(list_elem* pElem);







list_elem* list_pop_head(list* pList);
# 98 "./lib/kernel/list.h"
bool elem_find(list* pList, list_elem* obj_elem);
# 109 "./lib/kernel/list.h"
list_elem* list_traversal(list* pList, function* func, int arg);







uint32_t list_len(list* pList);







bool list_empty(list* pList);
# 6 "device/ide.h" 2
# 1 "./kernel/thread/sync.h" 1




# 1 "./kernel/thread/thread.h" 1




# 1 "./kernel/memory.h" 1



# 1 "./kernel/global.h" 1
# 5 "./kernel/memory.h" 2




typedef struct mem_block
{

    list_elem free_elem;
}mem_block;


typedef struct mem_block_desc
{

    uint32_t block_size;


    uint32_t blocks_pre_arena;


    struct list free_list;
}mem_block_desc;







typedef struct virtual_addr
{

    bitmap vaddr_bitmap;


    uint32_t vaddr_start;

}virtual_addr;


typedef enum pool_flags
{

    PF_KERNEL = 1,


    PF_USER = 2
}pool_flags;
# 93 "./kernel/memory.h"
void* get_kernel_pages(const uint32_t pg_cnt);






void mem_init();







void* get_user_pages(uint32_t pg_cnt);
# 117 "./kernel/memory.h"
void* get_a_page(pool_flags pf, uint32_t vaddr);


void block_desc_init(mem_block_desc* desc_array);


void* sys_malloc(uint32_t size);


void sys_free(void* ptr);
# 6 "./kernel/thread/thread.h" 2


typedef int16_t pid_t;


typedef void thread_func(void*);


typedef enum task_status
{

    TASK_RUNNING,


    TASK_READY,


    TASK_BLOCKED,

    TASK_WAITING,


    TASK_HANGING,


    TASK_DIED
}task_status;







typedef struct intr_stack
{

    uint32_t vec_no;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;


    uint32_t esp_dummy;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

    uint32_t err_code;
    void (*eip)(void);
    uint32_t cs;
    uint32_t eflags;


    void* esp;


    uint32_t ss;
}intr_stack;




typedef struct thread_stack
{
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;






    void (*eip)(thread_func* func, void* func_arg);






    void (*unused_retaddr);


    thread_func* function;


    void* func_arg;
}thread_stack;


typedef struct task_struct
{

    uint32_t* self_kstack;

    pid_t pid;


    task_status status;


    uint8_t priority;


    char name[14];


    uint8_t ticks;


    uint32_t elapsed_ticks;


    list_elem general_tag;


    list_elem all_list_tag;


    virtual_addr userprog_vaddr;


    uint32_t* pgdir_vaddr;


    mem_block_desc u_block_desc[7];


    uint32_t stack_magic;
}task_struct;







task_struct* get_running_thread_pcb();
# 161 "./kernel/thread/thread.h"
task_struct* thread_start(char* name,
                          int prio,
                          thread_func* function,
                          void* func_arg);






void thread_init(void);







void thread_block(const task_status stat);







void thread_unblock(task_struct* pthread);


void thread_yield(void);
# 6 "./kernel/thread/sync.h" 2
# 1 "kernel/semaphore/semaphore.h" 1



# 1 "./kernel/thread/thread.h" 1
# 5 "kernel/semaphore/semaphore.h" 2



typedef struct semaphore
{

    int value;


    list waiters;
}semaphore;
# 24 "kernel/semaphore/semaphore.h"
void sema_init(semaphore* psema, int value);







void sema_down(semaphore* psema);







void sema_up(semaphore* psema);
# 7 "./kernel/thread/sync.h" 2


typedef struct lock
{

    task_struct* holder;


    semaphore semaphore_binary;


    uint32_t holder_repeat_nr;
}lock;







void lock_init(lock* plock);







void lock_acquire(lock* plock);







void lock_release(lock* plock);
# 7 "device/ide.h" 2


typedef struct partition
{

    uint32_t start_lba;


    uint32_t sec_cnt;


    struct disk* my_disk;


    list_elem part_tag;


    char name[8];





    bitmap block_bitmap;


    bitmap inode_bitmap;


    list open_inode;
}partition;

typedef struct disk
{

    char name[8];


    struct ide_channel* my_channel;


    uint8_t dev_no;


    partition prim_parts[4];


    partition logic_parts[8];
}disk;

typedef struct ide_channel
{

    char name[16];


    uint16_t port_base;


    uint8_t irq_no;


    lock channel_lock;


    bool expecting_intr;


    semaphore disk_done;


    disk devices[2];
}ide_channel;



void ide_read(disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt);


void ide_write(disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt);


void ide_init();
# 2 "device/ide.c" 2
# 1 "./lib/kernel/stdio_kernel.h" 1




void printk(const char* format, ...);
# 3 "device/ide.c" 2
# 1 "./kernel/debug.h" 1
# 13 "./kernel/debug.h"
void panic_spin(char* filename, int line, const char* func, const char* condition);
# 4 "device/ide.c" 2
# 1 "./lib/kernel/io.h" 1
# 15 "./lib/kernel/io.h"
static inline void outb(uint16_t port, uint8_t data)
{


  asm volatile ("outb %b0, %w1" : : "a"(data), "Nd"(port));
}


static inline void outsw(uint16_t port, const void* addr, uint32_t word_cnt)
{






  asm volatile ("cld; rep movsw": "+S"(addr), "+c"(word_cnt): "d"(port));
}


static inline uint8_t inb(uint16_t port)
{
  uint8_t data;

  asm volatile("inb %w1, %b0": "=a"(data): "Nd"(port));
  return data;
}


static inline void insw(uint16_t port, void* addr, uint32_t word_cnt)
{


  asm volatile ("cld; rep insw": "+D"(addr), "+c"(word_cnt): "d"(port): "memory");
}
# 5 "device/ide.c" 2
# 1 "device/timer.h" 1






void mtime_sleep(uint32_t m_seconds);







void timer_init();
# 6 "device/ide.c" 2
# 1 "./kernel/interrupt.h" 1
# 9 "./kernel/interrupt.h"
typedef void* intr_handler;






typedef enum intr_status
{
    INTR_OFF,
    INTR_ON
}intr_status;






enum intr_status get_intr_status();







enum intr_status set_intr_status(enum intr_status status);






enum intr_status intr_enable();






enum intr_status intr_disable();


void idt_init(void);







void register_handler(const uint8_t vector_no, intr_handler function);
# 7 "device/ide.c" 2
# 43 "device/ide.c"
uint8_t channel_cnt;


ide_channel channels[2];


static int32_t ext_lba_base = 0;


static uint8_t p_no = 0;

static uint8_t l_no = 0;


list partition_list;

struct partition_table_entry
{

    uint8_t bootable;


    uint8_t start_head;


    uint8_t start_sec;


    uint8_t start_chs;


    uint8_t fs_type;


    uint8_t end_head;


    uint8_t end_sec;


    uint8_t end_chs;


    uint32_t start_lba;


    uint32_t sec_cnt;
}__attribute__((packed));

typedef struct partition_table_entry partition_table_entry;

struct boot_sector
{

    uint8_t other[446];
    partition_table_entry partition_table[4];


    uint16_t signature;
}__attribute__((packed));

typedef struct boot_sector boot_sector;


static void swap_pairs_bytes(const char* dst, char* buf, uint32_t len)
{
    uint8_t idx = 0;
    for(idx = 0; idx < len; idx++)
    {

        buf[idx + 1] = *dst++;
        buf[idx] = *dst++;
    }
    buf[idx] = '\0';
}


static void select_disk(disk* hd)
{

    uint8_t reg_device = 0xa0 | 0x40;

    if(hd->dev_no == 1)
    {
        reg_device |= 0x10;
    }

    outb((hd->my_channel->port_base + 6), reg_device);
}


static void select_sector(disk* hd, uint32_t lba, uint8_t sec_cnt)
{
    if(lba <= ((80 * 1024 * 1024 / 512) - 1)) {} else {panic_spin("device/ide.c", 136, __func__, "lba <= max_lba"); };
    ide_channel* channel = hd->my_channel;


    outb((hd->my_channel->port_base + 2), sec_cnt);


    outb((hd->my_channel->port_base + 3), lba);
    outb((hd->my_channel->port_base + 4), lba >> 8);
    outb((hd->my_channel->port_base + 5), lba >> 16);

    uint8_t reg_device = 0xa0 | 0x40 |
            (hd->dev_no == 1 ? 0x10 : 0) |
            lba >> 24;


    outb((hd->my_channel->port_base + 6), reg_device);
}


static void cmd_out(ide_channel* channel, uint8_t cmd)
{

    channel->expecting_intr = 1;
    outb((channel->port_base + 7), cmd);
}


static read_from_sector(disk* hd, void* buf, uint8_t sec_cnt)
{
    uint32_t size_in_byte;


    if(sec_cnt == 0)
    {
        size_in_byte = 256 * 512;
    }
    else
    {
        size_in_byte = sec_cnt * 512;
    }

    insw((hd->my_channel->port_base + 0), buf, size_in_byte / 2);
}


static void write2sector(disk* hd, void* buf, uint8_t sec_cnt)
{
    uint32_t size_in_byte;
    if(sec_cnt == 0)
    {
        size_in_byte = 256 * 512;
    }
    else
    {
        size_in_byte = sec_cnt * 512;
    }
    outsw((hd->my_channel->port_base + 0), buf, size_in_byte / 2);
}


static bool busy_wait(disk* hd)
{
    printk(" busy_wait ");
    ide_channel* channel = hd->my_channel;
    uint16_t time_limit = 30 * 1000;
    for(; time_limit >= 0; time_limit -= 10)
    {
        if(!(inb((hd->my_channel->port_base + 7)) & 0x80))
        {
            return inb((hd->my_channel->port_base + 7)) & 0x8;
        }
        else
        {

            mtime_sleep(10);
        }
    }
    printk("disk not ready!\n");
    return 0;
}


static void identify_disk(disk* hd)
{
    char id_info[512];
    select_disk(hd);


    cmd_out(hd->my_channel, 0xec);


    sema_down(&hd->my_channel->disk_done);

    if(!busy_wait(hd))
    {
        char error[64];
        sprintf(error, "%s identify failed||||||\n", hd->name);
        panic_spin("device/ide.c", 234, __func__, error);
    }

    printk("sema pu\n");


    read_from_sector(hd, id_info, 1);


    char buf[64];


    uint8_t sn_start = 10 * 2;


    uint8_t sn_len = 20;


    uint8_t md_start = 27 * 2;


    uint8_t md_len = 40;

    swap_pairs_bytes(&id_info[sn_start], buf, sn_len);

    printk("disk %s info;\n  SN: %s\n", hd->name, sn_len);
    memset(buf, 0, sizeof(buf));
    swap_pairs_bytes(&id_info[md_start], buf, md_len);
    printk(" MODULE: %s \n", buf);


    uint32_t sectors = *(uint32_t*)&id_info[60 * 2];
    printk(" SECTORS: %d\n", sectors);
    printk(" CAPACITY: %dMB\n", sectors * 512 / 1024 / 1024);
}


static void partition_scan(disk* hd, uint32_t ext_lba)
{

    boot_sector* bs = sys_malloc(sizeof(boot_sector));
    ide_read(hd, ext_lba, bs, 1);
    partition_table_entry* p = bs->partition_table;


    for(uint8_t part_idx = 0; part_idx < 4; part_idx++)
    {
        if(p->fs_type == 0x5)
        {
            if(ext_lba_base != 0)
            {

                partition_scan(hd, p->start_lba + ext_lba_base);
            }
            else
            {

                ext_lba_base = p->start_lba;
                partition_scan(hd, p->start_lba);
            }
        }
        else if(p->fs_type != 0)
        {
            if(ext_lba == 0)
            {
                hd->prim_parts[p_no].start_lba = ext_lba + p->start_lba;
                hd->prim_parts[p_no].sec_cnt = p->sec_cnt;
                hd->prim_parts[p_no].my_disk = hd;
                list_push_back(&partition_list, &hd->prim_parts[p_no].part_tag);
                sprintf(hd->prim_parts[p_no].name, "%s%s", hd->name, p_no + 1);
                p_no++;
                if(p_no < 4) {} else {panic_spin("device/ide.c", 305, __func__, "p_no < 4"); };
            }
            else
            {
                hd->logic_parts[l_no].start_lba = ext_lba + p->start_lba;
                hd->logic_parts[l_no].sec_cnt = p->sec_cnt;
                hd->logic_parts[l_no].my_disk = hd;
                list_push_back(&partition_list, &hd->logic_parts[l_no].part_tag);
                sprintf(hd->logic_parts[l_no].name, "%s%s", hd->name, l_no + 5);
                l_no++;
                if(l_no >= 8)
                {
                    return;
                }
            }
        }
        p++;
    }
    sys_free(bs);
}


static partition_info(list_elem* pelem, int arg __attribute__((unused)))
{
    partition* part = (partition*)((uint32_t)pelem - (uint32_t)(&((partition*)0)->part_tag));
    printk(" %s start_lba; 0x%x, sec_cnt: 0x%x\n", part->name, part->start_lba, part->sec_cnt);

    return 0;
}


void ide_read(disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt)
{
    lock_acquire(&hd->my_channel->channel_lock);


    select_disk(hd);


    uint32_t secs_op;


    uint32_t secs_done = 0;


    while(secs_done < sec_cnt)
    {
        if(secs_done + 256 <= sec_cnt)
        {
            secs_op = 256;
        }
        else
        {
            secs_op = sec_cnt - secs_done;
        }


        select_sector(hd, lba + secs_done, secs_op);


        cmd_out(hd->my_channel, 0x20);





        sema_down(&hd->my_channel->disk_done);


        if(!busy_wait(hd))
        {
            char error[64];
            sprintf(error, "%s read sector %d failed|||\n", hd->name, lba);
            panic_spin("device/ide.c", 378, __func__, error);
        }


        read_from_sector(hd, (void*)((uint32_t)buf + secs_done * 512),
                        secs_op);

        secs_done += secs_op;
    }
    lock_release(&hd->my_channel->channel_lock);
}


void ide_write(disk* hd, uint32_t lba, void* buf, uint32_t sec_cnt)
{
    lock_acquire(&hd->my_channel->channel_lock);


    select_disk(hd);

    uint32_t secs_op;
    uint32_t secs_done = 0;
    while(secs_done < sec_cnt)
    {
        if(secs_done + 256 <= sec_cnt)
        {
            secs_op = 256;
        }
        else
        {
            secs_op = sec_cnt - secs_done;
        }


        select_sector(hd, lba + secs_done, secs_op);


        cmd_out(hd->my_channel, 0x30);


        if(!busy_wait(hd))
        {
            char error[64];
            sprintf(error, "%s write sector %d failed|||||\n", hd->name, lba);
            panic_spin("device/ide.c", 422, __func__, error);
        }


        write2sector(hd, (void*)((uint32_t)buf + secs_done * 512), secs_op);


        sema_down(&hd->my_channel->disk_done);

        secs_done += secs_op;

    }
    lock_release(&hd->my_channel->channel_lock);
}


void intr_hd_handler(uint8_t irq_no)
{

    uint8_t ch_no = irq_no - 0x2e;

    ide_channel* channel = &channels[ch_no];


    if(channel->expecting_intr == 1)
    {
        channel->expecting_intr = 0;
        sema_up(&channel->disk_done);


        inb((channel->port_base + 7));
    }
}



void ide_init()
{
    printk("ide init start\n");


    uint8_t hd_cnt = *((uint8_t*)(0x475));

    if(hd_cnt > 0) {} else {panic_spin("device/ide.c", 465, __func__, "hd_cnt > 0"); };

    list_init(&partition_list);

    channel_cnt = ((hd_cnt + 2 - 1) / (2));

    ide_channel* channel;

    uint8_t dev_no = 0;

    for(uint8_t channel_no = 0; channel_no < channel_cnt; channel_no++)
    {
        channel = &channels[channel_no];
        sprintf(channel->name, "ide%d", channel_no);

        switch(channel_no)
        {
        case 0:

            channel->port_base = 0x1f0;

            channel->irq_no = 0x20 + 14;
            break;
        case 1:

            channel->port_base = 0x170;

            channel->irq_no = 0x20 + 15;
            break;
        }

        channel->expecting_intr = 0;

        lock_init(&channel->channel_lock);


        sema_init(&channel->disk_done, 0);

        register_handler(channel->irq_no, intr_hd_handler);

        while(dev_no < 2)
        {
            disk* hd = &channel->devices[dev_no];
            hd->my_channel = channel;
            hd->dev_no = dev_no;
            sprintf(hd->name, "sd%c", "a" + channel_no * 2 + dev_no);

            printk("hd name %s\n", hd->name);

            identify_disk(hd);
            if(dev_no != 0)
            {
                partition_scan(hd, 0);
            }
            p_no = 0;
            l_no = 0;
            dev_no++;
        }

        dev_no = 0;
    }


    list_traversal(&partition_list, partition_info, (int)((void*)0));

    printk("ide_init done\n");
}
