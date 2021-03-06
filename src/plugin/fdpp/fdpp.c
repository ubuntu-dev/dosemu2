/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * Purpose: fdpp kernel support
 *
 * Author: Stas Sergeev
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fdpp/thunks.h>
#include <fdpp/bprm.h>
#if FDPP_API_VER != 12
#error wrong fdpp version
#endif
#include "emu.h"
#include "init.h"
#include "int.h"
#include "utilities.h"
#include "coopth.h"
#include "dos2linux.h"
#include "fatfs.h"
#include "disks.h"
#include "doshelpers.h"

static char fdpp_krnl[16];
#define MAX_CLNUP_TIDS 5
static int clnup_tids[MAX_CLNUP_TIDS];
static int num_clnup_tids;

static void copy_stk(uint8_t *sp, uint8_t len)
{
    uint8_t *stk;
    if (!len)
	return;
    LWORD(esp) -= len;
    stk = SEG_ADR((uint8_t *), ss, sp);
    memcpy(stk, sp, len);
}

static void fdpp_call(struct vm86_regs *regs, uint16_t seg,
        uint16_t off, uint8_t *sp, uint8_t len,
        jmp_buf *canc_jmp, jmp_buf **canc_prev)
{
    struct vm86_regs saved_regs = REGS;
    REGS = *regs;
    copy_stk(sp, len);
    assert(num_clnup_tids < MAX_CLNUP_TIDS);
    clnup_tids[num_clnup_tids++] = coopth_get_tid();
    if (canc_jmp)
	*canc_prev = coopth_set_cancel_target(canc_jmp);
    do_call_back(seg, off);
    num_clnup_tids--;
    *regs = REGS;
    REGS = saved_regs;
}

static void fdpp_call_noret(struct vm86_regs *regs, uint16_t seg,
	uint16_t off, uint8_t *sp, uint8_t len)
{
    REGS = *regs;
    coopth_leave();
    fake_iret();
    copy_stk(sp, len);
    jmp_to(0xffff, 0);
    fake_call_to(seg, off);
}

static void fdpp_abort(const char *file, int line)
{
    p_dos_str("\nfdpp crashed.\n");
    dosemu_error("fdpp: abort at %s:%i\n", file, line);
    p_dos_str("Press any key to exit.\n");
    set_IF();
    com_biosgetch();
    clear_IF();
    leavedos(3);
}

static void fdpp_exit(int rc)
{
    leavedos(rc);
}

static void fdpp_panic(const char *msg)
{
    error("fdpp: PANIC: %s\n", msg);
    p_dos_str("PANIC: %s\n", msg);
    p_dos_str("Press any key to exit.\n");
    set_IF();
    com_biosgetch();
    clear_IF();
    leavedos(3);
}

static void fdpp_print(int prio, const char *format, va_list ap)
{
    switch(prio) {
    case FDPP_PRINT_TERMINAL:
        vprintf(format, ap);
        break;
    case FDPP_PRINT_LOG:
        vlog_printf(-1, format, ap);
        break;
    case FDPP_PRINT_SCREEN:
        p_dos_vstr(format, ap);
        break;
    }
}

static uint8_t *fdpp_so2lin(uint16_t seg, uint16_t off)
{
    return LINEAR2UNIX(SEGOFF2LINEAR(seg, off));
}

static void fdpp_relax(void)
{
    int ii = isset_IF();

    set_IF();
    coopth_wait();
    if (!ii)
	clear_IF();
}

static void fdpp_debug(const char *msg)
{
    dosemu_error("%s\n", msg);
}

static void fdpp_cleanup(void)
{
    int i;
    for (i = 0; i < num_clnup_tids; i++) {
        coopth_cancel(clnup_tids[i]);
        coopth_unsafe_detach(clnup_tids[i], __FILE__);
    }
    num_clnup_tids = 0;
}

static struct fdpp_api api = {
    .so2lin = fdpp_so2lin,
    .exit = fdpp_exit,
    .abort = fdpp_abort,
    .print = fdpp_print,
    .debug = fdpp_debug,
    .panic = fdpp_panic,
    .cpu_relax = fdpp_relax,
    .asm_call = fdpp_call,
    .asm_call_noret = fdpp_call_noret,
};

