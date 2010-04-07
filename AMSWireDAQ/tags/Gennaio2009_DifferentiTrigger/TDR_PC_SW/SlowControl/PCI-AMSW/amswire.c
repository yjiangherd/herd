/* -*-C-*-
 * amswire.c - AMSWire PCI Board device driver for Linux Kernel 2.4 or above.
 *
 *-----------------------------------------------------------------------
 *
 * Project          : Linux
 * Moduletyp        : C-Code
 * Author           : Xudong CAI
 * Created          : 05 Dec. 2001
 *
 * modified by AL Nov-2006...
 *
 *-----------------------------------------------------------------------
 */


/************************************************************************/
/* Includes                                                             */
/************************************************************************/

#include <linux/module.h>
#include <linux/init.h>
 
#include <asm/segment.h>
#include <asm/system.h>
#include <asm/irq.h>
 
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/page.h>
#include <asm/mman.h>
#include <asm/bitops.h>
#include <asm/param.h>
#include <asm/semaphore.h>
 
#include <linux/version.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/wrapper.h>
#include <linux/byteorder/swab.h>
 
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/sys.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>

#include "amsw_dev.h"
#include "amsw_lib.h"

/************************************************************************
	Defines
 ************************************************************************/
#define AMS_VENDOR_ID			0x414D
#define AMSW_DEVICE_ID			0x5304
#define AMSWIRE_SUB_VENDOR_ID	0x0000
#define AMSWIRE_SUBSYSTEM_ID	0x0000

#define AMSW_MAJOR_NUMBER		241

#define amsw_bit_set(x, n)			(x & (1 << n))
#define amsw_bit_clear(x, n)			!(x & (1 << n))
#define amsw_set_bit(x, n)			x |= (1 << n)
#define amsw_clear_bit(x, n)			x &= ~(1 << n)

#define AMSW_USE_POLL

/************************************************************************
	Typedefs / Structs
 ************************************************************************/

typedef struct __mem_desc {
	u_long 				va;			/* kernel virtual address */
	u_long				ua;			/* user space virtual address */
	int					size;
	struct __mem_desc	*next;
} AMSW_MDESC;

typedef struct {
	/* device info */
	struct pci_dev	*pdev;			/* Device information */
	u_char   		irq;			/* IRQ number */

	/* driver parameters */
	int				inuse;			/* Device inuse flag */
	int				irq_ena;		/* Interrupt enabled flag */
	int				irq_mask;		/* Interrupt mask */
	volatile u_long	irq_status;		/* IRQ status */

	int				dma_wr;			/* Write DMA enabled flag */
	int				dma_rd;			/* Read DMA enabled flag */
	volatile int	dma_done;		/* DMA done flag */

	/* BAR information */
	u_long			pBar0;			/* BAR0 physical address */
	u_long			vBar0;			/* BAR0 virtual address */
	int				size0;			/* size of BAR0 */

	u_long			pBar1;			/* BAR1 physical address */
	u_long			vBar1;			/* BAR1 virtual address */
	int				size1;			/* size of BAR1 */

	u_long			pBar2;			/* BAR2 physical address */
	u_long			vBar2;			/* BAR2 virtual address */
	int				size2;			/* size of BAR1 */

	/* firmware info */
	u_short			a_ver;			/* Actel version */
	u_long			x_ver;			/* Xilinx version */

	/* IRQ counters */
	u_long			irq_cnt;
	u_long			irq_err_cnt;
	u_long			irq_fail_cnt;
	u_long			irq_rx_cnt;
	u_long			irq_dma_cnt;

	/* DMA counters */
	u_long			dma_cnt;
	u_long			dma_err_cnt;
	u_long			dma_rd_cnt;
	u_long			dma_wr_cnt;

	/* TX control */
	volatile int	tx_chan_stat;	/* TX channel hardware status */
	volatile int	tx_soft_stat;	/* TX channel software status */

	/* RX control */
	volatile int	rx_chan_stat;	/* RX channel status */
	volatile int	rx_overrun;		/* RX overrun status */
	volatile int	rx_error;		/* RX error status */
	volatile int	rx_size[8];		/* RX size for each channel */
	volatile u_long	rx_status[8];	/* RX status for each channel */

	/* TX counters */
	u_long			tx_cnt[8];			/* TX counter */
	u_long			tx_fail_cnt[8];	/* Fail to make TX */

	/* RX counters */
	u_long			rx_cnt[8];
	u_long			rx_fail_cnt[8];
	u_long			rx_err_cnt[8];
	u_long			rx_overrun_cnt[8];
} AMSW_DEV;

/************************************************************************
	Globals
 ************************************************************************/

char amswire_vers[] = "3.0";
char device_name[] = "/dev/amsw";
int dma_write = 0;
int dma_read  = 1;      // AL
//int dma_read  = 0;        // AL
int debug = 1;
int swapbyte = 1;

/************************************************************************
	Forward declarations
 ************************************************************************/

static int amsw_dev_open(struct inode *inode, struct file *file);
static int amsw_dev_close(struct inode *inode, struct file *file);
static ssize_t amsw_dev_read(struct file *file, char *buf, size_t nbytes, loff_t *ppos);
static ssize_t amsw_dev_write(struct file *file, const char *buf, size_t nbytes, loff_t *ppos);
static int amsw_dev_ioctl(struct inode *inode, struct file *file, u_int cmd, u_long param);
static void amsw_irq_handle(int irq, void *user_arg, struct pt_regs *regs);
static int amsw_probe(struct pci_dev *, const struct pci_device_id *);
static void amsw_remove(struct pci_dev *);
static int amsw_dev_mmap(struct file *file, struct vm_area_struct *vma);
void *amsw_dev_kmalloc(struct file * file, int size);
void amsw_dev_kfree(struct file *file, void *addr);
int amsw_direct_dma(AMSW_DEV *pd, int dir, u_long pci_offset, u_long buff, int size);
int amsw_indirect_dma(AMSW_DEV *pd, int dir, u_long pci_offset, void *buff, int size);
ssize_t amsw_dev_mem_read(AMSW_DEV *pd, u_long offset, void *buff, size_t nbytes);
ssize_t amsw_dev_mem_write(AMSW_DEV *pd, u_long offset, void *buff, size_t nbytes);
void memcpy_swap(void *dest, const void *src, size_t nbytes);
void mem_swap(void *data, size_t nbytes);
void amsw_dev_init(AMSW_DEV *pd);
void amsw_dev_irq_proc(u_long data);
int amsw_dev_tx(AMSW_DEV *pd, AMSW_DATA *d);
int amsw_dev_rx(AMSW_DEV *pd, AMSW_DATA *d);

