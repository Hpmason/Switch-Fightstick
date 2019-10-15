/*
Nintendo Switch Fightstick - Proof-of-Concept

Based on the LUFA library's Low-Level Joystick Demo
	(C) Dean Camera
Based on the HORI's Pokken Tournament Pro Pad design
	(C) HORI

This project implements a modified version of HORI's Pokken Tournament Pro Pad
USB descriptors to allow for the creation of custom controllers for the
Nintendo Switch. This also works to a limited degree on the PS3.

Since System Update v3.0.0, the Nintendo Switch recognizes the Pokken
Tournament Pro Pad as a Pro Controller. Physical design limitations prevent
the Pokken Controller from functioning at the same level as the Pro
Controller. However, by default most of the descriptors are there, with the
exception of Home and Capture. Descriptor modification allows us to unlock
these buttons for our use.
*/

/** \file
 *
 *  Main source file for the posts printer demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "Joystick.h"
#include "uart.h"

//Define Serial Baudrate
#define BAUD 38400
#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))
// Main entry point.
int main(void) {
	// We'll start by performing hardware and peripheral setup.
	SetupHardware();
	// We'll then enable global interrupts for our use.
	GlobalInterruptEnable();
	// Once that's done, we'll enter an infinite loop.


	uart_init(BAUD);

	for (;;)
	{
		// We need to run our task to process and deliver data for our IN and OUT endpoints.
		HID_Task();
		// We also need to run the main USB management task.
		USB_USBTask();
	}
}

// Configures hardware and peripherals, such as the USB peripherals.
void SetupHardware(void) {
	// We need to disable watchdog if enabled by bootloader/fuses.
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	// We need to disable clock division before initializing the USB hardware.
	clock_prescale_set(clock_div_1);
	// We can then initialize our hardware and peripherals, including the USB stack.
	CPU_PRESCALE(0);
	// The USB stack should be initialized last.
	USB_Init();
}

// Fired to indicate that the device is enumerating.
void EVENT_USB_Device_Connect(void) {
	// We can indicate that we're enumerating here (via status LEDs, sound, etc.).
}

// Fired to indicate that the device is no longer connected to a host.
void EVENT_USB_Device_Disconnect(void) {
	// We can indicate that our device is not ready (via status LEDs, sound, etc.).
}

// Fired when the host set the current configuration of the USB device after enumeration.
void EVENT_USB_Device_ConfigurationChanged(void) {
	bool ConfigSuccess = true;

	// We setup the HID report endpoints.
	ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_OUT_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_IN_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);

	// We can read ConfigSuccess to indicate a success or failure at this point.
}

// Process control requests sent to the device from the USB host.
void EVENT_USB_Device_ControlRequest(void) {
	// We can handle two control requests: a GetReport and a SetReport.

	// Not used here, it looks like we don't receive control request from the Switch.
}

// Process and deliver data from IN and OUT endpoints.
void HID_Task(void) {
	// If the device isn't connected and properly configured, we can't do anything here.
	if (USB_DeviceState != DEVICE_STATE_Configured)
		return;

	// We'll start with the OUT endpoint.
	Endpoint_SelectEndpoint(JOYSTICK_OUT_EPADDR);
	// We'll check to see if we received something on the OUT endpoint.
	if (Endpoint_IsOUTReceived())
	{
		// If we did, and the packet has data, we'll react to it.
		if (Endpoint_IsReadWriteAllowed())
		{
			// We'll create a place to store our data received from the host.
			USB_JoystickReport_Output_t JoystickOutputData;
			// We'll then take in that data, setting it up in our storage.
			while(Endpoint_Read_Stream_LE(&JoystickOutputData, sizeof(JoystickOutputData), NULL) != ENDPOINT_RWSTREAM_NoError);
			// At this point, we can react to this data.

			// However, since we're not doing anything with this data, we abandon it.
		}
		// Regardless of whether we reacted to the data, we acknowledge an OUT packet on this endpoint.
		Endpoint_ClearOUT();
	}

	// We'll then move on to the IN endpoint.
	Endpoint_SelectEndpoint(JOYSTICK_IN_EPADDR);
	// We first check to see if the host is ready to accept data.
	if (Endpoint_IsINReady())
	{
		// We'll create an empty report.
		USB_JoystickReport_Input_t JoystickInputData;
		// We'll then populate this report with what we want to send to the host.
		GetNextReport(&JoystickInputData);
		// Once populated, we can output this data to the host. We do this by first writing the data to the control stream.
		while(Endpoint_Write_Stream_LE(&JoystickInputData, sizeof(JoystickInputData), NULL) != ENDPOINT_RWSTREAM_NoError);
		// We then send an IN packet on this endpoint.
		Endpoint_ClearIN();
	}
}


#define DATA_LENGTH 7
#define EXTERNAL_INPUT 0
#define SYNC_CONTROLLER 1
#define LED_ON (PORTD |= (1<<6))
#define LED_OFF (PORTD &= ~(1<<6))
char state = EXTERNAL_INPUT;

USB_JoystickReport_Input_t last_report;

int report_count = 0;


// Prepare the next report for the host.
void GetNextReport(USB_JoystickReport_Input_t* const ReportData) {
	// Prepare an empty report
	memset(ReportData, 0, sizeof(USB_JoystickReport_Input_t));
	// Set stick and HAT values to default values
	ReportData->LX = STICK_CENTER;
	ReportData->LY = STICK_CENTER;
	ReportData->RX = STICK_CENTER;
	ReportData->RY = STICK_CENTER;
	ReportData->HAT = HAT_CENTER;

	// When device first starts, run controller sync procedure
	if(state == SYNC_CONTROLLER) {
		if(report_count > 100) {
			report_count = 0;
			state = EXTERNAL_INPUT;
		}
		else if(report_count == 25 || report_count == 50) {
			ReportData->Button |= SWITCH_L | SWITCH_R;
		}
		else if(report_count == 75 || report_count == 100){
			ReportData->Button |= SWITCH_A;
		}
		report_count++;

		_delay_ms(1000/30);
	} else { 
		// After syncing procedure, use last report and modify it based on serial data
		memcpy(ReportData, &last_report, sizeof(USB_JoystickReport_Input_t));
		// If number of bytes received is greater than or equal to payload size, update Report with data
		if(uart_available() >= DATA_LENGTH) {
			LED_ON;
			// loop through received bytes
			uint8_t c;
			for(int i = 0; i < DATA_LENGTH - 1; i++){
				// Get byte from uart buffer
				c = uart_getchar();
				//uart_putchar(c);
				//Depending on index of the byte, update that part of the report
				switch(i) {
					case 0:
	   					ReportData->LX = c;
						break;
					case 1:
			 			ReportData->LY = c;
			 			break;
					case 2:
			 			ReportData->RX = c;
			   			break;
					case 3:
						ReportData->RY = c;
						break;
					case 4:
						ReportData->HAT = c;
						break;
					case 5: ; // We read both bytes for the 16bit button value in the same cycle
						uint16_t c2 = uart_getchar();
						uart_putchar(c2);
						ReportData->Button = (c2 << 8) | c;
						break;
				}
			}
		} else{
			LED_OFF;
		}
		//_delay_ms(1000/30);
	}
	memcpy(&last_report, ReportData, sizeof(USB_JoystickReport_Input_t));
}
