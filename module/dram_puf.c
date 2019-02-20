
/***
The below code tests for 60 secs for running a loop continuously
In the code, we can execute it for 60 secs


Performs Rowhammering Only



***/


/***********Header Files************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>  // for threads
#include <linux/time.h>   // for using jiffies 
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <bcm_host.h>

/************ Variable Declaration *********/
static unsigned int puf_init_val = 0xaaaaaaaa;
module_param(puf_init_val, uint, S_IRUGO);
static unsigned int puf_delaysec =60;
module_param(puf_delaysec , uint, S_IRUGO);
static unsigned int puf_base_address =0xa0000000;
module_param(puf_base_address, uint, S_IRUGO);
static char *mystring = "once";
module_param(mystring, charp, S_IRUGO);
MODULE_PARM_DESC(mystring, "A character string");
static unsigned int hammer_init_value = 0x55555555;
module_param(hammer_init_value, uint, S_IRUGO);
static unsigned int no_hammer_rows = 32;
module_param(no_hammer_rows, uint, S_IRUGO);
static unsigned int no_of_measurements_per_sampledecay = 20;
module_param(no_of_measurements_per_sampledecay, uint,S_IRUGO);

static unsigned int no_PUF_rows = 32;
module_param(no_PUF_rows, uint, S_IRUGO);

char *hammerall = "all";

unsigned int puf_complete_flag=1;
unsigned int PUF_size=1024;                     //1024*4 byte//
unsigned int OMAP_EMIF2 =0x4d000010;
unsigned int OMAP_EMIF2_SHW =0x4d000014;
unsigned int OMAP_EMIF2_temp_polling =0x4d0000cc;

#define same_bank_row_size 0x8000
#define row_size 0x1000		/*Size of 4Kb Data Chunk*/

//static struct delayed_work PUF_work;   -- Used by earlier method for scheduling the task

// Below are variables for thread creation
int len = sizeof(struct task_struct);
struct task_struct *thread1;

// The below pointer declaration is used for cache operation
unsigned int *cache_mem_ptr;



/************Functions for Reading and Writing**************************/
/*
*   This function writes the value write_vale to the system address system_addr.
*/
void write_OMAP_system_address(unsigned int system_addr,unsigned int write_val){
  void *write_virtaddr;
  //unsigned int written_value;

  write_virtaddr = ioremap(system_addr,sizeof(unsigned int));
  *((unsigned int*)write_virtaddr)=write_val;
  iounmap(write_virtaddr);
}

/*
*   This function reads the value from system address system_addr.
*/
void read_OMAP_system_address(unsigned int system_addr, unsigned int puf_init_val){
  void *read_virtaddr;
  unsigned int x;	
  read_virtaddr = ioremap(system_addr, sizeof(unsigned int));
  x = *((unsigned int*)read_virtaddr);
  if (x != puf_init_val)
  printk(KERN_ALERT "R:0x%08x A:0x%08x\n",*((unsigned int*)read_virtaddr), system_addr);
  //printk(KERN_ALERT "%x\n",x);
  iounmap(read_virtaddr);
}