/************************************************************************
	Local
 ************************************************************************/

static int ncards;
static AMSW_DEV amswdev[AMSW_MAX_NCARDS];

static struct file_operations amsw_dev_fops =
{
	open:			amsw_dev_open,
	release:		amsw_dev_close,
	read:			amsw_dev_read,
	write:			amsw_dev_write,
	ioctl:			amsw_dev_ioctl,
	mmap:			amsw_dev_mmap,
};

MODULE_PARM(dma_write, "i");
MODULE_PARM(dma_read, "i");
MODULE_PARM(debug, "i");
MODULE_PARM(swapbyte, "i");

MODULE_AUTHOR("Xudong CAI");
MODULE_DESCRIPTION("AMSW-PCI Device Driver");

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,10)
MODULE_LICENSE("GPL");
#endif

/************************************************************************
	Function:
   		memcpy_swap - Memory copy to PCI with byte swap (le32 to be32)

	INPUTS:
		dest ------ destination memory
		src ------- source memory
		nbytes ---- number of bytes

   	RETURNS:
 		0 on success
 ************************************************************************/
#define LETOBE32(x) (((x >> 24) & 0x000000FFU) | \
				   ((x << 24) & 0xFF000000U) | \
				   ((x >>  8) & 0x0000FF00U) | \
				   ((x <<  8) & 0x00FF0000U))

void memcpy_swap(void *dest, const void *src, size_t nbytes) {
	u_long *s = (u_long *) src;
	u_long *d = (u_long *) dest;
	int size;
	int i;

	/* preparations */
	size = (nbytes + 3) / 4;	/* using long word unit */

	/* Loop copy */
	for ( i = 0; i < size; i++ ) d[i] = LETOBE32(s[i]);
}

/************************************************************************
	Function:
   		mem_swap - byte swap with memory area (le32 to be32)

	INPUTS:
		data ------ memory address
		nbytes ---- number of bytes

   	RETURNS:
 		0 on success
 ************************************************************************/
void mem_swap(void *data, size_t nbytes) {
	u_long *d = (u_long *) data;
	int size;
	int i;

	/* preparations */
	size = (nbytes + 3) / 4;	/* using long word unit */

	/* Loop copy */
	for ( i = 0; i < size; i++ ) d[i] = LETOBE32(d[i]);
}

/************************************************************************
	Function:
		amsw_dev_init - Initialize AMSWire device and driver parameters

	RETURNS:
		none
 ************************************************************************/
void amsw_dev_init(AMSW_DEV *pd) {
	int i;

	/* Initialize driver info */
	pd->tx_chan_stat   = 0xff;			/* 1 - ready, 0 - in use */
	pd->tx_soft_stat   = 0xff;			/* 1 - ready, 0 - in use */

	pd->rx_chan_stat   = 0;			    /* 1 - RX done, 0 - no RX */
	pd->rx_overrun     = 0;				/* 1 - overrun, 0 - no overrun */
	pd->rx_error       = 0;				/* 1 - error,   0 - no error */

	pd->irq_cnt        = 0;
	pd->irq_err_cnt    = 0;
	pd->irq_fail_cnt   = 0;
	pd->irq_rx_cnt	   = 0;
	pd->irq_dma_cnt    = 0;

	pd->dma_cnt        = 0;
	pd->dma_err_cnt    = 0;
	pd->dma_rd_cnt     = 0;
	pd->dma_wr_cnt     = 0;

	for ( i = 0; i < 8; i++ ) {
		pd->tx_cnt[i]         = 0;
		pd->tx_fail_cnt[i]    = 0;

		pd->rx_cnt[i]         = 0;
		pd->rx_fail_cnt[i]    = 0;
		pd->rx_err_cnt[i]     = 0;
		pd->rx_overrun_cnt[i] = 0;
	}
}

/************************************************************************
	Function:
		amsw_dev_irq_proc - Process IRQ

	RETURNS:
		none
 ************************************************************************/
void amsw_dev_irq_proc(u_long data) {
	AMSW_DEV *pd = (AMSW_DEV *) data;
	u_long *reg = (u_long *) pd->vBar1;
	int i;

	/* read IRQ status */
	pd->irq_status = reg[AMSW_CSR_INT];

	/* check if status is valid */
	if ( (pd->irq_status & 0xfe000000) ) {
		int ii;

		pd->irq_err_cnt++;
		for ( ii = 0; ii < 10; ii++ ) {
			pd->irq_status = reg[AMSW_CSR_INT];
			if ( !(pd->irq_status & 0xfe000000) ) break;
		}
		if ( ii == 10 ) {
			pd->irq_fail_cnt++;
			return;
		}
	}

	/* Check RX done status */
	for ( i = 0; i < 8; i++ ) {
		if ( amsw_bit_set(pd->irq_status, i) ) {
			if ( amsw_bit_clear(pd->rx_chan_stat, i) ) {
				pd->irq_rx_cnt++;
				amsw_set_bit(pd->rx_chan_stat, i);

				/* Check error status */
				pd->rx_status[i] = reg[i * 2 + 1];
				if ( pd->rx_status[i] & AMSW_RX_ERR_MASK ) {
					pd->rx_err_cnt[i]++;
					amsw_set_bit(pd->rx_error, i);		/* nothing to be done when error occurs */
					if ( debug > 0 ) printk("AMSWIRE: RX error, RX channel = %d, IRQ status = %08lx, "
											"RX status = %08lx, TX status = %08lx\n",
											i, pd->irq_status, pd->rx_status[i], reg[i * 2]);
					pd->rx_size[i] = 4;		/* minimum size, no data will be read */
					continue;
				}
				/* Check overrun */
				else if ( pd->rx_status[i] & 0x70 ) {
					pd->rx_overrun_cnt[i]++;
					amsw_set_bit(pd->rx_overrun, i);
					if ( debug > 0 ) printk("AMSWIRE: RX overrun (new IRQ), RX channel = %d, IRQ status = %08lx, "
											"RX status = %08lx, TX status = %08lx\n",
											i, pd->irq_status, pd->rx_status[i], reg[i * 2]);
				}

				/* Set size */
				pd->rx_size[i] = (pd->rx_status[i] & AMSW_RX_SIZE_MASK) >> AMSW_RX_SIZE_SHFT;
			}
			else {
				/* set overrun */
				pd->rx_overrun_cnt[i]++;
				amsw_set_bit(pd->rx_overrun, i);
				pd->rx_status[i] = reg[i * 2 + 1];
				if ( debug > 0 ) printk("AMSWIRE: RX overrun (more IRQ), RX channel = %d, IRQ status = %08lx, "
										"RX status = %08lx, TX status = %08lx\n",
										i, pd->irq_status, pd->rx_status[i], reg[i * 2]);

				/* set new size */
				pd->rx_size[i] = (pd->rx_status[i] & AMSW_RX_SIZE_MASK) >> AMSW_RX_SIZE_SHFT;
			}
		}
	}

	/* Check TX done status */
	for ( i = 0; i < 8; i++ )
		if ( amsw_bit_set(pd->irq_status, (i + 16)) ) amsw_set_bit(pd->tx_chan_stat, i);

	/* Check DMA done */
	if ( (pd->irq_status & AMSW_INT_DMADONE) ) {
		pd->irq_dma_cnt++;
		pd->dma_done = 1;
	}

	return;
}

