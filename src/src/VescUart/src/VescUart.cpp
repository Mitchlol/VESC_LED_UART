#include <stdint.h>
#include "VescUart.h"

VescUart::VescUart(uint32_t timeout_ms) : _TIMEOUT(timeout_ms) {
	nunchuck.valueX         = 127;
	nunchuck.valueY         = 127;
	nunchuck.lowerButton  	= false;
	nunchuck.upperButton  	= false;
}

void VescUart::setSerialPort(Stream* port)
{
	serialPort = port;
}

void VescUart::setDebugPort(Stream* port)
{
	debugPort = port;
}

int VescUart::receiveUartMessage(uint8_t * payloadReceived) {

	// Messages <= 255 starts with "2", 2nd byte is length
	// Messages > 255 starts with "3" 2nd and 3rd byte is length combined with 1st >>8 and then &0xFF

	// Makes no sense to run this function if no serialPort is defined.
	if (serialPort == NULL)
		return -1;

	uint16_t counter = 0;
	uint16_t endMessage = 256;
	bool messageRead = false;
	
	uint16_t lenPayload = 0;
	
	uint32_t timeout = millis() + _TIMEOUT; // Defining the timestamp for timeout (100ms before timeout)

	while ( millis() < timeout && messageRead == false) {

		while (serialPort->available()) {

			buffer1[counter++] = serialPort->read();

			if (counter == 2) {

				switch (buffer1[0])
				{
					case 2:
						endMessage = buffer1[1] + 5; //Payload size + 2 for sice + 3 for SRC and End.
						lenPayload = buffer1[1];
					break;

					case 3:
						// ToDo: Add Message Handling > 255 (starting with 3)
						if( debugPort != NULL ){
							debugPort->println("Message is larger than 256 bytes - not supported");
						}
					break;

					default:
						if( debugPort != NULL ){
							debugPort->println("Unvalid start bit");
						}
					break;
				}
			}

			if (counter >= sizeof(buffer1)) {
				break;
			}

			if (counter == endMessage && buffer1[endMessage - 1] == 3) {
				buffer1[endMessage] = 0;
				if (debugPort != NULL) {
					debugPort->println("End of message reached!");
				}
				messageRead = true;
				break; // Exit if end of message is reached, even if there is still more data in the buffer.
			}
		}
	}
	if(messageRead == false && debugPort != NULL ) {
		debugPort->println("Timeout");
	}
	
	bool unpacked = false;

	if (messageRead) {
		unpacked = unpackPayload(buffer1, endMessage, payloadReceived);
	}

	if (unpacked) {
		// Message was read
		return lenPayload; 
	}
	else {
		// No Message Read
		return 0;
	}
}


bool VescUart::unpackPayload(uint8_t * message, int lenMes, uint8_t * payload) {

	uint16_t crcMessage = 0;
	uint16_t crcPayload = 0;

	// Rebuild crc:
	crcMessage = message[lenMes - 3] << 8;
	crcMessage &= 0xFF00;
	crcMessage += message[lenMes - 2];

	if(debugPort!=NULL){
		debugPort->print("SRC received: "); debugPort->println(crcMessage);
	}

	// Extract payload:
	memcpy(payload, &message[2], message[1]);

	crcPayload = crc16(payload, message[1]);

	if( debugPort != NULL ){
		debugPort->print("SRC calc: "); debugPort->println(crcPayload);
	}
	
	if (crcPayload == crcMessage) {
		if( debugPort != NULL ) {
			debugPort->print("Received: "); 
			serialPrint(message, lenMes); debugPort->println();

			debugPort->print("Payload :      ");
			serialPrint(payload, message[1] - 1); debugPort->println();
		}

		return true;
	}else{
		return false;
	}
}