/*
*   This function disables the DRAM refresh of the external memory interface 2 (EMIF2). Note, that
*  if DRAM refresh of EMIF1 is disabled, it will not be possible to boot a linux kernel.
*/
void disable_refresh(void){
  void *read_virtaddr;     //read address for EMIF 2 register
  void *write_virtaddr;    //write address for EMIF 2 register

  write_virtaddr = ioremap(OMAP_EMIF2,sizeof(unsigned int));
  read_virtaddr = ioremap(OMAP_EMIF2, sizeof(unsigned int));
  *((unsigned int*)write_virtaddr)=0x80000000;
  printk(KERN_ALERT "EMIF 2 REG Write 0x%08x at 0x%08x\n",*((unsigned int*)write_virtaddr),OMAP_EMIF2);
  printk(KERN_ALERT "EMIF 2 REG Read 0x%08x at 0x%08x\n",*((unsigned int*)read_virtaddr),OMAP_EMIF2);
  iounmap(read_virtaddr);
  iounmap(write_virtaddr);

  write_virtaddr = ioremap(OMAP_EMIF2_SHW,sizeof(unsigned int));
  read_virtaddr = ioremap(OMAP_EMIF2_SHW, sizeof(unsigned int));
  *((unsigned int*)write_virtaddr)=0x80000000;
  printk(KERN_ALERT "EMIF 2 REG Write 0x%08x at 0x%08x\n",*((unsigned int*)write_virtaddr),OMAP_EMIF2_SHW);
  printk(KERN_ALERT "EMIF 2 REG Read 0x%08x at 0x%08x\n",*((unsigned int*)read_virtaddr),OMAP_EMIF2_SHW);
  iounmap(read_virtaddr);
  iounmap(write_virtaddr);

  write_virtaddr = ioremap( OMAP_EMIF2_temp_polling,sizeof(unsigned int));
  read_virtaddr = ioremap( OMAP_EMIF2_temp_polling, sizeof(unsigned int));
  *((unsigned int*)write_virtaddr)=0x08016893;
  printk(KERN_ALERT "EMIF 2 REG Write 0x%08x at 0x%08x\n",*((unsigned int*)write_virtaddr),OMAP_EMIF2_temp_polling);
  printk(KERN_ALERT "EMIF 2 REG Read 0x%08x at 0x%08x\n",*((unsigned int*)read_virtaddr), OMAP_EMIF2_temp_polling);
  printk(KERN_ALERT "Temp polling alert disabled");
  printk(KERN_ALERT "Refresh Disabled at EMIF2");
  iounmap(read_virtaddr);
  iounmap(write_virtaddr);
}

/*
*   This function enables the DRAM refresh of the external memory interface 2 (EMIF2).
*/
void enable_refresh(void){
  void *read_virtaddr;     //read address for EMIF 2 register
  void *write_virtaddr;    //write address for EMIF 2 register

  write_virtaddr = ioremap(OMAP_EMIF2,sizeof(unsigned int));
  read_virtaddr = ioremap(OMAP_EMIF2, sizeof(unsigned int));
  *((unsigned int*)write_virtaddr)=0x00000618;
  printk(KERN_ALERT "EMIF 2 REG Write 0x%08x at 0x%08x\n",*((unsigned int*)write_virtaddr),OMAP_EMIF2);
  printk(KERN_ALERT "EMIF 2 REG Read 0x%08x at 0x%08x\n",*((unsigned int*)read_virtaddr),OMAP_EMIF2);
  iounmap(read_virtaddr);
  iounmap(write_virtaddr);

  write_virtaddr = ioremap(OMAP_EMIF2_SHW,sizeof(unsigned int));
  read_virtaddr = ioremap(OMAP_EMIF2_SHW, sizeof(unsigned int));
  *((unsigned int*)write_virtaddr)=0x00000618;
  printk(KERN_ALERT "EMIF 2 REG Write 0x%08x at 0x%08x\n",*((unsigned int*)write_virtaddr),OMAP_EMIF2_SHW);
  printk(KERN_ALERT "EMIF 2 REG Read 0x%08x at 0x%08x\n",*((unsigned int*)read_virtaddr),OMAP_EMIF2_SHW);
  iounmap(read_virtaddr);
  iounmap(write_virtaddr);

  write_virtaddr = ioremap( OMAP_EMIF2_temp_polling,sizeof(unsigned int));
  read_virtaddr = ioremap( OMAP_EMIF2_temp_polling, sizeof(unsigned int));
  *((unsigned int*)write_virtaddr)=0x58016893;
  printk(KERN_ALERT "EMIF 2 REG Write 0x%08x at 0x%08x\n",*((unsigned int*)write_virtaddr),OMAP_EMIF2_temp_polling);
  printk(KERN_ALERT "EMIF 2 REG Read 0x%08x at 0x%08x\n",*((unsigned int*)read_virtaddr), OMAP_EMIF2_temp_polling);
  printk(KERN_ALERT "Temp polling alert enabled");
  printk(KERN_ALERT "Refresh enabled at EMIF2");
  iounmap(read_virtaddr);
  iounmap(write_virtaddr);
}

/*
*   This function reads the contents of the PUF memory range.
*/

/* The below code is for testing purposes
*static unsigned int PUF_read_query()
*{
  unsigned int addr,puf_read_loop,puf_read_vale;
  puf_read_vale=0;
  addr=0xa0000000;

  printk(KERN_ALERT "PUF Query START.\n");
  for(puf_read_loop=0;puf_read_loop<PUF_size;puf_read_loop++){
    read_OMAP_system_address(addr);
    addr=addr+4;
  }

  printk(KERN_ALERT "PUF Query END.\n");
  enable_refresh();
  return puf_read_vale;
}*/

