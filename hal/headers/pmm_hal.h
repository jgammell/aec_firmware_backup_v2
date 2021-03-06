/*
 * pmm_hal.h
 *
 *  Created on: Feb 13, 2021
 *      Author: jgamm
 */

#ifndef HAL_HEADERS_PMM_HAL_H_
#define HAL_HEADERS_PMM_HAL_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    volatile       uint8_t  PMMCTL0L;
    volatile       uint8_t  PMMCTL0H;
    volatile const uint16_t PMMCTL1;
    volatile       uint16_t SVSMHCTL;
    volatile       uint16_t SVSMLCTL;
    volatile       uint16_t SVSMIO;
    volatile const uint16_t RESERVED0;
    volatile       uint16_t PMMIFG;
    volatile       uint16_t PMMRIE;
    volatile       uint16_t PM5CTL0;
} PMM_Registers_Struct;

#define PMM0 ((PMM_Registers_Struct *) 0x0120U)

// Power management module control register 0 (PMMCTL0)
#define PMM_PMMCTL0_RESET (0x9600U)

#define PMM_PMMCTL0_PMMCOREV_SHIFT  (0U)
#define PMM_PMMCTL0_PMMSWBOR_SHIFT  (2U)
#define PMM_PMMCTL0_PMMSWPOR_SHIFT  (3U)
#define PMM_PMMCTL0_PMMREGOFF_SHIFT (4U)
#define PMM_PMMCTL0_PMMPW_SHIFT     (8U)

#define PMM_PMMCTL0_PMMCOREV_MASK  (3U)
#define PMM_PMMCTL0_PMMSWBOR_MASK  (1U)
#define PMM_PMMCTL0_PMMSWPOR_MASK  (1U)
#define PMM_PMMCTL0_PMMREGOFF_MASK (1U)
#define PMM_PMMCTL0_PMMPW_MASK     (0xFFU)

#define PMM_PMMCTL0_PMMCOREV  (PMM_PMMCTL0_PMMCOREV_MASK<<PMM_PMMCTL0_PMMCOREV_SHIFT)
#define PMM_PMMCTL0_PMMSWBOR  (PMM_PMMCTL0_PMMSWBOR_MASK<<PMM_PMMCTL0_PMMSWBOR_SHIFT)
#define PMM_PMMCTL0_PMMSWPOR  (PMM_PMMCTL0_PMMSWPOR_MASK<<PMM_PMMCTL0_PMMSWPOR_SHIFT)
#define PMM_PMMCTL0_PMMREGOFF (PMM_PMMCTL0_PMMREGOFF_MASK<<PMM_PMMCTL0_PMMREGOFF_SHIFT)
#define PMM_PMMCTL0_PMMPW     (PMM_PMMCTL0_PMMPW_MASK<<PMM_PMMCTL0_PMMPW_SHIFT)

typedef enum
{
    pmmPmmCoreV0 = 0U<<PMM_PMMCTL0_PMMCOREV_SHIFT,
    pmmPmmCoreV1 = 1U<<PMM_PMMCTL0_PMMCOREV_SHIFT,
    pmmPmmCoreV2 = 2U<<PMM_PMMCTL0_PMMCOREV_SHIFT,
    pmmPmmCoreV3 = 3U<<PMM_PMMCTL0_PMMCOREV_SHIFT
} PMM_PmmCoreV_Enum;

// Supply voltage supervisor and monitor high-side control register (SVSMHCTL)
#define PMM_SVSMHCTL_RESET (0x4400U)

#define PMM_SVSMHCTL_SVSMHRRL_SHIFT   (0U)
#define PMM_SVSMHCTL_SVSMHDLYST_SHIFT (3U)
#define PMM_SVSMHCTL_SVSHMD_SHIFT     (4U)
#define PMM_SVSMHCTL_SVSMHEVM_SHIFT   (6U)
#define PMM_SVSMHCTL_SVSHACE_SHIFT    (7U)
#define PMM_SVSMHCTL_SVSHRVL_SHIFT    (8U)
#define PMM_SVSMHCTL_SVSHE_SHIFT      (10U)
#define PMM_SVSMHCTL_SVSHFP_SHIFT     (11U)
#define PMM_SVSMHCTL_SVMHOVPE_SHIFT   (12U)
#define PMM_SVSMHCTL_SVMHE_SHIFT      (14U)
#define PMM_SVSMHCTL_SVMHFP_SHIFT     (15U)

