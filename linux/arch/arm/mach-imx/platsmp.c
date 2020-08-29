/*
 * Copyright 2011 Freescale Semiconductor, Inc.
 * Copyright 2011 Linaro Ltd.
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <linux/init.h>
#include <linux/of_address.h>
#include <linux/of.h>
#include <linux/smp.h>
#include <linux/types.h>

#include <asm/cacheflush.h>
#include <asm/page.h>
#include <asm/smp_scu.h>
#include <asm/mach/map.h>

#include "common.h"
#include "hardware.h"

u32 g_diag_reg;
static void __iomem *scu_base;

static u64 cpu_release_addr[NR_CPUS];

static struct map_desc scu_io_desc __initdata = {
	/* .virtual and .pfn are run-time assigned */
	.length		= SZ_4K,
	.type		= MT_DEVICE,
};

void __init imx_scu_map_io(void)
{
	unsigned long base;

	/* Get SCU base */
	asm("mrc p15, 4, %0, c15, c0, 0" : "=r" (base));

	scu_io_desc.virtual = IMX_IO_P2V(base);
	scu_io_desc.pfn = __phys_to_pfn(base);
	iotable_init(&scu_io_desc, 1);

	scu_base = IMX_IO_ADDRESS(base);
}

static int imx_boot_secondary(unsigned int cpu, struct task_struct *idle)
{
	imx_set_cpu_jump(cpu, v7_secondary_startup);
	imx_enable_cpu(cpu, true);
	return 0;
}

/*
 * Initialise the CPU possible map early - this describes the CPUs
 * which may be present or become present in the system.
 */
static void __init imx_smp_init_cpus(void)
{
	int i, ncores;

	ncores = scu_get_core_count(scu_base);

	for (i = ncores; i < NR_CPUS; i++)
		set_cpu_possible(i, false);
}

void imx_smp_prepare(void)
{
	scu_enable(scu_base);
}

static void __init imx_smp_prepare_cpus(unsigned int max_cpus)
{
	imx_smp_prepare();

	/*
	 * The diagnostic register holds the errata bits.  Mostly bootloader
	 * does not bring up secondary cores, so that when errata bits are set
	 * in bootloader, they are set only for boot cpu.  But on a SMP
	 * configuration, it should be equally done on every single core.
	 * Read the register from boot cpu here, and will replicate it into
	 * secondary cores when booting them.
	 */
	asm("mrc p15, 0, %0, c15, c0, 1" : "=r" (g_diag_reg) : : "cc");
	sync_cache_w(&g_diag_reg);
}

struct smp_operations  imx_smp_ops __initdata = {
	.smp_init_cpus		= imx_smp_init_cpus,
	.smp_prepare_cpus	= imx_smp_prepare_cpus,
	.smp_boot_secondary	= imx_boot_secondary,
#ifdef CONFIG_HOTPLUG_CPU
	.cpu_die		= imx_cpu_die,
	.cpu_kill		= imx_cpu_kill,
#endif
};

#define DCFG_CCSR_SCRATCHRW1	0x200

static int ls1021a_boot_secondary(unsigned int cpu, struct task_struct *idle)
{
	arch_send_wakeup_ipi_mask(cpumask_of(cpu));

	return 0;
}

static void __init ls1021a_smp_prepare_cpus(unsigned int max_cpus)
{
	struct device_node *np;
	void __iomem *dcfg_base;
	unsigned long paddr;

	np = of_find_compatible_node(NULL, NULL, "fsl,ls1021a-dcfg");
	dcfg_base = of_iomap(np, 0);
	BUG_ON(!dcfg_base);

	paddr = virt_to_phys(secondary_startup);
	writel_relaxed(cpu_to_be32(paddr), dcfg_base + DCFG_CCSR_SCRATCHRW1);

	iounmap(dcfg_base);
}

struct smp_operations  ls1021a_smp_ops __initdata = {
	.smp_prepare_cpus	= ls1021a_smp_prepare_cpus,
	.smp_boot_secondary	= ls1021a_boot_secondary,
};

static int layerscape_smp_boot_secondary(unsigned int cpu,
					 struct task_struct *idle)
{
	u32 secondary_startup_phys;
	__le32 __iomem *release_addr;

	secondary_startup_phys = virt_to_phys(secondary_startup);

	release_addr = ioremap_cache((u32)cpu_release_addr[cpu],
				     sizeof(u64));
	if (!release_addr)
		return -ENOMEM;

	writel_relaxed(secondary_startup_phys, release_addr);
	writel_relaxed(0, release_addr + 1);
	__cpuc_flush_dcache_area((__force void *)release_addr,
				 sizeof(u64));

	sev();

	iounmap(release_addr);

	return 0;
}

static void layerscape_smp_init_cpus(void)
{
	struct device_node *dnt = NULL;
	unsigned int cpu = 0;

	while ((dnt = of_find_node_by_type(dnt, "cpu"))) {
		if (of_property_read_u64(dnt, "cpu-release-addr",
		    &cpu_release_addr[cpu])) {
			pr_err("CPU %d: missing or invalid cpu-release-addr property\n",
			cpu);
		}

		cpu++;
	}
}

const struct smp_operations layerscape_smp_ops __initconst = {
	.smp_init_cpus		= layerscape_smp_init_cpus,
	.smp_boot_secondary	= layerscape_smp_boot_secondary,
};