void read_row(unsigned int row_base_address, unsigned int puf_init_val){
	//unsigned int puf_read_value=0x0;
	unsigned int puf_address=row_base_address;

	for(puf_address=row_base_address;puf_address<(row_base_address+row_size);puf_address+=4){
		read_OMAP_system_address(puf_address, puf_init_val);
		
	}
}

void Read_puf(unsigned int puf_base_address,unsigned int no_PUF_rows,unsigned int pair_alternate_flag, unsigned int puf_init_val){

	unsigned int current_row=0;
	unsigned int puf_address=0;

	puf_address=puf_base_address+same_bank_row_size;  //Setting base address for PUF section @ ROW 1
	printk(KERN_ALERT "Starting PUF read-out\n");

	switch(pair_alternate_flag){ //Set Hammer rows
		case 0:
		{
			printk(KERN_ALERT "[i] Single-Sided Rowhammer (SSRH)\n");
			if(no_PUF_rows==1){
				//address_decode(puf_address,0); //Decode ROW and COL address form system address
				read_row(puf_address, puf_init_val);
			}
			else{
				for(current_row=0;current_row<no_PUF_rows/2;current_row++){
					//address_decode(puf_address,0);  //Decode ROW and COL address form system address
					read_row(puf_address, puf_init_val);
					puf_address=puf_address+same_bank_row_size;
					//address_decode(puf_address,0);  //Decode ROW and COL address form system address
					read_row(puf_address, puf_init_val);
					puf_address=puf_address+(2*same_bank_row_size);
				}
			}
			break;
		}
		default:
		{
			printk(KERN_ALERT "PUF Alternate Mode\n");
			for(current_row=0;current_row<no_PUF_rows*2;current_row++){
				if(current_row%2!=0){
					//address_decode(puf_address,0); //Decode ROW and COL address form system address
					read_row(puf_address, puf_init_val);
					puf_address=puf_base_address+((current_row+2)*same_bank_row_size);
				}
			}
			break;
		}
	}
	printk(KERN_ALERT "Finished PUF read-out\n");
}



/*
*   This function writes the initialization value to the PUF memory range.
*/
/* The below code is for testing purposes
static void PUF_write_query(void){
  unsigned int addr;
  unsigned int puf_write_loop;
  puf_write_loop=0;

  addr=0xa0000000;
  for(puf_write_loop=0;puf_write_loop<PUF_size;puf_write_loop++){
    write_OMAP_system_address(addr,puf_init_val);
    addr=addr+4;
  }
}
*/

void hammering_rows(unsigned int puf_base_address,unsigned int no_hammer_rows,unsigned int pair_alternate_flag){

	unsigned int hammer_address=0;
	unsigned int x=0;
	void *read_virtaddr;
	unsigned int current_row=0;
	hammer_address=puf_base_address; //Setting base address for Hammer section @ ROW 0

	switch(pair_alternate_flag){
		case 0:
		{
			if(no_hammer_rows==1){
				//address_decode(hammer_address,0); //Decode ROW and COL address form system address
				read_virtaddr = ioremap(hammer_address, sizeof(unsigned int));
  	 			x = *((unsigned int*)read_virtaddr);
        		iounmap(read_virtaddr);
				hammer_address=hammer_address+(1024*same_bank_row_size);
				read_virtaddr = ioremap(hammer_address, sizeof(unsigned int));
  	 			x = *((unsigned int*)read_virtaddr);
       			iounmap(read_virtaddr);
			}else{
				for(current_row=0;current_row<(no_hammer_rows/2)+1;current_row++){
				//address_decode(hammer_address,0); //Decode ROW and COL address form system address
					read_virtaddr = ioremap(hammer_address, sizeof(unsigned int));
  	 				x = *((unsigned int*)read_virtaddr);
        				iounmap(read_virtaddr);
					hammer_address=hammer_address+(3*same_bank_row_size);
				}
			}
			break;
		}
		default:
		{
			for(current_row=0;current_row<=no_hammer_rows*2;current_row++){
				if(current_row%2==0){
				//address_decode(hammer_address,0); //Decode ROW and COL address form system address
					read_virtaddr = ioremap(hammer_address, sizeof(unsigned int));
  	 				x = *((unsigned int*)read_virtaddr);
        				iounmap(read_virtaddr);
					hammer_address=puf_base_address+((current_row+2)*same_bank_row_size);
					}
				}
				break;
			}
	}
}