static void fdpp_pre_boot(void)
{
    int i;
    struct _bprm bprm;
    uint16_t bprm_seg = 0x1fe0 + 0x7c0 + 0x20;  // stack+bs
    uint16_t seg = 0x0060;
    uint16_t ofs = 0x0000;
    dosaddr_t loadaddress = SEGOFF2LINEAR(seg, ofs);

    error("fdpp booting, this is very experimental!\n");
    LWORD(eax) = bprm_seg;
    HI(bx) = BPRM_VER;

    LO(bx) = 0x80;
    FOR_EACH_HDISK(i, {
	if (disk_root_contains(&hdisktab[i], CONF4_IDX)) {
	    bprm.CfgDrive = hdisktab[i].drive_num;
	    break;
	}
    });

    FOR_EACH_HDISK(i, {
	if (disk_root_contains(&hdisktab[i], CMD_IDX)) {
	    uint8_t drv_num = hdisktab[i].drive_num;
	    fatfs_t *f1 = get_fat_fs_by_drive(drv_num);
	    struct sys_dsc *sf1 = fatfs_get_sfiles(f1);

	    bprm.ShellDrive = drv_num;
	    if (sf1[CMD_IDX].flags & FLG_COMCOM32)
		error("booting with comcom32, this is very experimental\n");
	    break;
	}
    });

    FOR_EACH_HDISK(i, {
	if (disk_root_contains(&hdisktab[i], DEMU_IDX)) {
	    bprm.DeviceDrive = hdisktab[i].drive_num;
	    break;
	}
    });

    FOR_EACH_HDISK(i, {
	if (disk_root_contains(&hdisktab[i], AUT2_IDX)) {
	    uint16_t seg = bprm_seg + 8;
	    char *env = SEG2LINEAR(seg);
	    char drv = HDISK_NUM(i) + 'A';
	    int len = sprintf(env, "DOSEMUDRV=%c", drv);
	    uint8_t drv_num = hdisktab[i].drive_num;
	    fatfs_t *f1 = get_fat_fs_by_drive(drv_num);
	    struct sys_dsc *sf1 = fatfs_get_sfiles(f1);

	    len++;
	    len += sprintf(env + len, "FDPP_AUTOEXEC=%c:\\%s", drv,
	        sf1[AUT2_IDX].name);
	    len++;
	    len += sprintf(env + len, "#0 :SWITCHES=/F");
	    len++;
	    env[len] = '\0'; // second terminator
	    bprm.InitEnvSeg = seg;
	    break;
	}
    });
    MEMCPY_2DOS(SEGOFF2LINEAR(bprm_seg, 0), &bprm, sizeof(bprm));

    SREG(ds)  = loadaddress >> 4;
    SREG(es)  = loadaddress >> 4;
    SREG(ss)  = 0x1FE0;
    LWORD(esp) = 0x7c00;  /* temp stack */
    LWORD(ebp) = 0x7C00;
    SREG(cs)  = seg;
    LWORD(eip) = ofs;

    register_plugin_call(DOS_HELPER_PLUGIN_ID_FDPP, FdppCall);
    register_cleanup_handler(fdpp_cleanup);
    int_try_disable_revect();
    /* try disable int hooks as well */
    if (config.int_hooks == -1)
	config.int_hooks = config.force_revect;
}

static void fdpp_fatfs_hook(struct sys_dsc *sfiles, fatfs_t *fat)
{
    const char *dir = fatfs_get_host_dir(fat);
    const struct sys_dsc sys_fdpp = { .name = fdpp_krnl, .is_sys = 1,
	    .pre_boot = fdpp_pre_boot };

    if (strcmp(dir, fddir_boot) != 0)
	return;
    sfiles[FDP_IDX] = sys_fdpp;
}

CONSTRUCTOR(static void init(void))
{
    int req_ver = 0;
    char *fdpath;
    const char *fdkrnl;
    const char *fddir = NULL;
    int err = FdppInit(&api, FDPP_API_VER, &req_ver);
    if (err) {
	if (req_ver != FDPP_API_VER)
	    error("fdpp version mismatch: %i %i\n", FDPP_API_VER, req_ver);
	leavedos(3);
    }
    fddir = getenv("FDPP_KERNEL_DIR");
    if (!fddir)
	fddir = FdppDataDir();
    assert(fddir);

    fdkrnl = FdppKernelName();
    assert(fdkrnl);
    fdpath = assemble_path(fddir, fdkrnl);
    err = access(fdpath, R_OK);
    free(fdpath);
    if (err)
	return;
    strcpy(fdpp_krnl, fdkrnl);
    strupper(fdpp_krnl);
    fddir_boot = strdup(fddir);
    fatfs_set_sys_hook(fdpp_fatfs_hook);
}