#define PMM_SVSMHCTL_SVSMHRRL_MASK   (7U)
#define PMM_SVSMHCTL_SVSMHDLYST_MASK (1U)
#define PMM_SVSMHCTL_SVSHMD_MASK     (1U)
#define PMM_SVSMHCTL_SVSMHEVM_MASK   (1U)
#define PMM_SVSMHCTL_SVSHACE_MASK    (1U)
#define PMM_SVSMHCTL_SVSHRVL_MASK    (3U)
#define PMM_SVSMHCTL_SVSHE_MASK      (1U)
#define PMM_SVSMHCTL_SVSHFP_MASK     (1U)
#define PMM_SVSMHCTL_SVMHOVPE_MASK   (1U)
#define PMM_SVSMHCTL_SVMHE_MASK      (1U)
#define PMM_SVSMHCTL_SVMHFP_MASK     (1U)

#define PMM_SVSMHCTL_SVSMHRRL   (PMM_SVSMHCTL_SVSMHRRL_MASK<<PMM_SVSMHCTL_SVSMHRRL_SHIFT)
#define PMM_SVSMHCTL_SVSMHDLYST (PMM_SVSMHCTL_SVMHE_MASK<<PMM_SVSMHCTL_SVSMHDLYST_SHIFT)
#define PMM_SVSMHCTL_SVSHMD     (PMM_SVSMHCTL_SVSHMD_MASK<<PMM_SVSMHCTL_SVSHMD_SHIFT)
#define PMM_SVSMHCTL_SVSMHEVM   (PMM_SVSMHCTL_SVSMHEVM_MASK<<PMM_SVSMHCTL_SVSMHEVM_SHIFT)
#define PMM_SVSMHCTL_SVSHACE    (PMM_SVSMHCTL_SVSHACE_MASK<<PMM_SVSMHCTL_SVSHACE_SHIFT)
#define PMM_SVSMHCTL_SVSHRVL    (PMM_SVSMHCTL_SVSHRVL_MASK<<PMM_SVSMHCTL_SVSHRVL_SHIFT)
#define PMM_SVSMHCTL_SVSHE      (PMM_SVSMHCTL_SVSHE_MASK<<PMM_SVSMHCTL_SVSHE_SHIFT)
#define PMM_SVSMHCTL_SVSHFP     (PMM_SVSMHCTL_SVSHFP_MASK<<PMM_SVSMHCTL_SVSHFP_SHIFT)
#define PMM_SVSMHCTL_SVMHOVPE   (PMM_SVSMHCTL_SVMHOVPE_MASK<<PMM_SVSMHCTL_SVMHOVPE_SHIFT)
#define PMM_SVSMHCTL_SVMHE      (PMM_SVSMHCTL_SVMHE_MASK<<PMM_SVSMHCTL_SVMHE_SHIFT)
#define PMM_SVSMHCTL_SVMHFP     (PMM_SVSMHCTL_SVMHFP_MASK<<PMM_SVSMHCTL_SVMHFP_SHIFT)

// Supply voltage supervisor and monitor low-side control register
#define PMM_SVSMLCTL_RESET (0x4400U)

#define PMM_SVSMLCTL_SVSMLRRL_SHIFT  (0U)
#define PMM_SVSMLCTL_SVSMLDLYT_SHIFT (3U)
#define PMM_SVSMLCTL_SVSLMD_SHIFT    (4U)
#define PMM_SVSMLCTL_SVSMLEVM_SHIFT  (6U)
#define PMM_SVSMLCTL_SVSMLACE_SHIFT  (7U)
#define PMM_SVSMLCTL_SVSLRVL_SHIFT   (8U)
#define PMM_SVSMLCTL_SVSLE_SHIFT     (10U)
#define PMM_SVSMLCTL_SVSLFP_SHIFT    (11U)
#define PMM_SVSMLCTL_SVMLOVPE_SHIFT  (12U)
#define PMM_SVSMLCTL_SVMLE_SHIFT     (14U)
#define PMM_SVSMLCTL_SVMLFP_SHIFT    (15U)