void write_row(unsigned int row_base_address,unsigned int write_value){
	unsigned int puf_address=row_base_address;

	for(puf_address=row_base_address;puf_address<(row_base_address+row_size);puf_address+=4){
		write_OMAP_system_address(puf_address,write_value);
	}
}

void Init_puf_and_hammer_rows(unsigned int puf_base_address,unsigned int no_PUF_rows,unsigned int puf_init_value,unsigned int no_hammer_rows,unsigned int hammer_init_value,unsigned int pair_alternate_flag){

	unsigned int current_row=0;
	unsigned int puf_address=0;
	unsigned int hammer_address=0;

	//Setting base address for PUF section @ ROW 1
	puf_address=puf_base_address+same_bank_row_size;
	//Setting base address for Hammer section @ ROW 0
	hammer_address=puf_base_address;

	printk(KERN_ALERT "[i] Initialiting PUF & hammer rows\n");

	//Set Hammer rows
	switch(pair_alternate_flag){
		case 0:
		{
			printk(KERN_ALERT "[i] Single-Sided Rowhammer (SSRH)\n");
			//address_decode(puf_address,0); //Decode ROW and COL address form system address
			if(no_PUF_rows==1){
				write_row(puf_address,puf_init_value);
				//address_decode(hammer_address,0); //Decode ROW and COL address form system address
				write_row(hammer_address,hammer_init_value);
			}else{
				for(current_row=0;current_row<no_PUF_rows/2;current_row++){
					//address_decode(puf_address,0);  //Decode ROW and COL address form system address
					write_row(puf_address,puf_init_value);
					puf_address=puf_address+same_bank_row_size;
					//address_decode(puf_address,0);  //Decode ROW and COL address form system address
					write_row(puf_address,puf_init_value);
					puf_address=puf_address+(2*same_bank_row_size);
				}
				for(current_row=0;current_row<(no_hammer_rows/2)+1;current_row++){
					//address_decode(hammer_address,0); //Decode ROW and COL address form system address
					write_row(hammer_address,hammer_init_value);
					hammer_address=hammer_address+(3*same_bank_row_size);
				}
			}
			break;
		}
		default:
		{
			printk(KERN_ALERT "[i] Double-Sided Rowhammer (DSRH)\n");
			for(current_row=0;current_row<no_PUF_rows*2;current_row++){
				if(current_row%2!=0){
				//address_decode(puf_address,0); //Decode ROW and COL address form system address
				write_row(puf_address,puf_init_value);
				puf_address=puf_base_address+((current_row+2)*same_bank_row_size);
				}
			}
			for(current_row=0;current_row<=no_hammer_rows*2;current_row++){
				if(current_row%2==0){
					//address_decode(hammer_address,0); //Decode ROW and COL address form system address
					write_row(hammer_address,hammer_init_value);
					hammer_address=puf_base_address+((current_row+2)*same_bank_row_size);
				}
			}
			break;
		}

	}
	printk(KERN_ALERT "[i] Finished initialiting PUF & hammer rows\n");

	return;
}


/*  The below function invalidates cache memory for a range set which shall 
 *  be hammered for Rowhammering of PUF.  
 * 
 */