int VescUart::packSendPayload(uint8_t * payload, int lenPay) {

	uint16_t crcPayload = crc16(payload, lenPay);
	int count = 0;
	
	if (lenPay <= 256)
	{
		buffer1[count++] = 2;
		buffer1[count++] = lenPay;
	}
	else
	{
		buffer1[count++] = 3;
		buffer1[count++] = (uint8_t)(lenPay >> 8);
		buffer1[count++] = (uint8_t)(lenPay & 0xFF);
	}

	memcpy(buffer1 + count, payload, lenPay);
	count += lenPay;

	buffer1[count++] = (uint8_t)(crcPayload >> 8);
	buffer1[count++] = (uint8_t)(crcPayload & 0xFF);
	buffer1[count++] = 3;
	// messageSend[count] = NULL;
	
	if(debugPort!=NULL){
		debugPort->print("Package to send: "); serialPrint(buffer1, count);
	}

	// Sending package
	if( serialPort != NULL )
		serialPort->write(buffer1, count);

	// Returns number of send bytes
	return count;
}

bool VescUart::processReadPacket(uint8_t * message) {

	COMM_PACKET_ID packetId;
	int32_t index = 0;

	packetId = (COMM_PACKET_ID)message[0];
	message++; // Removes the packetId from the actual message (payload)

	switch (packetId){
		case COMM_FW_VERSION: // Structure defined here: https://github.com/vedderb/bldc/blob/43c3bbaf91f5052a35b75c2ff17b5fe99fad94d1/commands.c#L164

			fw_version.major = message[index++];
			fw_version.minor = message[index++];
			return true;
		case COMM_GET_VALUES: // Structure defined here: https://github.com/vedderb/bldc/blob/43c3bbaf91f5052a35b75c2ff17b5fe99fad94d1/commands.c#L164

			data.tempMosfet 		= buffer_get_float16(message, 10.0, &index); 	// 2 bytes - mc_interface_temp_fet_filtered()
			data.tempMotor 			= buffer_get_float16(message, 10.0, &index); 	// 2 bytes - mc_interface_temp_motor_filtered()
			data.avgMotorCurrent 	= buffer_get_float32(message, 100.0, &index); // 4 bytes - mc_interface_read_reset_avg_motor_current()
			data.avgInputCurrent 	= buffer_get_float32(message, 100.0, &index); // 4 bytes - mc_interface_read_reset_avg_input_current()
			index += 4; // Skip 4 bytes - mc_interface_read_reset_avg_id()
			index += 4; // Skip 4 bytes - mc_interface_read_reset_avg_iq()
			data.dutyCycleNow 		= buffer_get_float16(message, 1000.0, &index); 	// 2 bytes - mc_interface_get_duty_cycle_now()
			data.rpm 				= buffer_get_float32(message, 1.0, &index);		// 4 bytes - mc_interface_get_rpm()
			data.inpVoltage 		= buffer_get_float16(message, 10.0, &index);		// 2 bytes - GET_INPUT_VOLTAGE()
			data.ampHours 			= buffer_get_float32(message, 10000.0, &index);	// 4 bytes - mc_interface_get_amp_hours(false)
			data.ampHoursCharged 	= buffer_get_float32(message, 10000.0, &index);	// 4 bytes - mc_interface_get_amp_hours_charged(false)
			data.wattHours			= buffer_get_float32(message, 10000.0, &index);	// 4 bytes - mc_interface_get_watt_hours(false)
			data.wattHoursCharged	= buffer_get_float32(message, 10000.0, &index);	// 4 bytes - mc_interface_get_watt_hours_charged(false)
			data.tachometer 		= buffer_get_int32(message, &index);				// 4 bytes - mc_interface_get_tachometer_value(false)
			data.tachometerAbs 		= buffer_get_int32(message, &index);				// 4 bytes - mc_interface_get_tachometer_abs_value(false)
			data.error 				= (mc_fault_code)message[index++];								// 1 byte  - mc_interface_get_fault()
			data.pidPos				= buffer_get_float32(message, 1000000.0, &index);	// 4 bytes - mc_interface_get_pid_pos_now()
			data.id					= message[index++];								// 1 byte  - app_get_configuration()->controller_id	

			return true;
		break;

		case COMM_CUSTOM_APP_DATA:
			if(message[index++] != 101){
				return false;
			} else if(message[index] == 1){
				index++;
				floatData.pidValue = buffer_get_float32_auto(message, &index);
				floatData.pitch = buffer_get_float32_auto(message, &index);
				floatData.roll = buffer_get_float32_auto(message, &index);
				
				floatData.state = message[index++]; // uint 8
				floatData.setpointAdjustmentType = floatData.state >> 4;
				floatData.state = floatData.state & 0b00001111;
				
				floatData.switchState = message[index++]; // uint 8
				floatData.beepReason = floatData.switchState >> 4;
				floatData.switchState = floatData.switchState & 0b00001111;

				floatData.adc1 = buffer_get_float32_auto(message, &index);
				floatData.adc2 = buffer_get_float32_auto(message, &index);
				floatData.floatSetpoint = buffer_get_float32_auto(message, &index);
				floatData.floatAtr = buffer_get_float32_auto(message, &index);
				floatData.floatBraketilt = buffer_get_float32_auto(message, &index);
				floatData.floatTorquetilt = buffer_get_float32_auto(message, &index);
				floatData.floatTurntilt = buffer_get_float32_auto(message, &index);
				floatData.floatInputtilt = buffer_get_float32_auto(message, &index);
				floatData.truePitch = buffer_get_float32_auto(message, &index);
				floatData.filteredCurrent = buffer_get_float32_auto(message, &index);
				floatData.floatAccDiff = buffer_get_float32_auto(message, &index);
				floatData.appliedBoosterCurrent = buffer_get_float32_auto(message, &index);
				floatData.motorCurrent = buffer_get_float32_auto(message, &index);
				floatData.throttleVal = buffer_get_float32_auto(message, &index);
				return true;	
			} else if(message[index] == 25) {
				index++;
				floatData.led_type = message[index++]; // uint 8
                floatData.led_brightness = message[index++]; // uint 8
                floatData.led_brightness_idle = message[index++]; // uint 8
				floatData.led_status_brightness = message[index++]; // uint 8
				floatData.led_mode = message[index++]; // uint 8
                floatData.led_mode_idle = message[index++]; // uint 8
				floatData.led_status_mode = message[index++]; // uint 8
				floatData.led_status_count = message[index++]; // uint 8
                floatData.led_forward_count = message[index++]; // uint 8
                floatData.led_rear_count = message[index++]; // uint 8
			} else if(message[index] == 29) {
				index++;
				floatData.batteryPercent = buffer_get_float32_auto(message, &index);
			} else {
				return false;
			}
			
		break;

		/* case COMM_GET_VALUES_SELECTIVE:

			uint32_t mask = 0xFFFFFFFF; */

		default:
			return false;
		break;
	}
}