#define PMM_SVSMLCTL_SVSMLRRL_MASK  (7U)
#define PMM_SVSMLCTL_SVSMLDLYT_MASK (1U)
#define PMM_SVSMLCTL_SVSLMD_MASK    (1U)
#define PMM_SVSMLCTL_SVSMLEVM_MASK  (1U)
#define PMM_SVSMLCTL_SVSMLACE_MASK  (1U)
#define PMM_SVSMLCTL_SVSLRVL_MASK   (3U)
#define PMM_SVSMLCTL_SVSLE_MASK     (1U)
#define PMM_SVSMLCTL_SVSLFP_MASK    (1U)
#define PMM_SVSMLCTL_SVMLOVPE_MASK  (1U)
#define PMM_SVSMLCTL_SVMLE_MASK     (1U)
#define PMM_SVSMLCTL_SVMLFP_MASK    (1U)

#define PMM_SVSMLCTL_SVSMLRRL  (PMM_SVSMLCTL_SVSMLRRL_MASK<<PMM_SVSMLCTL_SVSMLRRL_SHIFT)
#define PMM_SVSMLCTL_SVSMLDLYT (PMM_SVSMLCTL_SVSMLDLYT_MASK<<PMM_SVSMLCTL_SVSMLDLYT_SHIFT)
#define PMM_SVSMLCTL_SVSLMD    (PMM_SVSMLCTL_SVSLMD_MASK<<PMM_SVSMLCTL_SVSLMD_SHIFT)
#define PMM_SVSMLCTL_SVSMLEVM  (PMM_SVSMLCTL_SVSMLEVM_MASK<<PMM_SVSMLCTL_SVSMLEVM_SHIFT)
#define PMM_SVSMLCTL_SVSMLACE  (PMM_SVSMLCTL_SVSMLACE_MASK<<PMM_SVSMLCTL_SVSMLACE_SHIFT)
#define PMM_SVSMLCTL_SVSLRVL   (PMM_SVSMLCTL_SVSLRVL_MASK<<PMM_SVSMLCTL_SVSLRVL_SHIFT)
#define PMM_SVSMLCTL_SVSLE     (PMM_SVSMLCTL_SVSLE_MASK<<PMM_SVSMLCTL_SVSLE_SHIFT)
#define PMM_SVSMLCTL_SVSLFP    (PMM_SVSMLCTL_SVSLFP_MASK<<PMM_SVSMLCTL_SVSLFP_SHIFT)
#define PMM_SVSMLCTL_SVMLOVPE  (PMM_SVSMLCTL_SVMLOVPE_MASK<<PMM_SVSMLCTL_SVMLOVPE_SHIFT)
#define PMM_SVSMLCTL_SVMLE     (PMM_SVSMLCTL_SVMLE_MASK<<PMM_SVSMLCTL_SVMLE_SHIFT)
#define PMM_SVSMLCTL_SVMLFP    (PMM_SVSMLCTL_SVMLFP_MASK<<PMM_SVSMLCTL_SVMLFP_SHIFT)

// SVSIN and SVMOUT Control Register (SVSMIO)
#define PMM_SVSMIO_RESET (0x0020U)

#define PMM_SVSMIO_SVMLOE_SHIFT    (3U)
#define PMM_SVSMIO_SVMLVLROE_SHIFT (4U)
#define PMM_SVSMIO_SVMOUTPOL_SHIFT (5U)
#define PMM_SVSMIO_SVMHOE_SHIFT    (11U)
#define PMM_SVSMIO_SVMHVLROE_SHIFT (12U)

#define PMM_SVSMIO_SVMLOE_MASK    (1U)
#define PMM_SVSMIO_SVMLVLROE_MASK (1U)
#define PMM_SVSMIO_SVMOUTPOL_MASK (1U)
#define PMM_SVSMIO_SVMHOE_MASK    (1U)
#define PMM_SVSMIO_SVMHVLROE_MASK (1U)

#define PMM_SVSMIO_SVMLOE    (PMM_SVSMIO_SVMLOE_MASK<<PMM_SVSMIO_SVMLOE_SHIFT)
#define PMM_SVSMIO_SVMLVLROE (PMM_SVSMIO_SVMLVLROE_MASK<<PMM_SVSMIO_SVMLVLROE_SHIFT)
#define PMM_SVSMIO_SVMOUTPOL (PMM_SVSMIO_SVMOUTPOL_MASK<<PMM_SVSMIO_SVMOUTPOL_SHIFT)
#define PMM_SVSMIO_SVMHOE    (PMM_SVSMIO_SVMHOE_MASK<<PMM_SVSMIO_SVMHOE_SHIFT)
#define PMM_SVSMIO_SVMHVLROE (PMM_SVSMIO_SVMHVLROE_MASK<<PMM_SVSMIO_SVMHVLROE_SHIFT)