/************************************************************************
	Function:
		amsw_dev_tx - send TX data

	RETURNS:
		0 on success
 ************************************************************************/
int amsw_dev_tx(AMSW_DEV *pd, AMSW_DATA *d) {
	u_long *reg = (u_long *) pd->vBar1;
	int ret;

	/* Check channel validate */
	if ( d->chan < 0 || d->chan > 7 ) return -EFAULT;

	/* Check channel status */
	if ( amsw_bit_clear((pd->tx_chan_stat & pd->tx_soft_stat), d->chan) ) return -EBUSY;
	amsw_clear_bit(pd->tx_soft_stat, d->chan);
	pd->tx_cnt[d->chan]++;

	/* Write TX data to memory */
	if ( (ret = amsw_dev_mem_write(pd, (d->chan << AMSW_BUFF_SHIFT), (void *) d->data, d->size)) < 0 ) {
		pd->tx_fail_cnt[d->chan]++;
		if ( debug > 0 ) printk("AMSWIRE: Fail to write data to TX buffer %d\n", d->chan);
		amsw_set_bit(pd->tx_soft_stat, d->chan);
		return ret;
	}

	/* Start transmit */
	amsw_clear_bit(pd->tx_chan_stat, d->chan);
	amsw_set_bit(pd->tx_soft_stat, d->chan);
	reg[d->chan << 1] = d->csr | AMSW_TX_START;

	return 0;
}

/************************************************************************
	Function:
		amsw_dev_rx - get RX data

	RETURNS:
		0 on success
 ************************************************************************/
int amsw_dev_rx(AMSW_DEV *pd, AMSW_DATA *d) {
	u_long *reg = (u_long *) pd->vBar1;
	int ret;

	/* Check channel */
	if ( d->chan < 0 || d->chan > 7 ) return -EFAULT;

	/* Check RX status */
	if ( amsw_bit_clear(pd->rx_chan_stat, d->chan) ) return -ENODATA;
	amsw_clear_bit(pd->rx_chan_stat, d->chan);
	pd->rx_cnt[d->chan]++;

	/* Read current RX status */
	pd->rx_status[d->chan] = reg[(d->chan << 1) + 1];

	/* Check RX error */
	if ( amsw_bit_set(pd->rx_error, d->chan) ) {
		d->status = 2;
		d->csr = pd->rx_status[d->chan];
		amsw_clear_bit(pd->rx_error, d->chan);
		return 0;
	}

	/* Read RX data */
	if ( d->size > 0 ) {
		if ( (ret = amsw_dev_mem_read(pd, ((d->chan << AMSW_BUFF_SHIFT) + AMSW_BUFF_OFFSET), (void *) d->data, d->size)) < 0 ) {
			pd->rx_fail_cnt[d->chan]++;
			if ( debug > 0 ) printk("AMSWIRE: Fail to read data from RX buffer %d\n", d->chan);
			return ret;
		}
	}

	/* Check again RX status for overrun */
	pd->rx_status[d->chan] |= reg[(d->chan << 1) + 1];
	if ( (pd->rx_status[d->chan] & 0x70) || amsw_bit_set(pd->rx_chan_stat, d->chan) ) {
		amsw_clear_bit(pd->rx_chan_stat, d->chan);
		amsw_set_bit(pd->rx_overrun, d->chan);
		if ( debug > 0 ) printk("AMSWIRE: RX overrun (RX), channel = %d, IRQ status = %08lx, "
								"RX status = %08lx, TX status = %08lx\n",
								d->chan, pd->irq_status, pd->rx_status[d->chan], reg[d->chan << 1]);
	}

	/* Check overrun */
	if ( amsw_bit_set(pd->rx_overrun, d->chan) ) {
		amsw_clear_bit(pd->rx_overrun, d->chan);
		d->status = 1;
	}
	else d->status = 0;

	/* Set CSR */
	d->csr = pd->rx_status[d->chan];

	return 0;
}

/************************************************************************
	Function:
		amsw_dev_open - Open AMSWire device

	RETURNS:
		0 on success
 ************************************************************************/
static int amsw_dev_open(struct inode *inode, struct file *file) {
	AMSW_DEV *pd;

	/* Check minor device ID */
	if ( MINOR(inode->i_rdev) >= ncards ) {
		printk("AMSWIRE: open non-exist device, minor ID = %d\n", MINOR(inode->i_rdev));
		return -EINVAL;
	}

	/* Pass device pointer to file structure */
	pd = &amswdev[MINOR(inode->i_rdev)];
	file->private_data = (void *) pd;
	file->f_pos = 0;

	/* Increase module in use count */
	MOD_INC_USE_COUNT;

	return 0;
}

/************************************************************************
	Function:
		amsw_dev_close - Close AMSWire device

	RETURNS:
		0 on success
 ************************************************************************/
static int amsw_dev_close(struct inode *inode, struct file *file) {
	AMSW_MDESC *mp, **mpp;

	/* Pass device pointer to file structure */
	mpp = (AMSW_MDESC **) &(file->f_pos);

	/* Free all buffers */
	if ( ((int) *mpp) ) {
		/* Find address */
		for ( mp = *mpp; mp; mp = mp->next ) {
			mpp = &(mp->next);
			amsw_dev_kfree(file, (void *) mp->ua);
		}
	}

	/* Clear module in use count */
	MOD_DEC_USE_COUNT;

	return 0;
}

/************************************************************************
	Function:
		amsw_direct_dma - Direct DMA R/W between device and user buffer

	INPUTS:
		pd ------------ Device descriptor
		dir ----------- DMA transfer direction (AMSW_DMA_RD or AMSW_DMA_WR)
		pci_offset ---- PCI offset
		buff ---------- Buffer physical address
		size ---------- Number of bytes to be transferred

	RETURNS:
		= 0 on success
		< 0 error code
 ************************************************************************/