bool VescUart::getFWversion(void){
	return getFWversion(0);
}

bool VescUart::getFWversion(uint8_t canId){
	
	int32_t index = 0;
	int payloadSize = (canId == 0 ? 1 : 3);
	uint8_t payload[payloadSize];
	
	if (canId != 0) {
		payload[index++] = { COMM_FORWARD_CAN };
		payload[index++] = canId;
	}
	payload[index++] = { COMM_FW_VERSION };

	packSendPayload(payload, payloadSize);

	uint8_t message[256];
	int messageLength = receiveUartMessage(message);
	if (messageLength > 0) { 
		return processReadPacket(message); 
	}
	return false;
}

bool VescUart::getVescValues(void) {
	return getVescValues(0);
}

bool VescUart::getVescValues(uint8_t canId) {

	if (debugPort!=NULL){
		debugPort->println("Command: COMM_GET_VALUES "+String(canId));
	}

	int32_t index = 0;
	int payloadSize = (canId == 0 ? 1 : 3);
	uint8_t payload[payloadSize];
	if (canId != 0) {
		payload[index++] = { COMM_FORWARD_CAN };
		payload[index++] = canId;
	}
	payload[index++] = { COMM_GET_VALUES };

	packSendPayload(payload, payloadSize);

	int messageLength = receiveUartMessage(buffer2);

	if (messageLength > 55) {
		return processReadPacket(buffer2); 
	}
	return false;
}