// Power Management Module Interrupt Flag Register (PMMIFG)
#define PMM_PMMIFG_RESET (0x0000U)

#define PMM_PMMIFG_SVSMLDLYIFG_SHIFT (0U)
#define PMM_PMMIFG_SVMLIFG_SHIFT     (1U)
#define PMM_PMMIFG_SVMLVLRIFG_SHIFT  (2U)
#define PMM_PMMIFG_SVSMHDLYIFG_SHIFT (4U)
#define PMM_PMMIFG_SVMHIFG_SHIFT     (5U)
#define PMM_PMMIFG_SVMHVLRIFG_SHIFT  (6U)
#define PMM_PMMIFG_PMMBORIFG_SHIFT   (8U)
#define PMM_PMMIFG_PMMRSTIFG_SHIFT   (9U)
#define PMM_PMMIFG_PMMPORIFG_SHIFT   (10U)
#define PMM_PMMIFG_SVSHIFG_SHIFT     (12U)
#define PMM_PMMIFG_SVSLIFG_SHIFT     (13U)
#define PMM_PMMIFG_PMMLPM5IFG_SHIFT  (15U)

#define PMM_PMMIFG_SVSMLDLYIFG_MASK (1U)
#define PMM_PMMIFG_SVMLIFG_MASK     (1U)
#define PMM_PMMIFG_SVMLVLRIFG_MASK  (1U)
#define PMM_PMMIFG_SVSMHDLYIFG_MASK (1U)
#define PMM_PMMIFG_SVMHIFG_MASK     (1U)
#define PMM_PMMIFG_SVMHVLRIFG_MASK  (1U)
#define PMM_PMMIFG_PMMBORIFG_MASK   (1U)
#define PMM_PMMIFG_PMMRSTIFG_MASK   (1U)
#define PMM_PMMIFG_PMMPORIFG_MASK   (1U)
#define PMM_PMMIFG_SVSHIFG_MASK     (1U)
#define PMM_PMMIFG_SVSLIFG_MASK     (1U)
#define PMM_PMMIFG_PMMLPM5IFG_MASK  (1U)

#define PMM_PMMIFG_SVSMLDLYIFG (PMM_PMMIFG_SVSMLDLYIFG_MASK<<PMM_PMMIFG_SVSMLDLYIFG_SHIFT)
#define PMM_PMMIFG_SVMLIFG     (PMM_PMMIFG_SVMLIFG_MASK<<PMM_PMMIFG_SVMLIFG_SHIFT)
#define PMM_PMMIFG_SVMLVLRIFG  (PMM_PMMIFG_SVMLVLRIFG_MASK<<PMM_PMMIFG_SVMLVLRIFG_SHIFT)
#define PMM_PMMIFG_SVSMHDLYIFG (PMM_PMMIFG_SVSMHDLYIFG_MASK<<PMM_PMMIFG_SVSMLDLYIFG_SHIFT)
#define PMM_PMMIFG_SVMHIFG     (PMM_PMMIFG_SVMHIFG_MASK<<PMM_PMMIFG_SVMHIFG_SHIFT)
#define PMM_PMMIFG_SVMHVLRIFG  (PMM_PMMIFG_SVMHVLRIFG_MASK<<PMM_PMMIFG_SVMHVLRIFG_SHIFT)
#define PMM_PMMIFG_PMMBORIFG   (PMM_PMMIFG_PMMBORIFG_MASK<<PMM_PMMIFG_PMMBORIFG_SHIFT)
#define PMM_PMMIFG_PMMRSTIFG   (PMM_PMMIFG_PMMRSTIFG_MASK<<PMM_PMMIFG_PMMRSTIFG_SHIFT)
#define PMM_PMMIFG_PMMPORIFG   (PMM_PMMIFG_PMMPORIFG_MASK<<PMM_PMMIFG_PMMPORIFG_SHIFT)
#define PMM_PMMIFG_SVSHIFG     (PMM_PMMIFG_SVSHIFG_MASK<<PMM_PMMIFG_SVSHIFG_SHIFT)
#define PMM_PMMIFG_SVSLIFG     (PMM_PMMIFG_SVSLIFG_MASK<<PMM_PMMIFG_SVSLIFG_SHIFT)
#define PMM_PMMIFG_PMMLPM5IFG  (PMM_PMMIFG_PMMLPM5IFG_MASK<<PMM_PMMIFG_PMMLPM5IFG_SHIFT)