int amsw_direct_dma(AMSW_DEV *pd, int dir, u_long pci_offset, u_long buff, int size) {
	int i;
	u_int dma_ctrl;
	u_short pci_stat;
	u_long offset;
	u_long addr;
	int blk_len;
	int dma = (int) pd->vBar2;

	/* Set initial offset */
	offset = pci_offset;

	/* Debug info */
	if ( debug > 1 ) printk("AMSWIRE: Direct DMA %s, addr = %08lx, offset = %08lx, size = %d\n",
						(dir == AMSW_DMA_RD ? "read" : "write"), (u_long) buff, pci_offset, size);

	pd->dma_cnt++;
	if ( dir == AMSW_DMA_RD ) pd->dma_rd_cnt++;
	else pd->dma_wr_cnt++;

	/* Start loop for 4K limit of DMA transfer */
	for ( addr = buff; size > 0; addr += AMSW_DMA_SIZE, size -= AMSW_DMA_SIZE, offset += AMSW_DMA_SIZE ) {
		/* Calculate block length */
		blk_len = size > AMSW_DMA_SIZE ? AMSW_DMA_SIZE : size;

		/* Prepare DMA control word */
		dma_ctrl = dir | (blk_len == AMSW_DMA_SIZE ? 0 : blk_len << 16);

		/* Write physical address (seen from PCI) */
		outl(addr, dma + AMSW_DMA_PADDR);

		/* Write device local address offset */
		outl(offset, dma + AMSW_DMA_LADDR);

		/* Write DMA control */
		if ( pd->dma_done ) {
			if ( debug > 0 ) printk("AMSWIRE: unexpected DMA done interrupt\n");
			pd->dma_done = 0;
		}
		outl(dma_ctrl, dma + AMSW_DMA_CTRL);

		/* Wait for DMA done */
		for ( i = 0; i < AMSW_DMA_TIMEOUT; i++ ) {
			if ( pd->dma_done ) {
				pd->dma_done = 0;
				dma_ctrl = inl(dma + AMSW_DMA_CTRL);
				if ( dma_ctrl & AMSW_DMA_ERROR ) {
					pci_read_config_word(pd->pdev, PCI_STATUS, &pci_stat);
					printk("AMSWIRE: DMA (%s) error, INT Status = %08lx, DMA status = %08x, PCI status = %04x\n",
							(dir == AMSW_DMA_RD ? "read" : "write"), pd->irq_status, dma_ctrl, pci_stat);
					pd->dma_err_cnt++;
					return -EIO;
				}
				break;
			}
		}

		/* Check Timeout */
		if ( i == AMSW_DMA_TIMEOUT ) {
			dma_ctrl = inl(dma + AMSW_DMA_CTRL);
			pci_read_config_word(pd->pdev, PCI_STATUS, &pci_stat);
			printk("AMSWIRE: DMA (%s) timeout %d, IRQ Status = %08lx, DMA status = %08x, PCI status = %04x\n",
				(dir == AMSW_DMA_RD ? "read" : "write"), i, pd->irq_status, dma_ctrl, pci_stat);
			pd->dma_err_cnt++;
			return -ETIMEDOUT;
		}
	}
	return 0;
}

/************************************************************************
	Function:
		amsw_indirect_dma - Indirect DMA R/W between device and user buffer

	INPUTS:
		pd ------------ Device descriptor
		dir ----------- DMA transfer direction (AMSW_DMA_RD or AMSW_DMA_WR)
		pci_offset ---- PCI offset
		buff ---------- Buffer address
		size ---------- Number of bytes to be transferred

	RETURNS:
		= 0 on success
		< 0 error code
 ************************************************************************/
int amsw_indirect_dma(AMSW_DEV *pd, int dir, u_long pci_offset, void *buff, int size) {
	void *va;
	u_long pAddr;
	int i;
	u_int dma_ctrl;
	u_short pci_stat;
	u_long offset;
	u_long addr;
	int blk_len;
	int dma = (int) pd->vBar2;
	int ret = 0;

	pd->dma_cnt++;
	if ( dir == AMSW_DMA_RD ) pd->dma_rd_cnt++;
	else pd->dma_wr_cnt++;

	/* Allocate memory for DMA */
	if ( !(va = (void *) __get_dma_pages(GFP_KERNEL, ((AMSW_DMA_SIZE + PAGE_SIZE - 1) /  PAGE_SIZE))) ) {
		printk("AMSWIRE: Fail to allocate memory for DMA\n");
		pd->dma_err_cnt++;
		return -EFAULT;
	}

	/* Get physical address */
	pAddr = (u_long) __pa(va);

	/* Set initial offset */
	offset = pci_offset;

	if ( debug > 1 ) printk("AMSWIRE: Indirect DMA %s, addr = %08lx, offset = %08lx, size = %d, kernel buffer = %08lx\n",
						(dir == AMSW_DMA_RD ? "read" : "write"), (u_long) buff, pci_offset, size, pAddr);

	for ( addr = (u_long) buff; size > 0; addr += AMSW_DMA_SIZE, size -= AMSW_DMA_SIZE, offset += AMSW_DMA_SIZE ) {
		/* Calculate block length */
		blk_len = size > AMSW_DMA_SIZE ? AMSW_DMA_SIZE : size;

		/* Memory copy (write only) */
		if ( dir == AMSW_DMA_WR ) {
			if ( swapbyte )
				memcpy_swap((char *) va, (char *) addr, blk_len);
			else
				memcpy((char *) va, (char *) addr, blk_len);
		}

		/* Prepare DMA control word */
		dma_ctrl = dir | (blk_len == AMSW_DMA_SIZE ? 0 : blk_len << 16);

		/* Write physical address (seen from PCI) */
		outl(pAddr, dma + AMSW_DMA_PADDR);

		/* Write device local address offset */
		outl(offset, dma + AMSW_DMA_LADDR);

		/* Write DMA control */
		if ( pd->dma_done ) {
			if ( debug > 0 ) printk("AMSWIRE: unexpected DMA done interrupt\n");
			pd->dma_done = 0;
		}
		outl(dma_ctrl, dma + AMSW_DMA_CTRL);

		/* Wait for DMA done */
		for ( i = 0; i < AMSW_DMA_TIMEOUT; i++ ) {
			if ( pd->dma_done ) {
				pd->dma_done = 0;
				dma_ctrl = inl(dma + AMSW_DMA_CTRL);
				if ( dma_ctrl & AMSW_DMA_ERROR ) {
					pci_read_config_word(pd->pdev, PCI_STATUS, &pci_stat);
					printk("AMSWIRE: DMA (%s) error, INT Status = %08lx, DMA status = %08x, PCI status = %04x\n",
							(dir == AMSW_DMA_RD ? "read" : "write"), pd->irq_status, dma_ctrl, pci_stat);
					ret = -EIO;
					pd->dma_err_cnt++;
					goto dma_end;
				}
				break;
			}
		}

		/* Check Timeout */
		if ( i == AMSW_DMA_TIMEOUT ) {
			dma_ctrl = inl(dma + AMSW_DMA_CTRL);
			pci_read_config_word(pd->pdev, PCI_STATUS, &pci_stat);
			printk("AMSWIRE: DMA (%s) timeout %d, IRQ Status = %08lx, DMA status = %08x, PCI status = %04x\n",
				(dir == AMSW_DMA_RD ? "read" : "write"), i, pd->irq_status, dma_ctrl, pci_stat);
			ret = -ETIMEDOUT;
			pd->dma_err_cnt++;
			goto dma_end;
		}

		/* Memory copy (read only) */
		if ( dir == AMSW_DMA_RD ) {
			if ( swapbyte )
				memcpy_swap((char *) addr, (char *) va, blk_len);
			else
				memcpy((char *) addr, (char *) va, blk_len);
		}
	}

dma_end:

	/* Free memory for DMA */
	free_pages((u_long) va, ((AMSW_DMA_SIZE + PAGE_SIZE - 1) /  PAGE_SIZE));
	return ret;
}