void configure_cache(unsigned int hammer_address, unsigned int no_hammer_rows,unsigned int pair_alternate_flag){
	
	unsigned int  rows, current_row;
	unsigned long mem_oper, mem_start, mem_len;
	void *write_virtaddr;
	//unsigned int written_value;
		
	if((no_hammer_rows/2) >= 64){
			rows = 64;
		}
		else{
			rows = no_hammer_rows;	
		}
		
			
	switch(pair_alternate_flag){
		case 0:
		{
			if(no_hammer_rows==1){
				
				mem_oper = 0x48291000 + (0x10 * 0);		// Write  value 0x00000001
				mem_start = 0x48291004 + (0x10 * 0);		// Write the start address for memory to be invalidated, PUF Read address
				mem_len = 0x48291008 + (0x10 * 0);		//Write value 0x00000fff
				
				//hammer_address
				write_virtaddr = ioremap(mem_oper,sizeof(unsigned int));
				*((unsigned int*)write_virtaddr)=0x00000001;
				iounmap(write_virtaddr);
				
				write_virtaddr = ioremap(mem_start,sizeof(unsigned int));
				*((unsigned int*)write_virtaddr)=hammer_address;
				iounmap(write_virtaddr);
				
				write_virtaddr = ioremap(mem_start,sizeof(unsigned int));
				*((unsigned int*)write_virtaddr)=0x00000fff;		//Range of 4Kb
				iounmap(write_virtaddr);
			
							
				mem_oper = 0x48291000 + (0x10 * 1);		// Write  value 0x00000001
				mem_start = 0x48291004 + (0x10 * 1);		// Write the start address for memory to be invalidated, PUF Read address
				mem_len = 0x48291008 + (0x10 * 1);		//Write value 0x00000fff
				
				hammer_address=hammer_address+(1024*same_bank_row_size);
				
				write_virtaddr = ioremap(mem_oper,sizeof(unsigned int));
				*((unsigned int*)write_virtaddr)=0x00000001;
				iounmap(write_virtaddr);
				
				write_virtaddr = ioremap(mem_start,sizeof(unsigned int));
				*((unsigned int*)write_virtaddr)=hammer_address;
				iounmap(write_virtaddr);
				
				write_virtaddr = ioremap(mem_start,sizeof(unsigned int));
				*((unsigned int*)write_virtaddr)=0x00000fff;		//Range of 4Kb
				iounmap(write_virtaddr);
			
								
			}else{
				for(current_row=0;current_row<rows;current_row++){
					//hammer _address
					mem_oper = 0x48291000 + (0x10 * current_row);	// Write  value 0x00000001
					mem_start = 0x48291004 + (0x10 * current_row);		// Write the start address for memory to be invalidated, PUF Read address
					mem_len = 0x48291008 + (0x10 * current_row);		//Write value 0x00000fff
					
					write_virtaddr = ioremap(mem_oper,sizeof(unsigned int));
					*((unsigned int*)write_virtaddr)=0x00000001;
					iounmap(write_virtaddr);
					
					write_virtaddr = ioremap(mem_start,sizeof(unsigned int));
					*((unsigned int*)write_virtaddr)=hammer_address;
					iounmap(write_virtaddr);
					
					write_virtaddr = ioremap(mem_start,sizeof(unsigned int));
					*((unsigned int*)write_virtaddr)=0x00000fff;		//Range of 4Kb
					iounmap(write_virtaddr);
					
					hammer_address=hammer_address+(3*same_bank_row_size);
				}
			}
			break;
		}
		default:
		{
			for(current_row=0;current_row < rows;current_row++){
							//hammer _address
					mem_oper = 0x48291000 + (0x10 * current_row);		// Write  value 0x00000001
					mem_start = 0x48291004 + (0x10 * current_row);		// Write the start address for memory to be invalidated, PUF Read address
					mem_len = 0x48291008 + (0x10 * current_row);		//Write value 0x00000fff
					
					write_virtaddr = ioremap(mem_oper,sizeof(unsigned int));	//
					*((unsigned int*)write_virtaddr)=0x00000001;
					iounmap(write_virtaddr);
					
					write_virtaddr = ioremap(mem_start,sizeof(unsigned int));	//Set Address Range
					*((unsigned int*)write_virtaddr)=hammer_address;
					iounmap(write_virtaddr);
					
					write_virtaddr = ioremap(mem_start,sizeof(unsigned int));
					*((unsigned int*)write_virtaddr)=0x00000fff;		//Range of 4Kb
					iounmap(write_virtaddr);
										
					hammer_address=hammer_address+(2*same_bank_row_size);
				}
				break;
			}
	}
		
	}

/* The below function is used for checking whether the CMU unit of 
 * Cortex A9 Processor for Pandaboard has been configured to invalidate 
 * the Memory Range Set. Run the function: void configure_cache(...) To 
 * invalidate the address range in the cache. The parameter 
 * no_hammer_rows must be same as that of the cache configure 
 * function. If there is a mismatch in value, there might be 
 * segmentation fault while the module is loaded in the kernel
 * */