bool VescUart::getFloatValues(void) {

	if (debugPort!=NULL){
		debugPort->println("Command: COMM_CUSTOM_APP_DATA ");
	}

	int32_t index = 0;
	int payloadSize = 3;
	uint8_t payload[payloadSize];
	payload[index++] = { COMM_CUSTOM_APP_DATA };
	payload[index++] = 101;
	payload[index++] = 0x1;

	packSendPayload(payload, payloadSize);

	int messageLength = receiveUartMessage(buffer2);

	if (messageLength > 55) {
		return processReadPacket(buffer2); 
	}
	return false;
}

bool VescUart::getFloatBattery(void) {

	if (debugPort!=NULL){
		debugPort->println("Command: COMM_CUSTOM_APP_DATA ");
	}

	int32_t index = 0;
	int payloadSize = 3;
	uint8_t payload[payloadSize];
	payload[index++] = { COMM_CUSTOM_APP_DATA };
	payload[index++] = 101;
	payload[index++] = 29; // Get battery

	packSendPayload(payload, payloadSize);

	int messageLength = receiveUartMessage(buffer2);

	if (messageLength > 2) {
		return processReadPacket(buffer2); 
	}
	return false;
}

bool VescUart::getFloatLeds(void) {

	if (debugPort!=NULL){
		debugPort->println("Command: COMM_CUSTOM_APP_DATA ");
	}

	int32_t index = 0;
	int payloadSize = 3;
	uint8_t payload[payloadSize];
	payload[index++] = { COMM_CUSTOM_APP_DATA };
	payload[index++] = 101;
	payload[index++] = 25; // Get battery

	packSendPayload(payload, payloadSize);

	int messageLength = receiveUartMessage(buffer2);

	if (messageLength > 10) {
		return processReadPacket(buffer2); 
	}
	return false;
}

void VescUart::setNunchuckValues() {
	return setNunchuckValues(0);
}

void VescUart::setNunchuckValues(uint8_t canId) {

	if(debugPort!=NULL){
		debugPort->println("Command: COMM_SET_CHUCK_DATA "+String(canId));
	}	
	int32_t index = 0;
	int payloadSize = (canId == 0 ? 11 : 13);
	uint8_t payload[payloadSize];

	if (canId != 0) {
		payload[index++] = { COMM_FORWARD_CAN };
		payload[index++] = canId;
	}
	payload[index++] = { COMM_SET_CHUCK_DATA };
	payload[index++] = nunchuck.valueX;
	payload[index++] = nunchuck.valueY;
	buffer_append_bool(payload, nunchuck.lowerButton, &index);
	buffer_append_bool(payload, nunchuck.upperButton, &index);
	
	// Acceleration Data. Not used, Int16 (2 byte)
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = 0;
	payload[index++] = 0;

	if(debugPort != NULL){
		debugPort->println("Nunchuck Values:");
		debugPort->print("x="); debugPort->print(nunchuck.valueX); debugPort->print(" y="); debugPort->print(nunchuck.valueY);
		debugPort->print(" LBTN="); debugPort->print(nunchuck.lowerButton); debugPort->print(" UBTN="); debugPort->println(nunchuck.upperButton);
	}

	packSendPayload(payload, payloadSize);
}

void VescUart::setCurrent(float current) {
	return setCurrent(current, 0);
}

void VescUart::setCurrent(float current, uint8_t canId) {
	int32_t index = 0;
	int payloadSize = (canId == 0 ? 5 : 7);
	uint8_t payload[payloadSize];
	if (canId != 0) {
		payload[index++] = { COMM_FORWARD_CAN };
		payload[index++] = canId;
	}
	payload[index++] = { COMM_SET_CURRENT };
	buffer_append_int32(payload, (int32_t)(current * 1000), &index);
	packSendPayload(payload, payloadSize);
}

