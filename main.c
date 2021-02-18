#include "in430.h"
#include "timerb_hal.h"
#include "ucs_hal.h"
#include "io_hal.h"
#include "pmm_hal.h"
#include "driverlib.h"

void test(void)
{
    TB_unitTest(TB0);
}

void init(void)
{
    WDT_A_hold(WDT_A_BASE);
    PMM_reset(PMM0);
    PMM_writePmmCoreV(PMM0, pmmPmmCoreV3);
    UCS_reset(UCS0);
    UCS_DcoConfig_Struct dco_config =
    { // Approximately Fdco=24MHz, Fdcodiv=6MHz
     .dismod = false,
     .dcorsel = 5,
     .flln = 183,
     .flld = ucsCtl2Flld4,
     .fllrefdiv = ucsCtl3Fllrefdiv1,
     .selref = ucsCtl3SelrefREFOCLK
    };
    UCS_configureDco(UCS0, &dco_config);
    UCS_MclkConfig_Struct mclk_config =
    {
     .selm = ucsCtl4SelmDCOCLK,
     .divm = ucsCtl5Divm1,
     .mclkreqen = true
    };
    UCS_configureMclk(UCS0, &mclk_config);
    UCS_SmclkConfig_Struct smclk_config =
    {
     .sels = ucsCtl4SelsDCOCLKDIV,
     .divs = ucsCtl5Divs1,
     .smclkreqen = true
    };
    UCS_configureSmclk(UCS0, &smclk_config);
    UCS_AclkConfig_Struct aclk_config =
    {
     .sela = ucsCtl4SelaREFOCLK,
     .diva = ucsCtl5Diva4,
     .divpa = ucsCtl5Divpa32,
     .aclkreqen = true
    };
    UCS_configureAclk(UCS0, &aclk_config);
    IO_reset(P1);
    IO_reset(P2);
    IO_reset(P3);
    IO_reset(P4);
    IO_reset(P5);
    IO_reset(P6);
    IO_reset(P7);
    IO_reset(P8);
    IO_reset(PJ);
}

int main(void)
{
    init();
    test();

    while(1);

}