void check_cache_configure(unsigned int no_hammer_rows){
	//No need for pointer, remove it from code
	unsigned int rows, current_row, x, mem_chk;
	void *write_virtaddr;
	
	if((no_hammer_rows/2) >= 64){
			rows = 64;
		}
		else{
			rows = no_hammer_rows;	
		}
	
	for(current_row = rows; current_row > 0 ; current_row--){
	mem_chk = 0x4829100C + (0x10 * current_row);	// To Know the status of the cache ranges, Read register to val 0x06
	
	write_virtaddr = ioremap(mem_chk,sizeof(unsigned int));	//
	x=*((unsigned int*)write_virtaddr);
	iounmap(write_virtaddr);	
	if(x != 0x00000006){
			printk(KERN_ALERT "Cache not Set %d\n", mem_chk);
			
		}
	}
	
	}





void get_puf(unsigned int base_address_puf){
	//PUF code begin 

	unsigned int puf_init_value=0x0;			//PUF Init Value
	unsigned int hammer_number=0;	                //Number of hammers
	unsigned int measurment_loop=0;	                //Loop variable for measurements
	unsigned int no_of_measurements_per_sampledecay=20; //number of sample per sample decay
	unsigned int hammer_flag=0x1;                     //Hammer Flag.. Hammer Yes or No
	unsigned int no_PUF_rows=32;	                //No of Rows for PUF > 1Row has 1024 words total size:4KB:::
	unsigned int no_hammer_rows=32;	                //No of Rows for Hammer > 1Row has 1024 words total size:4KB::: e.g No of Hammer rows is 8
	unsigned int hammer_init_value=0x0;      	//Hammer Rows init Value
	int puf_row_select=0;
	int puf_init_select=0;
	int RH_init_select=0;
	unsigned int pair_or_alternate_flag=0x1;	// 0x1: ALT (DSRH), 0x0: PRH (SSRH)
  // The below is code for timing specifications
	unsigned int currentdecay=0;                      //current decay in running loop
  	unsigned int Sample_delay=60*HZ;                     //Measurement sample decay(s)
	unsigned int total_delay=120*HZ;                      //Total decay time(s)
	unsigned long current_timer_value=0;              //current Timer value in msec,Reference to get timer value from this point
	//unsigned long relative_decay_time=0x0;             //Decay time relative to starting of application
	unsigned long j0,j1;	
	//int delay = puf_delaysec*HZ;

	int cache_flag=1;
	//int pair_or_alternate_flag = 0;

	while(cache_flag<2){
		switch(cache_flag){
			case 0 : printk(KERN_ALERT "\t[!]Cache will be ebabled!\n"); break;
			case 1 : printk(KERN_ALERT "\t[!]Cache will be disabled!\n");break;
			default: printk(KERN_ALERT "\t[!]cache flag invalidate!\n");break;
		}

		while(pair_or_alternate_flag<2){

			
	
			while(puf_init_select<	3){ //begin multiple test with multiple ending loop

		//Set rows
/*
		switch(puf_row_select){
	        case 0: no_hammer_rows=8;no_PUF_rows=8; break;
	        case 1: no_hammer_rows=32;no_PUF_rows=32; break;
	        case 2: no_hammer_rows=128;no_PUF_rows=128; break;
	        default:
	        	no_hammer_rows=8;
    	    		no_PUF_rows=8;
        		puf_row_select=0;
	        	RH_init_select++;
        		break;
    		}
*/
	    	//Set Hammer row IV
				switch(RH_init_select){
					case 0: hammer_init_value=0x0; break;
					case 1: hammer_init_value=0x55555555; break;
					case 2: hammer_init_value=0xaaaaaaaa; break;
					case 3: hammer_init_value=0xffffffff; break;
					default:
						hammer_init_value=0x0;
						RH_init_select=0;
						puf_init_select++;
						break;
    				}

    		//Set PUF row IV
				switch(puf_init_select){
        				case 0: puf_init_value=0x0; break;
        				case 1: puf_init_value=0xaaaaaaaa; break;
	        			case 2: puf_init_value=0xffffffff; break;
        				default:puf_init_value=0x0; break;
    				}
				if(puf_init_select<3){
	
    				printk(KERN_ALERT "[i] Starting the Rowhammer PUF for PandaBoard\n");

				printk(KERN_ALERT "Number of PUF rows: %d\n PUF init Value :%08x\n Rowhammer rows init Value:%08x\n",no_PUF_rows,puf_init_value,hammer_init_value);
				


				switch(pair_or_alternate_flag){
					case 0 : printk(KERN_ALERT "\t[!]Rowhammer Mode: SSRH\n"); break;
					case 1 : printk(KERN_ALERT "\t[!]Rowhammer Mode: DSRH\n"); break;
					default: printk(KERN_ALERT "\t[!]pair_or_alternate_flag invalidate!\n");break;
				}

		// Iterating the individual measurements
				for(measurment_loop=0;measurment_loop<no_of_measurements_per_sampledecay;measurment_loop++){
				printk(KERN_ALERT "[i] Start measurement: %d\n",measurment_loop);

			// Iterating the invidivual decay times
					for(currentdecay=Sample_delay;currentdecay<=total_delay;currentdecay+=Sample_delay){
						//printk(KERN_ALERT "Only Hammering",currentdecay/HZ);
						disable_refresh();
						Init_puf_and_hammer_rows(base_address_puf,no_PUF_rows,puf_init_value,no_PUF_rows,hammer_init_value,pair_or_alternate_flag);
					//Configure the Cache to invalidate the cache lines
						if(cache_flag){
							configure_cache(base_address_puf, no_PUF_rows,pair_or_alternate_flag);
							printk(KERN_ALERT "\n[!]Cache disabled!\n");
						}else{
							printk(KERN_ALERT "\n[!]Cache enabled!\n");
						}
						printk(KERN_ALERT "[i]\tTimer elapsed since its reset %lu sec\n",current_timer_value/HZ);
						printk(KERN_ALERT "[i] Decay: %d sec\n",currentdecay/HZ);
				
						//printk(KERN_ALERT "[i]\tRelative Decay: %lu msec\n",relative_decay_time);
						j0 = jiffies;
  						j1 = j0 + currentdecay;
						while (time_before(jiffies, j1)){ 
						//Rowhammer Code here
							if(hammer_flag==1){
								hammering_rows(base_address_puf,no_hammer_rows,pair_or_alternate_flag);
								hammer_number++;
        							schedule();
				
							}
				
						}
				
						printk(KERN_ALERT "[i] Total hammer attempts per row: %d\n",hammer_number);
						enable_refresh();
						printk(KERN_ALERT "[i] Starting PUF read-out\n");
						Read_puf(base_address_puf,no_PUF_rows,pair_or_alternate_flag, puf_init_value);
						printk(KERN_ALERT "[i] PUF reading end\n");

						printk(KERN_ALERT "[i] Finished-PUF query for decaytime: %d\n",currentdecay);
						hammer_number=0;
					}
					printk(KERN_ALERT "End measurement:%d\n",measurment_loop);
				}
				}
				RH_init_select++;
			}
			pair_or_alternate_flag++; puf_row_select=0; puf_init_select=0; RH_init_select=0;
		}
		cache_flag++; pair_or_alternate_flag=0; puf_row_select=0; puf_init_select=0; RH_init_select=0;
	}
	return;
}
//End get_puf function