void VescUart::setBrakeCurrent(float brakeCurrent) {
	return setBrakeCurrent(brakeCurrent, 0);
}

void VescUart::setBrakeCurrent(float brakeCurrent, uint8_t canId) {
	int32_t index = 0;
	int payloadSize = (canId == 0 ? 5 : 7);
	uint8_t payload[payloadSize];
	if (canId != 0) {
		payload[index++] = { COMM_FORWARD_CAN };
		payload[index++] = canId;
	}

	payload[index++] = { COMM_SET_CURRENT_BRAKE };
	buffer_append_int32(payload, (int32_t)(brakeCurrent * 1000), &index);

	packSendPayload(payload, payloadSize);
}

void VescUart::setRPM(float rpm) {
	return setRPM(rpm, 0);
}

void VescUart::setRPM(float rpm, uint8_t canId) {
	int32_t index = 0;
	int payloadSize = (canId == 0 ? 5 : 7);
	uint8_t payload[payloadSize];
	if (canId != 0) {
		payload[index++] = { COMM_FORWARD_CAN };
		payload[index++] = canId;
	}
	payload[index++] = { COMM_SET_RPM };
	buffer_append_int32(payload, (int32_t)(rpm), &index);
	packSendPayload(payload, payloadSize);
}

void VescUart::setDuty(float duty) {
	return setDuty(duty, 0);
}

void VescUart::setDuty(float duty, uint8_t canId) {
	int32_t index = 0;
	int payloadSize = (canId == 0 ? 5 : 7);
	uint8_t payload[payloadSize];
	if (canId != 0) {
		payload[index++] = { COMM_FORWARD_CAN };
		payload[index++] = canId;
	}
	payload[index++] = { COMM_SET_DUTY };
	buffer_append_int32(payload, (int32_t)(duty * 100000), &index);

	packSendPayload(payload, payloadSize);
}

void VescUart::sendKeepalive(void) {
	return sendKeepalive(0);
}

void VescUart::sendKeepalive(uint8_t canId) {
	int32_t index = 0;
	int payloadSize = (canId == 0 ? 1 : 3);
	uint8_t payload[payloadSize];
	if (canId != 0) {
		payload[index++] = { COMM_FORWARD_CAN };
		payload[index++] = canId;
	}
	payload[index++] = { COMM_ALIVE };
	packSendPayload(payload, payloadSize);
}

void VescUart::serialPrint(uint8_t * data, int len) {
	if(debugPort != NULL){
		for (int i = 0; i <= len; i++)
		{
			debugPort->print(data[i]);
			debugPort->print(" ");
		}
		debugPort->println("");
	}
}

void VescUart::printVescValues() {
	if(debugPort != NULL){
		debugPort->print("avgMotorCurrent: "); 	debugPort->println(data.avgMotorCurrent);
		debugPort->print("avgInputCurrent: "); 	debugPort->println(data.avgInputCurrent);
		debugPort->print("dutyCycleNow: "); 	debugPort->println(data.dutyCycleNow);
		debugPort->print("rpm: "); 				debugPort->println(data.rpm);
		debugPort->print("inputVoltage: "); 	debugPort->println(data.inpVoltage);
		debugPort->print("ampHours: "); 		debugPort->println(data.ampHours);
		debugPort->print("ampHoursCharged: "); 	debugPort->println(data.ampHoursCharged);
		debugPort->print("wattHours: "); 		debugPort->println(data.wattHours);
		debugPort->print("wattHoursCharged: "); debugPort->println(data.wattHoursCharged);
		debugPort->print("tachometer: "); 		debugPort->println(data.tachometer);
		debugPort->print("tachometerAbs: "); 	debugPort->println(data.tachometerAbs);
		debugPort->print("tempMosfet: "); 		debugPort->println(data.tempMosfet);
		debugPort->print("tempMotor: "); 		debugPort->println(data.tempMotor);
		debugPort->print("error: "); 			debugPort->println(data.error);
	}
}