/************************************************************************
	Function:
		amsw_mem_read - Read from dual-port memory of AMSW-PCI card

	RETURNS:
		>= 0 - size of real operation
		 < 0 - error code
 ************************************************************************/
ssize_t amsw_dev_mem_read(AMSW_DEV *pd, u_long offset, void *buff, size_t nbytes) {
	int size;
	int status;
	struct vm_area_struct *vma;
	u_long addr = 0;
	int user_flag;

	/* Get size in long word */
	if ( nbytes <= 0 ) return 0;
	size = (nbytes + 3) / 4 * 4;

	/* Check address */
	vma = find_vma(current->mm, (int) buff);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,10)
	if ( vma->vm_flags & VM_ACCOUNT ) {
#else
	if ( !(vma->vm_flags & (VM_SHARED | VM_MAYSHARE)) ) {
#endif
		user_flag = 1;
	}
	else {
		addr = (u_long) (vma->vm_pgoff << PAGE_SHIFT) | ((u_long) buff - vma->vm_start);
		user_flag = 0;
	}

	/* Check DMA enable */
	if ( pd->dma_rd && nbytes > 32 ) {
		/* Make DMA transfer */
		if ( user_flag ) status = amsw_indirect_dma(pd, AMSW_DMA_RD, offset, buff, size);
		else {
			status = amsw_direct_dma(pd, AMSW_DMA_RD, offset, addr, size);
			if ( swapbyte ) mem_swap(__va(addr), nbytes);
		}

		/* Check status returned for DMA operation */
		if ( status < 0 ) return status;
	}
	else if (swapbyte)
		memcpy_swap(buff, (void *) (pd->vBar0 + offset), nbytes);
	else if ( user_flag )
		copy_to_user(buff, (void *) (pd->vBar0 + offset), nbytes);
	else
		memcpy(buff, (void *) (pd->vBar0 + offset), nbytes);

	return nbytes;
}

/************************************************************************
	Function:
		amsw_mem_write - Write to dual-port memory of AMSW-PCI card

	RETURNS:
		>= 0 - size of real operation
		 < 0 - error code
 ************************************************************************/
ssize_t amsw_dev_mem_write(AMSW_DEV *pd, u_long offset, void *buff, size_t nbytes) {
	int size;
	int status;
	struct vm_area_struct *vma;
	u_long addr = 0;
	int user_flag;

	/* Get size in long word */
	if ( nbytes <= 0 ) return 0;
	size = (nbytes + 3) / 4 * 4;

	/* Check address */
	vma = find_vma(current->mm, (int) buff);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,10)
	if ( vma->vm_flags & VM_ACCOUNT ) user_flag = 1;
#else
	if ( !(vma->vm_flags & (VM_SHARED | VM_MAYSHARE)) ) user_flag = 1;
#endif
	else {
		addr = (u_long) (vma->vm_pgoff << PAGE_SHIFT) | ((u_long) buff - vma->vm_start);
		user_flag = 0;
	}

	/* Check DMA enable */
	if ( pd->dma_wr && nbytes > 32 ) {
		/* Make DMA transfer */
		if ( user_flag || swapbyte ) status = amsw_indirect_dma(pd, AMSW_DMA_WR, offset, buff, size);
		else 						 status = amsw_direct_dma(pd, AMSW_DMA_WR, offset, addr, size);

		/* Check status returned for DMA operation */
		if ( status < 0 ) return status;
	}
	else if ( swapbyte )
		memcpy_swap((void *) (pd->vBar0 + offset), buff, nbytes);
	else if ( user_flag )
		copy_to_user((void *) (pd->vBar0 + offset), buff, nbytes);
	else
		memcpy((void *) (pd->vBar0 + offset), buff, nbytes);

	return nbytes;
}

/************************************************************************
	Function:
		amsw_dev_read - Read from BAR2 of AMSWire device

	RETURNS:
		0 on success
 ************************************************************************/
static ssize_t amsw_dev_read(struct file *file, char *buf, size_t nbytes, loff_t *ppos) {
	register AMSW_DEV *pd = (AMSW_DEV *) file->private_data;
	AMSW_DATA *d = (AMSW_DATA *) buf;
	int ret;

	/* Check in use flag */
	if ( pd->inuse ) return -EBUSY;
	pd->inuse = 1;

	/* Read memory */
	ret = amsw_dev_mem_read(pd, d->pci_off, (void *) d->data, d->size);

	/* Finished */
	pd->inuse = 0;
	return ret;
}

/************************************************************************
	Function:
		amsw_dev_write - Write to BAR2 of AMSWire device

	RETURNS:
		0 on success
 ************************************************************************/
static ssize_t amsw_dev_write(struct file *file, const char *buf, size_t nbytes, loff_t *ppos) {
	register AMSW_DEV *pd = (AMSW_DEV *) file->private_data;
	AMSW_DATA *d = (AMSW_DATA *) buf;
	int ret;

	/* Check in use flag */
	if ( pd->inuse ) return -EBUSY;
	pd->inuse = 1;

	/* Write memory */
	ret = amsw_dev_mem_write(pd, d->pci_off, (void *) d->data, d->size);

	/* Finished */
	pd->inuse = 0;
	return ret;
}

/************************************************************************
	Function:
		amsw_dev_mmap - remap memory

	RETURNS:
		0 on success
 ************************************************************************/