// Power Management Module Reset and Interrupt Enable Register
#define PMM_PMMRIE_RESET (0x1100U)

#define PMM_PMMRIE_SVSMLFLYIE_SHIFT (0U)
#define PMM_PMMRIE_SVMLIE_SHIFT     (1U)
#define PMM_PMMRIE_SVMLVRIE_SHIFT   (2U)
#define PMM_PMMRIE_SVSMHDLYIE_SHIFT (4U)
#define PMM_PMMRIE_SVMHIE_SHIFT     (5U)
#define PMM_PMMRIE_SVMHVLRIE_SHIFT  (6U)
#define PMM_PMMRIE_SVSLPE_SHIFT     (8U)
#define PMM_PMMRIE_SVMLVLRPE_SHIFT  (9U)
#define PMM_PMMRIE_SVSHPE_SHIFT     (12U)
#define PMM_PMMRIE_SVMHVLRPE_SHIFT  (13U)

#define PMM_PMMRIE_SVSMLFLYIE_MASK (1U)
#define PMM_PMMRIE_SVMLIE_MASK     (1U)
#define PMM_PMMRIE_SVMLVRIE_MASK   (1U)
#define PMM_PMMRIE_SVSMHDLYIE_MASK (1U)
#define PMM_PMMRIE_SVMHIE_MASK     (1U)
#define PMM_PMMRIE_SVMHVLRIE_MASK  (1U)
#define PMM_PMMRIE_SVSLPE_MASK     (1U)
#define PMM_PMMRIE_SVMLVLRPE_MASK  (1U)
#define PMM_PMMRIE_SVSHPE_MASK     (1U)
#define PMM_PMMRIE_SVMHVLRPE_MASK  (1U)

#define PMM_PMMRIE_SVSMLFLYIE (PMM_PMMRIE_SVSMLFLYIE_MASK<<PMM_PMMRIE_SVSMLFLYIE_SHIFT)
#define PMM_PMMRIE_SVMLIE     (PMM_PMMRIE_SVMLIE_MASK<<PMM_PMMRIE_SVMLIE_SHIFT)
#define PMM_PMMRIE_SVMLVRIE   (PMM_PMMRIE_SVMLVRIE_MASK<<PMM_PMMRIE_SVMLVRIE_SHIFT)
#define PMM_PMMRIE_SVSMHDLYIE (PMM_PMMRIE_SVSMHDLYIE_MASK<<PMM_PMMRIE_SVSMHDLYIE_SHIFT)
#define PMM_PMMRIE_SVMHIE     (PMM_PMMRIE_SVMHIE_MASK<<PMM_PMMRIE_SVMHIE_SHIFT)
#define PMM_PMMRIE_SVMHVLRIE  (PMM_PMMRIE_SVMHVLRIE_MASK<<PMM_PMMRIE_SVMHVLRIE_SHIFT)
#define PMM_PMMRIE_SVSLPE     (PMM_PMMRIE_SVSLPE_MASK<<PMM_PMMRIE_SVSLPE_SHIFT)
#define PMM_PMMRIE_SVMLVLRPE  (PMM_PMMRIE_SVMLVLRPE_MASK<<PMM_PMMRIE_SVMLVLRPE_SHIFT)
#define PMM_PMMRIE_SVSHPE     (PMM_PMMRIE_SVSHPE_MASK<<PMM_PMMRIE_SVSHPE_SHIFT)
#define PMM_PMMRIE_SVMHVLRPE  (PMM_PMMRIE_SVMHVLRPE_MASK<<PMM_PMMRIE_SVMHVLRPE_SHIFT)

// Power Mode 5 Control Register 0 (PM5CTL0)
#define PMM_PM5CTL0_RESET (0x0000U)

#define PMM_PM5CTL0_LOCKLPM5_SHIFT (0U)

#define PMM_PM5CTL0_LOCKLPM5_MASK (1U)

#define PMM_PM5CTL0_LOCKPM5 (PMM_PM5CTL0_LOCKPM5_MASK<<PMM_PM5CTL0_LOCKPM5_SHIFT)

void PMM_reset(PMM_Registers_Struct * PMMx);

void PMM_writePmmCoreV(PMM_Registers_Struct * PMMx, PMM_PmmCoreV_Enum val);

#endif /* HAL_HEADERS_PMM_HAL_H_ */
