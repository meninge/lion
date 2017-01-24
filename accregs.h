
#define ACCREG3_WMODE_MASK        0x0000000F
#define ACCREG3_WMODE_FRAME       0x00000001
#define ACCREG3_WMODE_LEVEL1      0x00000002
#define ACCREG3_WMODE_RECODE12    0x00000004
#define ACCREG3_WMODE_LEVEL2      0x00000008

#define ACCREG3_FLAG_CLEAR        0x00000100

#define ACCREG3_FLAG_BUSYW        0x00000200
#define ACCREG3_FLAG_BUSYR        0x00000400

#define accreg_get_lvl1_nbneu()   (accreg_rd(0) & 0x0FFFF)
#define accreg_get_lvl2_nbneu()   ((accreg_rd(0) >> 16) & 0x0FFFF)
#define accreg_get_lvl1_nbin()    (accreg_rd(1) & 0x0FFFF)

#define accreg_set_wmode_frame()  accreg_wr(3, (accreg_rd(3) & ~ACCREG3_WMODE_MASK) | ACCREG3_WMODE_FRAME)
#define accreg_set_wmode_lvl1()   accreg_wr(3, (accreg_rd(3) & ~ACCREG3_WMODE_MASK) | ACCREG3_WMODE_LEVEL1)
#define accreg_set_wmode_rec12()  accreg_wr(3, (accreg_rd(3) & ~ACCREG3_WMODE_MASK) | ACCREG3_WMODE_RECODE12)
#define accreg_set_wmode_lvl2()   accreg_wr(3, (accreg_rd(3) & ~ACCREG3_WMODE_MASK) | ACCREG3_WMODE_LEVEL2)

#define accreg_set_lvl1_fsize(nb) accreg_wr(0, (accreg_rd(0) & 0xFFFF0000) | (nb))
#define accreg_set_lvl1_nbneu(nb) accreg_wr(1, (accreg_rd(1) & 0xFFFF0000) | (nb))
#define accreg_set_lvl2_nbneu(nb) accreg_wr(2, (accreg_rd(2) & 0xFFFF0000) | (nb))

#define accreg_clear()            accreg_wr(3, accreg_rd(3) | ACCREG3_FLAG_CLEAR)
#define accreg_check_clear()      ((accreg_rd(3) & ACCREG3_FLAG_CLEAR) != 0)

#define accreg_check_busyw()      ((accreg_rd(3) & ACCREG3_FLAG_BUSYW) != 0)
#define accreg_check_busyr()      ((accreg_rd(3) & ACCREG3_FLAG_BUSYR) != 0)

#define accreg_set_nboutputs(nb)  accreg_wr(6, nb)

// Send size of the 32 bits array and returns the needed number of bursts
#define MINIMUM_BURSTS(n) ((((n) % 16 == 0) ? (n) : ((n) + 16)) / 16)


extern unsigned accreg_lvl1_max_fsize;
extern unsigned accreg_lvl1_max_nbneu;
extern unsigned accreg_lvl2_max_nbneu;

extern unsigned accreg_lvl1_fsize;
extern unsigned accreg_lvl1_nbneu;
extern unsigned accreg_lvl2_nbneu;

extern volatile unsigned* accregs_ptr;

static inline uint32_t accreg_rd(unsigned reg) {
	return accregs_ptr[reg];
}
static inline void accreg_wr(unsigned reg, uint32_t v) {
	accregs_ptr[reg] = v;
}

void accreg_print_regs();

void accreg_config_get();
void accreg_config_print();

void accregs_print_fifo_counts();
int32_t accregs_pop_r2();
int32_t accregs_pop_1r();
int32_t accregs_pop_rd();
void accregs_push_rd(uint32_t val);