static int amsw_dev_mmap(struct file *file, struct vm_area_struct *vma) {
	AMSW_DEV *pd = (AMSW_DEV *) file->private_data;
	u_long offset;
	u_long addr;
	u_long vsize;

	/* Get offset */
	offset = vma->vm_pgoff << PAGE_SHIFT;
	vsize = vma->vm_end - vma->vm_start;

	/* Check offset */
	if ( offset == 0xba000000 ) {
		addr = pd->pBar0;
		vsize = pd->size0;
	}
	else if ( offset == 0xba100000 ) {
		addr = pd->pBar1;
		vsize = PAGE_SIZE;
	}
	else addr = offset;

	/* remap range */
	if ( remap_page_range(
#if ((LINUX_VERSION_CODE > KERNEL_VERSION(2,4,19)) && \
    (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,22))) || \
    (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0))
 			      vma,
#endif
			      vma->vm_start, addr, vsize, vma->vm_page_prot) )
		return -EAGAIN;


	return 0;
}

/************************************************************************
	Function:
		amsw_dev_kmalloc - Allocate kernel memory for user space

	RETURNS:
		0 on success
 ************************************************************************/
void * amsw_dev_kmalloc(struct file *file, int size) {
	AMSW_MDESC *mp_old, *mp_new;
	u_long ua, va;

	/* Allocate kernel memory for descriptor */
	mp_new = (AMSW_MDESC *) kmalloc(sizeof(AMSW_MDESC), GFP_KERNEL);
	if ( !mp_new ) {
		printk("AMSWIRE: can't allocate kernel memory for AMSW_MDESC\n");
		return 0;
	}

	/* why ? */
	if ( size <= PAGE_SIZE / 2 ) size = PAGE_SIZE - 0x10;
	if ( size > 0x1FFF0 ) {
		printk("AMSWIRE: can't allocate so big kernel memory for user, size = %d\n", size);
		return 0;
	}

	/* Allocate memory for user */
	va = (u_long) kmalloc(size, __GFP_DMA | __GFP_HIGH | __GFP_WAIT );
	if ( !va ) {
		printk("AMSWIRE: can't allocate %d bytes of kernel memory for user\n", size);
		kfree(mp_new);
		return 0;
	}

	/* Reserve memory area in mem_map table */
	for ( ua = va & PAGE_MASK; ua < va + size; ua += PAGE_SIZE )
		mem_map_reserve(virt_to_page(ua));

	/* make memory map */
	ua = do_mmap(file,						/* file structure */
				0,							/* don't care the target address */
				size,						/* size */
				PROT_READ | PROT_WRITE,		/* Protection */
				MAP_SHARED,					/* flag */
				__pa((void *) va));						/* Offset */

	/* Check error */
	if ( (int) ua == 0 ) {
		printk("AMSWIRE: error from do_mmap, error = %d\n", (int) ua);
		kfree(mp_new);
		kfree((void *) va);
		return 0;
	}
#ifdef AMSW_DEBUG
	printk("AMSWIRE: Allocate kernel memory %d bytes @ 0x%08lx, mapped to 0x%08lx\n", size, va, ua);
#endif

	/* Get the old point for the next */
	mp_old = (AMSW_MDESC *) ((long) file->f_pos);
	mp_new->va = va;
	mp_new->ua = ua;
	mp_new->size = size;
	mp_new->next = mp_old;
	file->f_pos = (loff_t) ((long) mp_new);

	return (void *) ua;
}

/************************************************************************
	Function:
		amsw_dev_kfree - Free kernel memory for user space

	RETURNS:
		0 on success
 ************************************************************************/
void amsw_dev_kfree(struct file *file, void *addr) {
	AMSW_MDESC *mp, **mpp;

	/* Get the first point */
	mpp = (AMSW_MDESC **) &file->f_pos;

	/* Find address */
	for ( mp = *mpp; mp; mp = mp->next ) {
		if ( mp->ua == (u_long) addr ) break;
		mpp = &(mp->next);
	}

	/* Unmap memory */
#ifdef AMSW_DEBUG
	printk("AMSWIRE: unmap memory %d bytes @ 0x%08lx (virtual @ 0x%08lx)\n", mp->size, mp->ua, mp->va);
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,10)
	do_munmap(current->mm, (int) addr & PAGE_MASK, (mp->size + ~PAGE_MASK) & PAGE_MASK, 0);
#else
	do_munmap(current->mm, (int) addr & PAGE_MASK, (mp->size + ~PAGE_MASK) & PAGE_MASK);
#endif

	addr = (void *) mp->va;

	/* Remove reserved memory area from mem_map table */
	do {
		mem_map_unreserve(virt_to_page(addr));
		addr += PAGE_SIZE;
	} while ( (u_long) addr < mp->va + mp->size );

	/* Change the next point */
	*mpp = mp->next;

	/* Free memory */
	kfree((void *) mp->va);
	kfree(mp);
}

/************************************************************************
	Function:
		amsw_dev_ioctl - I/O control of AMSWire device

	RETURNS:
		0 on success
 ************************************************************************/
static int amsw_dev_ioctl(struct inode *inode, struct file *file, u_int cmd, u_long param) {
	AMSW_DEV *pd = &amswdev[MINOR(inode->i_rdev)];
	register u_long *reg = (u_long *) pd->vBar1;
	register int dma = (int) pd->vBar2;

	/* Check commands */
	switch (cmd & 0xff000000) {
		case AMSW_IOC_GET_REG:
            *((u_long *) param) = reg[cmd & 0xff];
			break;
		case AMSW_IOC_SET_REG:
            reg[cmd & 0xff] = param;
			break;
		case AMSW_IOC_GET_DMA:
            *((u_long *) param) = inl(dma + AMSW_DMA_OFFSET + ((cmd & 0xff) << 2));
			break;
		case AMSW_IOC_SET_DMA:
            outl(param, dma + AMSW_DMA_OFFSET + ((cmd & 0xff) << 2));
			break;
		case AMSW_IOC_KMALLOC:
            *((u_long *) param) = (u_long) amsw_dev_kmalloc(file, *((int *) param));
			break;
		case AMSW_IOC_KFREE:
            amsw_dev_kfree(file, (void *) param);
			break;
		case AMSW_IOC_TX:
			{
				AMSW_DATA *d = (AMSW_DATA *) param;

				return amsw_dev_tx(pd, d);
			}
		case AMSW_IOC_RX:
			{
				AMSW_DATA *d = (AMSW_DATA *) param;

				return amsw_dev_rx(pd, d);
			}
		case AMSW_IOC_POLL_TX:
			{
				int i;

				/* check TX status */
				for ( i = 0; i < 8; i++ ) {
					if ( reg[i * 2] & 1 ) amsw_set_bit(pd->tx_chan_stat, i);
				}

				*((int *) param) = (pd->tx_chan_stat & pd->tx_soft_stat);
			}
			break;
		case AMSW_IOC_POLL_RX:
			{
				AMSW_POLL *p = (AMSW_POLL *) param;
				int i;

#ifdef AMSW_USE_POLL
				/* Get and process the current status */
				amsw_dev_irq_proc((u_long) pd);
#endif

				p->mask = pd->rx_chan_stat;
				for ( i = 0; i < 8; i++ ) p->size[i] = pd->rx_size[i];
			}
			break;
		case AMSW_IOC_INIT:
			amsw_dev_init(pd);
			break;
	}

	return 0;
}

