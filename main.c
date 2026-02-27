/******************************************************************************
* File Name: main.c
*
* Description: This is the source code for the MCWDT Interrupt Application Example
*              for ModusToolbox.
*
* Related Document: See README.md
*
*******************************************************************************
 * (c) 2024-2026, Infineon Technologies AG, or an affiliate of Infineon
 * Technologies AG. All rights reserved.
 * This software, associated documentation and materials ("Software") is
 * owned by Infineon Technologies AG or one of its affiliates ("Infineon")
 * and is protected by and subject to worldwide patent protection, worldwide
 * copyright laws, and international treaty provisions. Therefore, you may use
 * this Software only as provided in the license agreement accompanying the
 * software package from which you obtained this Software. If no license
 * agreement applies, then any use, reproduction, modification, translation, or
 * compilation of this Software is prohibited without the express written
 * permission of Infineon.
 *
 * Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
 * IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
 * THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
 * SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
 * Infineon reserves the right to make changes to the Software without notice.
 * You are responsible for properly designing, programming, and testing the
 * functionality and safety of your intended application of the Software, as
 * well as complying with any legal requirements related to its use. Infineon
 * does not guarantee that the Software will be free from intrusion, data theft
 * or loss, or other breaches ("Security Breaches"), and Infineon shall have
 * no liability arising out of any Security Breaches. Unless otherwise
 * explicitly approved by Infineon, the Software may not be used in any
 * application where a failure of the Product or any consequences of the use
 * thereof can reasonably be expected to result in personal injury.
*******************************************************************************/
#include "cy_pdl.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "mtb_hal.h"



/*******************************************************************************
* Global Variables
********************************************************************************/
/* MCWDT_0 interrupt configuration structure */
cy_stc_sysint_t mcwdt_irq_cfg =
{
    .intrSrc = ((NvicMux3_IRQn << 16) | srss_interrupt_mcwdt_0_IRQn),
    .intrPriority = 2UL
};

/* For the Retarget -IO (Debug UART) usage */
static cy_stc_scb_uart_context_t    UART_context;          /** UART context */
static mtb_hal_uart_t               UART_hal_obj;          /** Debug UART HAL object */

/*******************************************************************************
* Function Prototypes
********************************************************************************/
void handle_error(void);
void ISR_MCWDT_0(void);



/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function。
*
* Parameters:
*  none
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;
    cy_en_mcwdt_status_t mcwdt_init_status = CY_MCWDT_SUCCESS;

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;

    /* BSP initialization failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    /* Debug UART init */
    result = (cy_rslt_t)Cy_SCB_UART_Init(UART_HW, &UART_config, &UART_context);

    /* UART init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    Cy_SCB_UART_Enable(UART_HW);

    /* Setup the HAL UART */
    result = mtb_hal_uart_setup(&UART_hal_obj, &UART_hal_config, &UART_context, NULL);

    /* HAL UART init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    result = cy_retarget_io_init(&UART_hal_obj);

    /* HAL retarget_io init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize the User LED1 */
    result = Cy_GPIO_Pin_Init(CYBSP_USER_LED1_PORT, CYBSP_USER_LED1_PIN, &CYBSP_USER_LED1_config);

    /* GPIO init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize the User LED2 */
    result = Cy_GPIO_Pin_Init(CYBSP_USER_LED2_PORT, CYBSP_USER_LED2_PIN, &CYBSP_USER_LED2_config);

    /* GPIO init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize the User LED3 */
    result = Cy_GPIO_Pin_Init(CYBSP_USER_LED3_PORT, CYBSP_USER_LED3_PIN, &CYBSP_USER_LED3_config);

    /* GPIO init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize the MCWDT_0 */
    mcwdt_init_status = Cy_MCWDT_Init(MCWDT_0_HW, &MCWDT_0_config);

    if(mcwdt_init_status!=CY_MCWDT_SUCCESS)
    {
        handle_error();
    }

    /* Sets up the interrupt handler */
    Cy_SysInt_Init(&mcwdt_irq_cfg, ISR_MCWDT_0);

    /* Enable the MCWDT interrupt in NVIC */
    NVIC_EnableIRQ((IRQn_Type) NvicMux3_IRQn);

    /* Enable the MCWDT_0 counters */
    Cy_MCWDT_Unlock(MCWDT_0_HW);
    Cy_MCWDT_SetInterruptMask(MCWDT_0_HW, CY_MCWDT_CTR_Msk);
    Cy_MCWDT_Enable(MCWDT_0_HW, CY_MCWDT_CTR_Msk,
                    0u);
    Cy_MCWDT_Lock(MCWDT_0_HW);

    /* Print a message on UART */
    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");

    printf("*************** "
            "T2G MCU: Multi-Counter Watchdog Timer Example "
            "*************** \r\n\n");

    printf("\r\nMCWDT initialization is complete. USER LED blinking \r\n");

    for(;;)
    {

    }
}

/*******************************************************************************
* Function Name: ISR_MCWDT_0
********************************************************************************
* Summary:
*  MCWDT interrupt handler .
*
* Parameters:
*  None
*
* Return:
*  None
*******************************************************************************/
void ISR_MCWDT_0(void)
{
    uint32_t masked;
    masked = Cy_MCWDT_GetInterruptStatusMasked(MCWDT_0_HW);

    if(MCWDT_INTR_MASKED_CTR0_INT_Msk & masked)
    {
        Cy_GPIO_Inv(CYBSP_USER_LED1_PORT, CYBSP_USER_LED1_PIN);
    }
    if(MCWDT_INTR_MASKED_CTR1_INT_Msk & masked)
    {
        Cy_GPIO_Inv(CYBSP_USER_LED2_PORT, CYBSP_USER_LED2_PIN);
    }
    if(MCWDT_INTR_MASKED_CTR2_INT_Msk & masked)
    {
        Cy_GPIO_Inv(CYBSP_USER_LED3_PORT, CYBSP_USER_LED3_PIN);
    }
    Cy_MCWDT_ClearInterrupt(MCWDT_0_HW, masked);

}

/*******************************************************************************
* Function Name: handle_error
********************************************************************************
* Summary:
* This function processes unrecoverable errors such as UART / MCWDT component
* initialization error. In case of such error the system will stay in
* an infinite loop of this function.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void handle_error(void)
{
     /* Disable all interrupts */
    __disable_irq();

    /* Halt the CPU */
    CY_ASSERT(0);

}

/* [] END OF FILE */