// The below function runs only once
void get_puf_once(unsigned int base_address_puf){
	//PUF code begin 

	
	unsigned int hammer_number=0;	                //Number of hammers
	unsigned int measurment_loop=0;	                //Loop variable for measurements
	unsigned int no_of_measurements_per_sampledecay=1; //number of sample per sample decay
	unsigned int hammer_flag=0x1;                     //Hammer Flag.. Hammer Yes or No

	unsigned long pair_or_alternate_flag=0x1;	// 0x1: ALT (DSRH), 0x0: PRH (SSRH)
  // The below is code for timing specifications
	unsigned int currentdecay=0;                      //current decay in running loop
  	unsigned int Sample_delay=2*puf_delaysec*HZ;                     //Measurement sample decay(s)
	unsigned int total_delay=2*puf_delaysec*HZ;                      //Total decay time(s)
	unsigned long current_timer_value=0;              //current Timer value in msec,Reference to get timer value from this point
	//unsigned long relative_decay_time=0x0;             //Decay time relative to starting of application
	unsigned long j0,j1,j2;	
	
	
	printk(KERN_ALERT "[i] Starting the Rowhammer PUF for PandaBoard\n");

	printk(KERN_ALERT "Number of PUF rows: %d PUF init Value :%x Rowhammer rows init Value:%x\n",no_PUF_rows,puf_init_val,hammer_init_value);
		

		// Iterating the individual measurements
		for(measurment_loop=0;measurment_loop<no_of_measurements_per_sampledecay;measurment_loop++){
			//printk(KERN_ALERT "[i] Start measurement: %d\n",measurment_loop);

			// Iterating the invidivual decay times
			for(currentdecay=Sample_delay;currentdecay<=total_delay;currentdecay+=Sample_delay){
				printk(KERN_ALERT "[i] Start decaytime: %d\n",currentdecay/HZ);
				disable_refresh();
				Init_puf_and_hammer_rows(base_address_puf,no_PUF_rows,puf_init_val,no_PUF_rows,hammer_init_value,pair_or_alternate_flag);
				configure_cache(base_address_puf, no_PUF_rows,pair_or_alternate_flag);
				//printk(KERN_ALERT "[i]\tTimer elapsed since its reset %lu sec\n",current_timer_value/HZ);
				printk(KERN_ALERT "[i] Decay: %d sec\n",currentdecay/HZ);
				
				//printk(KERN_ALERT "[i]\tRelative Decay: %lu msec\n",relative_decay_time);
				j0 = jiffies;
  				j1 = j0 + currentdecay;
				while (time_before(jiffies, j1)){ 
						//Rowhammer Code here
				if(hammer_flag==1){
						hammering_rows(base_address_puf,no_hammer_rows,pair_or_alternate_flag);
						hammer_number++;
        					schedule();
				
					}
				
				}
				
				printk(KERN_ALERT "[i] Total hammer attempts per row: %d\n",hammer_number);
				enable_refresh();
				//printk(KERN_ALERT "[i] Starting PUF read-out\n");
				//j2 = jiffies;
				//while(time_before(jiffies,j2));
				Read_puf(base_address_puf,no_PUF_rows,pair_or_alternate_flag, puf_init_val);
				//printk(KERN_ALERT "[i] PUF reading end\n");

				printk(KERN_ALERT "[i] Finished-PUF query for decaytime: %d\n",currentdecay);
				hammer_number=0;
			}
			//printk(KERN_ALERT "End measurement:%d\n",measurment_loop);
		}
		
	
	return;
}