/************************************************************************
	Function:
		amsw_irq_handle - Interrupt handler routine

	RETURNS:
		0 on success
 ************************************************************************/
static void amsw_irq_handle(int irq, void *user_arg, struct pt_regs *regs) {
	register AMSW_DEV *pd = (AMSW_DEV *) user_arg;

	pd->irq_cnt++;

	/* call process routine */
	amsw_dev_irq_proc((u_long) pd);

	/* Set got IRQ flag */
	return;
}

/************************************************************************
	Function:
		amsw_probe - initialize AMSWire PCI module

	RETURNS:
		0 on success
 ************************************************************************/
static int amsw_probe(struct pci_dev *pdev, const struct pci_device_id *ent) {
	register AMSW_DEV *pd;
	u_long *reg;
	u_char  latency;

	/* Enable Device and set master */
	pci_write_config_word(pdev, PCI_COMMAND, (PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER));

	/* Get device information */
	pd = &amswdev[ncards++];
	pd->pdev = pdev;
	pd->irq = pdev->irq;

	/* Get BAR addresses */
	pd->pBar0 = pci_resource_start(pdev, 0);
	if ( !pd->pBar0 ) {
	    pci_read_config_dword(pdev, PCI_BASE_ADDRESS_0, (u_int *) &pd->pBar0);
	    pd->pBar0 &= 0xfffffff0;
	}
	pd->pBar1 = pci_resource_start(pdev, 1);
        if ( !pd->pBar1 ) {
	    pci_read_config_dword(pdev, PCI_BASE_ADDRESS_1, (u_int *) &pd->pBar1);
	    pd->pBar1 &= 0xfffffff0;
	}
	pd->pBar2 = pci_resource_start(pdev, 2);
        if ( !pd->pBar2 ) {
	    pci_read_config_dword(pdev, PCI_BASE_ADDRESS_2, (u_int *) &pd->pBar2);
	    pd->pBar2 &= 0xfffffff0;
	}
	
	/* Get BAR sizes */
	pd->size0 = pci_resource_len(pdev, 0);
	pd->size1 = pci_resource_len(pdev, 1);
	pd->size2 = pci_resource_len(pdev, 2);

	/* Map to virtual address */
	pd->vBar0 = (u_long) ioremap(pd->pBar0, pd->size0);
	pd->vBar1 = (u_long) ioremap(pd->pBar1, pd->size1);
	pd->vBar2 = pd->pBar2;		/* IO access */
	reg = (u_long *) pd->vBar1;

	/* Get versions */
	pci_read_config_word(pdev, PCI_CLASS_REVISION, &pd->a_ver);
	pd->x_ver = reg[AMSW_CSR_XVER];
	printk("AMSWIRE: Card No.%d: Actel version = %#4.4x, Xilinx version = %#8.8lx, Software version = %s\n",
		ncards - 1, pd->a_ver, pd->x_ver, amswire_vers);

#ifdef AMSW_SET_LATENCY_TIMER
	/* Set Latency Timer */
	if ( timer != AMSW_LATENCY_TIMER ) {
		timer = AMSW_LATENCY_TIMER;
		if ( pci_write_config_byte(pd->pdev, PCI_LATENCY_TIMER, timer) ) {
			printk("AMSWIRE: Read Latency Timer fails\n");
			return -EIO;
		}
	}
#endif	/* AMSW_SET_LATENCY_TIMER */

	/* Read the current setting */
	pci_read_config_byte(pdev, PCI_LATENCY_TIMER, &latency);

	printk("AMSWIRE:             BAR0 = %dK@%08lx, BAR1 = %d@%08lx, BAR2 = %d@%08lx, Latency = %d\n",
			pd->size0 / 1024, pd->pBar0, pd->size1, pd->pBar1, pd->size2, pd->vBar2, latency);

	/* initialize driver parameters */
	amsw_dev_init(pd);

	/* Read IRQ status word to clear pending interrupts */
	pd->irq_status = reg[AMSW_CSR_INT];

	/* Enable DMA according to paramter set */
	pd->dma_wr = dma_write;
	pd->dma_rd = dma_read;
	pd->dma_done = 0;

	/* Set driver interrupt mask */
#ifdef AMSW_USE_POLL
	pd->irq_mask = AMSW_MASK_DMADONE;
#else
	pd->irq_mask = AMSW_MASK_ALL;
#endif
	pd->irq_ena = 1;

	/* Start IRQ service */
	if ( request_irq(pd->irq, &amsw_irq_handle, SA_SHIRQ, device_name, pd) < 0 ) {
		printk("AMSWIRE: Fail to request IRQ\n");
		return -EFAULT;
	}

	/* Set device IRQ mask to really enable IRQ */
	reg[AMSW_CSR_MASK] = pd->irq_mask;

	/* Set device is not in use */
	pd->inuse = 0;

	return 0;
}


/************************************************************************
	Function:
		amsw_remove - remove AMSWire PCI module

	RETURNS:
		0 on success
 ************************************************************************/
static void amsw_remove(struct pci_dev *pdev) {
	register AMSW_DEV *pd;
	int card;

	/* Find card number */
	pd = &amswdev[0];
	for ( card = 0; card < ncards; card++ ) {
		pd = &amswdev[card];
		if ( pd->pdev == pdev ) break;
	}

	/* Unmap addresses */
	iounmap((void *) pd->vBar0);
	iounmap((void *) pd->vBar1);

	/* Free IRQ */
	free_irq(pd->irq, pd);

	printk("AMSWIRE: device driver for card No.%d is removed\n", card);
}

#define PROC_FILE_SUPPORT
#ifdef PROC_FILE_SUPPORT
/************************************************************************
	Function:
   		amsw_proc_read - Read information for PROC file

	RETURNS:
		0 on success
 ************************************************************************/
#define xPROCFS(fmt, args...) len += sprintf(buf+len, fmt, ##args)

