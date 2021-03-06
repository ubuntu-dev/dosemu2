#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <assert.h>

void msdos_api_call(sigcontext_t *scp, void *arg);
void msdos_api_winos2_call(sigcontext_t *scp, void *arg);
void xms_call(const sigcontext_t *scp,
	struct RealModeCallStructure *rmreg, void *arg);
void xms_ret(sigcontext_t *scp,
	const struct RealModeCallStructure *rmreg);

void set_io_buffer(uint8_t *ptr, unsigned int size);
void unset_io_buffer(void);
int is_io_error(uint16_t *r_code);

void callbacks_init(unsigned short rmcb_sel, void *(*cbk_args)(int),
	far_t *r_cbks);
void callbacks_done(far_t *r_cbks);

void rm_to_pm_regs(sigcontext_t *scp,
			  const struct RealModeCallStructure *rmreg,
			  unsigned int mask);

#ifdef DOSEMU
#define RMREG(r) (rmreg->r)
#define X_RMREG(r) (rmreg->e##r)
#define RMLWORD(r) LO_WORD_(X_RMREG(r), const)
#define E_RMREG(r) (rmreg->r)
#endif
/* pre_extender() is allowed to read only a small set of rmregs, check mask */
#define READ_RMREG(r, m) (assert(m & (1 << r##_INDEX)), RMREG(r))
#define ip_INDEX eip_INDEX
#define sp_INDEX esp_INDEX
#define flags_INDEX eflags_INDEX
#define RM_LO(r) LO_BYTE_(RMLWORD(r), const)

enum { RMCB_IO, RMCB_MS, RMCB_PS2MS, MAX_RMCBS };

#endif