/*
The below function takes care of timing so as to run the program accordingly
*/


/*In the call to kthread_create we have passed the following arguments 

  thread_fn : Which is the function that will be run as the thread. 

  NULL: As we are not passing any data to the function we have kept this NULL. 

  name: The process will be named "thread1" in the list of processes . 
 
 */

int thread_fn (void *data) {

  //printk(KERN_ALERT "Pandaboard PUF Kernel Module inserted\n");
  //printk(KERN_ALERT "Disabling Cache\n");
  if(!strcmp(mystring, hammerall))
	get_puf(puf_base_address);
  else
	get_puf_once(puf_base_address);

  //printk(KERN_ALERT "Rowhammering Completed");
  puf_complete_flag = 1;		
return 0;
}


int thread_init (void) {
   
    char our_thread[18]="rowhammer_threadx";
    printk(KERN_NOTICE "Module for PUF is loaded");   
    printk(KERN_ALERT "in init");

    unsigned int sdram_addr = bcm_host_get_sdram_address();
    
    printk(KERN_NOTICE "SDRAM Address at 0x%x", sdram_addr);

    read_OMAP_system_address(sdram_addr, 0x00000000); 

    /*thread1 = kthread_create(thread_fn,puf_delaysec,our_thread);
    if((thread1))
        {
        printk(KERN_ALERT "in if");
        wake_up_process(thread1);
        }
*/
    return 0;
}

void thread_cleanup(void) {
  int ret=1;	
  if(!puf_complete_flag){
	ret = kthread_stop(thread1);
	if(!ret)
  	printk(KERN_ALERT "Thread stopped");
  }
  printk(KERN_ALERT "Module Removed\n");
 
 return;
}

MODULE_LICENSE("GPL");   
module_init(thread_init);
module_exit(thread_cleanup);

MODULE_AUTHOR("PANDABOARD Rowhammer-BASED DRAM PUF");
MODULE_DESCRIPTION("Pandaboard Rowhammer-based DRAM PUF kernel module for changing DRAM refresh rate and reading/writing tp PUF memory locations");

