/**
 * \file emitterVLC_LoRaWAN.pde
 * \author Alexis Melian Segura
 * \date 16/08/19
 * \brief Program that define the VLC_LoRaWAN emitter.
 */

/****************************************************************************
*                              Includes                                     *
****************************************************************************/
#include "VLC.h"
#include "LoRaWAN.h"
#include "Conversions.h"
#include "Frame.h"

/****************************************************************************
*                              Defines                                      *
****************************************************************************/
#define DEBUG 1

/****************************************************************************
*                              Variables                                    *
****************************************************************************/

/** Variable associated with the receiving port. */
uint8_t* port_recived; 

/** Array where the data received through LoRaWAN is stored. */
char data [2000];

/** Size of the received data. */
int data_size_received;

/** Size of the framgment that compose the data. */
int fragment_size;

/****************************************************************************
*                              Main Program                                 *
****************************************************************************/

void setup() {
  // Serial bus initialize.
  USB.ON();
  // Transmission pin is defined as output.
  PIN_OUT();
  // The lamp is turned on.
  PIN_ON();
  // Initialization of LoRaWAN
  lorawan_object.init_lorawan();
  // Initialization of VLC
  vlc_object.init_VLC_emitter();

}


void loop(){

  // It checks if available data sent by the LoRaWAN gateway.
  if(lorawan_object.receive_lorawan(port_recived, data, &data_size_received, &fragment_size)){

    // The identifier of the destination network encapsulated in the received frame is obtained.
    uint8_t destination_network = conversions_object.char_to_uint8t(data[0],data[1]);

    // The network to which the data is directed is checked and the corresponding action is performed.
    if(destination_network & LORAWAN_NETWORK){  // LoRaWAN network.
      #if DEBUG == 1
        USB.println("LoRaWAN Network");
      #endif
      switch(conversions_object.char_to_uint8t(data[2],data[3]) & 0x7F){ // The purpose of the frame is identified.
        case BOARD_SENSOR: // Sending the associated data to a board sensors.
          #if DEBUG == 1
            USB.println("Board Sensor");
          #endif
          break;
        case EXTERNAL_SENSOR: // Sending the associated data to a external sensors.
          #if DEBUG == 1
            USB.println("External Sensor");
          #endif
          break;
        case MAC_REQUEST: // MAC request made by a Zigbee node.
          #if DEBUG == 1
            USB.println("MAC Request");
          #endif
          break;
        case MAC_ANSWER: // Answer to the MAC request made by a Zigbee node.
          #if DEBUG == 1
            USB.println("MAC Answer");
          #endif
          break;  
        case ACTUATOR: // Data associated with the interaction with actuators.
          #if DEBUG == 1
            USB.println("Actuator");
          #endif
          break;  
        case ZIGBEE_DATA: // Data directed to a Zigbee node.
          #if DEBUG == 1
            USB.println("Zigbee data");
          #endif
          break; 
        case VLC_DATA: // Data directed to a VLC network.
          #if DEBUG == 1
            USB.println("VLC data");
          #endif
          break;  
        default:
          break;  
      }
    }

    if(destination_network & VLC_NETWORK){  // VLC network.
      #if DEBUG == 1
          USB.println("VLC Network");
      #endif
     
      switch(conversions_object.char_to_uint8t(data[2],data[3]) & 0x7F){ // The purpose of the frame is identified.
        case BOARD_SENSOR: // Sending the associated data to a board sensors.
          #if DEBUG == 1
            USB.println("Board Sensor");
          #endif
          break;
        case EXTERNAL_SENSOR: // Sending the associated data to a external sensors.
          #if DEBUG == 1
            USB.println("External Sensor");
          #endif
          break;
        case MAC_REQUEST: // MAC request made by a Zigbee node.
          #if DEBUG == 1
            USB.println("MAC Request");
          #endif
          break;
        case MAC_ANSWER: // Answer to the MAC request made by a Zigbee node.
          #if DEBUG == 1
            USB.println("MAC Answer");
          #endif
          break;  
        case ACTUATOR: // Data associated with the interaction with actuators.
          #if DEBUG == 1
            USB.println("Actuator");
          #endif
          break;  
        case ZIGBEE_DATA: // Data directed to a Zigbee node.
          #if DEBUG == 1
            USB.println("Zigbee data");
          #endif
          break; 
        case VLC_DATA: // Data directed to a VLC network.
          #if DEBUG == 1
            USB.print(F("Data Received: "));
            for(int i=0; i<data_size_received; i++){
              USB.print(data[i]);
            }
            USB.println("VLC data");
          #endif
          vlc_object.send_VLC(data, data_size_received, fragment_size);
          break;  
        default:
          #if DEBUG == 1
            USB.println(F("Data has been received "));
            USB.print(F("Data Size: "));USB.println(data_size_received);
            USB.print(F("Data Received: "));
            for(int i=0; i<data_size_received; i++){
              USB.print(data[i]);
            }
            USB.println();
            USB.print(F("Fragment Size: "));USB.println(fragment_size);
          #endif
          break;        
      }
    }

    if(destination_network & ZIGBEE_NETWORK){  // Zigbee network.
      #if DEBUG == 1
        USB.println("Zigbee Network");
      #endif
      switch(conversions_object.char_to_uint8t(data[2],data[3]) & 0x7F){ // The purpose of the frame is identified.
        case BOARD_SENSOR: // Sending the associated data to a board sensors.
          #if DEBUG == 1
            USB.println("Board Sensor");
          #endif
          break;
        case EXTERNAL_SENSOR: // Sending the associated data to a external sensors.
          #if DEBUG == 1
            USB.println("External Sensor");
          #endif
          break;
        case MAC_REQUEST: // MAC request made by a Zigbee node.
          #if DEBUG == 1
            USB.println("MAC Request");
          #endif
          break;
        case MAC_ANSWER: // Answer to the MAC request made by a Zigbee node.
          #if DEBUG == 1
            USB.println("MAC Answer");
          #endif
          break;  
        case ACTUATOR: // Data associated with the interaction with actuators.
          #if DEBUG == 1
            USB.println("Actuator");
          #endif
          break;  
        case ZIGBEE_DATA: // Data directed to a Zigbee node.
          #if DEBUG == 1
            USB.println("Zigbee data");
          #endif
          break; 
        case VLC_DATA: // Data directed to a VLC network.
          #if DEBUG == 1
            USB.println("VLC data");
          #endif
          break;  
        default:
          #if DEBUG == 1
            USB.println(F("Data has been received "));
            USB.print(F("Data Size: "));USB.println(data_size_received);
            USB.print(F("Data Received: "));
            for(int i=0; i<data_size_received; i++){
              USB.print(data[i]);
            }
            USB.println();
            USB.print(F("Fragment Size: "));USB.println(fragment_size);
          #endif
          break;      
      }
    }
  }else{
    #if DEBUG == 1
      USB.println(F("No data received"));
    #endif
  }
  
  
}