int amsw_proc_read(char *buf, char **start, off_t offset, int length, int *eof, void *data) {
    int len = 0;
	int i, j;
    AMSW_DEV *pd;

    /* Global information */
    xPROCFS("---- Global Information:\n");
    xPROCFS("     Number of Cards:    %d\n", ncards);
    xPROCFS("     Software version:   %s\n\n", amswire_vers);

	for ( i = 0; i < ncards; i++ ) {
		pd = &amswdev[i];
		/* Board and driver information */
    	xPROCFS("---- Card %d:\n", i);
    	xPROCFS("     ---- Device Information\n");
    	xPROCFS("          BAR0:        %3dK @ 0x%08lx (0x%08lx)\n", pd->size0 / 1024, pd->vBar0, pd->pBar0);
    	xPROCFS("          BAR1:        %3d @ 0x%08lx (0x%08lx)\n", pd->size1, pd->vBar1, pd->pBar1);
    	xPROCFS("          BAR2:        %3d @ 0x%08lx (IO Space)\n", pd->size2, pd->vBar2);
    	xPROCFS("          IRQ Number:  %2d\n\n", pd->irq);

		xPROCFS("          Write DMA:   %s\n", (pd->dma_wr ? "Enabled" : "Disabled"));
    	xPROCFS("          Read DMA:    %s\n\n", (pd->dma_rd ? "Enabled" : "Disabled"));

		xPROCFS("     ---- Interrupts:\n");
    	xPROCFS("          Total:       %ld\n", pd->irq_cnt);
    	xPROCFS("          DMA:         %ld\n", pd->irq_dma_cnt);
    	xPROCFS("          RX:          %ld\n", pd->irq_rx_cnt);
    	xPROCFS("          Errors:      %ld\n", pd->irq_err_cnt);
    	xPROCFS("          Fails:       %ld\n", pd->irq_fail_cnt);
    	xPROCFS("          Status:      0x%08lx\n\n", pd->irq_status);

    	xPROCFS("     ---- DMA Operations:\n");
    	xPROCFS("          Total:       %ld\n", pd->dma_cnt);
    	xPROCFS("          Write:       %ld\n", pd->dma_wr_cnt);
    	xPROCFS("          Read:        %ld\n", pd->dma_rd_cnt);
    	xPROCFS("          Error:       %ld\n\n", pd->dma_err_cnt);

    	xPROCFS("     ---- TX Operations: (channel status = %02x/%02x)\n", pd->tx_chan_stat, pd->tx_soft_stat);
    	xPROCFS("          Channel:     ");
		for ( j = 0; j < 8; j++ ) xPROCFS("%-12d", j); xPROCFS("\n");
    	xPROCFS("          Total:       ");
		for ( j = 0; j < 8; j++ ) xPROCFS("%-12ld", pd->tx_cnt[j]); xPROCFS("\n");
    	xPROCFS("          Fail:        ");
		for ( j = 0; j < 8; j++ ) xPROCFS("%-12ld", pd->tx_fail_cnt[j]); xPROCFS("\n\n");

    	xPROCFS("     ---- RX Operations: (channel status = %02x)\n", pd->rx_chan_stat);
    	xPROCFS("          Channel:     ");
		for ( j = 0; j < 8; j++ ) xPROCFS("%-12d", j); xPROCFS("\n");
    	xPROCFS("          Total:       ");
		for ( j = 0; j < 8; j++ ) xPROCFS("%-12ld", pd->rx_cnt[j]); xPROCFS("\n");
    	xPROCFS("          Error:       ");
		for ( j = 0; j < 8; j++ ) xPROCFS("%-12ld", pd->rx_err_cnt[j]); xPROCFS("\n");
    	xPROCFS("          Fail:        ");
		for ( j = 0; j < 8; j++ ) xPROCFS("%-12ld", pd->rx_fail_cnt[j]); xPROCFS("\n");
    	xPROCFS("          Overrun:     ");
		for ( j = 0; j < 8; j++ ) xPROCFS("%-12ld", pd->rx_overrun_cnt[j]); xPROCFS("\n");
    	xPROCFS("          Status:      ");
		for ( j = 0; j < 8; j++ ) xPROCFS("%-12lx", pd->rx_status[j]); xPROCFS("\n\n");
	}

    *eof = 1;

    return len;
}
#endif  /* PROC_FILE_SUPPORT */

/* Define the AMSWire Device ID table */
static struct pci_device_id amsw_pci_tbl[] = {
	{AMS_VENDOR_ID, AMSW_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{0,}
};

/* Define AMSWire Device driver */
static struct pci_driver amsw_pci_driver = {
	name: device_name,
	probe: amsw_probe,
	remove: amsw_remove,
	id_table: amsw_pci_tbl,
};

/************************************************************************
	Function:
		init_module - install and initialize AMSWire PCI module

	RETURNS:
		0 on success
 ************************************************************************/
int init_module(void) {
	int err;

	/* Reset number of cards */
	ncards = 0;

	/* Initialize cards */
	if ( (err = pci_module_init(&amsw_pci_driver)) < 0 ) {
		if ( !ncards )
			printk("AMSWIRE: No AMSWire PCI card found.\n");
		else
			printk("AMSWIRE: error during pci_module_init, error = %x\n", err);
		return err;
	}

	/* Check if there is no cards found */
	if ( !ncards ) {
		printk("AMSWIRE: No AMSWire PCI card found.\n");
		return -ENODEV;
	}

	/* Register as a character device */
	if ( (err = register_chrdev(AMSW_MAJOR_NUMBER, device_name, &amsw_dev_fops)) < 0 ) {
		printk("AMSWIRE: Unable to allocate a major number. Error = %x\n", err);
		return -EIO;
	}

	printk("AMSWIRE: %d AMSW-PCI cards found in the system.\n", ncards);

#ifdef PROC_FILE_SUPPORT
    create_proc_read_entry("amsw",
                           0,                       /* default mode */
                           NULL,                    /* parent dir */
                           amsw_proc_read,     		/* Function to read data */
                           NULL);                   /* client data */
#endif  /* PROC_FILE_SUPPORT */

	return 0;
}


/************************************************************************
	Function:
		cleanup_module - deinstall AMSWire module

	RETURNS:
		none
 ************************************************************************/
void cleanup_module(void){

#ifdef PROC_FILE_SUPPORT
    remove_proc_entry("amsw", NULL);
#endif  /* if PROC_FILE_SUPPORT */

	/* Unregister device driver */
	pci_unregister_driver(&amsw_pci_driver);

	/* Unregister character device */
	if ( (unregister_chrdev(AMSW_MAJOR_NUMBER, device_name)) )
		printk("AMSWIRE: Fail to unregister character device\n");

	return;
}
