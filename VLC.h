/**
 * \file VLC.h
 * \author Alexis Melian Segura
 * \date 09/08/19
 * \brief Program that define the VLC emitter.
 */

#ifndef VLC_H
#define VLC_H

/****************************************************************************
*                             Includes                                     *
****************************************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/atomic.h>

#ifndef __WPROGRAM_H__
  #include "WaspClasses.h"
#endif

#include "Conversions.h"


/****************************************************************************
*                             Defines                                      *
****************************************************************************/

/** Defines whether the module will be used as a VLC transmitter (1) or as a VLC receiver (0). */
#define VLC_TRANSCEIVER 1

/** Message debug */
#define DEBUG_VLC 1

/** Digital transmission Pin. */
#define TRANSMISSION_PIN 2

/** Analog pin where the voltage received from the VLC receiver circuit will be obtained. */
#define ANALOG_RECEPTION_PIN 3

/** Working frequency of the board. */
#define BOARD_FREQUENCY (14.7456e6)

/** Communication frequency. */
#define COMMUNICATION_FREQUENCY (2e3)

/** Number of samples for each bit received, to apply oversampling. */
#define NUMBER_OF_SAMPLES 4

/** Word compouse with a bit of start, eight bits of data and a bit of stop. */
/** Start b7 b6 b5 b4 b3 b2 b1 b0 Stop */
#define WORD_LENGTH 10

/** Synchronization symbol send before the data send. */
#define SYNCHRONIZE_SYMBOL 0xD5 

/** Start of the frame. */
#define START_FLAG 0x02

/** End of the frame. */
#define END_FLAG 0x03

/** ADC voltage reference */
//#define ADC_REF_1.1   /// Internal reference 1.1v
//#define ADC_REF_2.56  /// Internal reference 2.56v
#define ADC_REF_5       /// External reference 5v

/** Difference threshold in reading ADC values to determine high and low levels. */
#define DIFFERENCE_THRESHOLD 1

/** Data maximum. */
#define DATA_MAX 50

/** Pin configuration as output pin. */
#define PIN_OUT() DDRA |= ((1 << TRANSMISSION_PIN))

/** Pin setting at high level. */
#define PIN_ON() PORTA |= ((1 << TRANSMISSION_PIN))

/** Pin setting at low level. */
#define PIN_OFF() PORTA &= ~((1 << TRANSMISSION_PIN))

/** Start symbol. */
#define START_SYMBOL 0x02

/** Stop symbol. */
#define STOP_SYMBOL 0x01

/** Mask format. */
#define START_STOP_MASK  ((STOP_SYMBOL << 20) | (START_SYMBOL << 18) | STOP_SYMBOL) //STOP/START/16bits/STOP

/** Manchester syncronization symbol. */
#define SYNC_SYMBOL_MANCHESTER  (0x6665)

/****************************************************************************
*                           Enumerations                                    *
****************************************************************************/
/** Receive states enumeration */
enum receiver_state {
  WAITING_SYNCHRONIZE, /// Waiting for synchronization
  SYNCHRONIZE, /// Synchronized, waiting for start flag (START_FLAG)
  START, /// Flag of start data received
  RECEIVING /// Receiving DATA
};

class VLC{
  public:

    /****************************************************************************
    *                             Functions                                     *
    ****************************************************************************/

    /**
    * \fn VLC()
    * 
    * Class constructor.
    */
    VLC();

    /**
    * \fn ~VLC()
    * 
    * Class destructor.
    */
    ~VLC();

    /**
    * \fn void init_VLC_emitter()
    * 
    * Function responsible for the configuration and preparation for the establishment of the emision of data by VLC.
    */
    void init_VLC_emitter();
  
    /**
    * \fn void int_frame(char* frame)
    * 
    * \param Frame of data.
    * 
    * Function that initilize the frame with the parameters associated to the syncronize and the start of the data.
    */
    void int_frame(char* frame);

    /**
    * \fn void int_emitter()
    * 
    * Function that initilize the variables that are used in the emision process.
    */
    void int_emitter();
  
    /**
    * \fn void create_frame(char* data, int data_size)
    * \param Data to be sent via VLC that will be included in the frame.
    * \param Size of the data to send.
    * \return A 0 is returned if the frame was created successfully. -1 is returned if the maximum data size is exceeded or data is being sent (frame_index> = 0).
    * 
    * Function that build the frame that is send via VLC.
    */
    int create_frame(char* data, int data_size);
  
    /**
    * \fn void write_data_frame(char * data, int data_size, char * frame)
    * \param Data to be sent that will be included in the frame.
    * \param Size of the data to send.
    * \param Frame that will be sent through VLC.
    * 
    * Function that writes the data to send in the frame.
    */
    void write_data_frame(char * data, int data_size, char * frame);
  
    /**
    * \fn void start_timer()
    * 
    * Function that initilize the timer.
    */
    void start_timer();
  
    /**
    * \fn void stop_timer()
    * 
    * Function that stop the timer.
    */
    void stop_timer();

        /**
    * \fn void init_variables()
    * 
    * Function responsible for initializing the variables used in the program.
    */
    void init_variables();

   /**
    * \fn void init_ADC()
    * 
    * Function that initilize the ADC of the microprocessor.
    */
    void init_ADC();
  
    /**
    * \fn void start_ADC()
    * 
    * Function that start the ADC conversion.
    */
    void start_ADC();
  
    /**
    * \fn void read_ADC()
    * \return ADC conversion result.
    * 
    * Function that return the ADC value obtained in the conversion.
    */
    int read_ADC();
  
    /**
    * \fn void send_half_bit()
    * 
    * Function responsible for sending data through VLC.
    */
    void send_half_bit();
  
    /**
    * \fn void data_to_manchester(unsigned char data, unsigned long int * data_manchester)
    * \param Data to be converted.
    * \param Pointer where the data converted through Manchester coding is saved.
    * 
    * Function that passes the data to its equivalent in Manchester coding.
    */
    void data_to_manchester(unsigned char data, unsigned long int * data_manchester);

    /**
    * \fn void send_VLC(char * msg, int msg_size)
    * \param Pointer associated with the message to be sent through VLC.
    * \param Size of the data to send.
    * \param Size of the data fragment to send.
    * 
    * Function responsible for generating the sending of data through VLC.
    */
    void send_VLC(char * msg, int msg_size, int fragment_size);

    /**
    * \fn void VLC_send(char * msg, int msg_size)
    * \param Pointer associated with the message to be sent through LoRaWAN.
    * \param Size of the data to send.
    * 
    * Function responsible for the call to the VLC sending function in addition to the processing of the data, highlighting the possible fragmentation of these.
    */
    void VLC_send(char * msg, int msg_size);

    /**
    * \fn void init_VLC_receptor()
    * 
    * Function responsible for the configuration and preparation for the establishment of the reception of data by VLC.
    */
    void init_VLC_receptor();

    /**
    * \fn void VLC_receive()
    * 
    * Function responsible for the collection of data received through VLC.
    */
    void VLC_receive();

    /**
    * \fn void receive_VLC(char * data)
    * \param Pointer associated to the buffer where the received data is saved.
    * 
    * Function responsible for the call to the function of the collection of data received through VLC and the processing of the data received, in case there is fragmentation..
    */
    void receive_VLC(char * data);   

   /**
    * \fn int add_byte_to_buffer(char * frame_buffer, int * frame_index, int * frame_size, enum receiver_state * frame_state ,unsigned char data)
    * \param Pointer to the buffer where the received data is saved.
    * \param Pointer to data frame index.
    * \param Pointer to data frame size.
    * \param Status of data reception.
    * \param Data received.
    * \return A 0 is returned if data is being received. A 1 is returned if the entire frame has been received. A -1 is returned in case the synchronization symbol has not been detected or the maximum buffer is exceeded.
    * 
    * Function that writes the data received in the frame.
    */
    int add_byte_to_buffer(char * frame_buffer, int * frame_index, int * frame_size, enum receiver_state * frame_state ,unsigned char data);
  
    /**
    * \fn void sample_data()
    * 
    * Function responsible for sampling the data received by VLC and captured by the analog port, for subsequent conversion and decoding of the data sent by the transmitter.
    */
    void sample_data();
  
    /**
    * \fn int insert_character(char current_value, int value_period, int * time_from_last_sync, unsigned int * detected_character)
    * \param Value obtained from the comparison of the current and previous ADC reading.
    * \param Period of value detected.
    * \param Pointer to variable that define the times since the last synchronization of a received data.
    * \param Pointer associated with the detection of a new character for the treatment of this.
    * \return A 1 is returned when a new character is detected, a -1 is returned in case no character is detected.
    * 
    * Function that treats the insertion in the data buffer a new character in case it has been received.
    */
    int insert_character(char current_value, int value_period, int * time_from_last_sync, unsigned int * detected_character);
  
    /**
    * \fn int is_a_character(int time_from_last_sync, unsigned int * detected_character)
    * \param Pointer to variable that define the timse since the last synchronization of a received data.
    * \param Pointer associated with the detection of a new character for the treatment of this.
    * \A 2 is returned in case the synchronization flag is found. A 1 is returned in case a new character has been received. A 0 is returned in case the synergization is still waiting or not enough samples have been collected to determine a new character.
    * 
    * Function that checks the format of each received character, verifying the composition of this for the chaos that meets the characteristics insert it in the data buffer.
    */
    int is_a_character(int time_from_last_sync, unsigned int * detected_character);
  
  
  private:

    /****************************************************************************
    *                             Variables                                     *
    ****************************************************************************/
   
    /** Variable associated to the counter of the bits that represent each character to send. */
    unsigned char bit_counter = 0 ;

    /** Variable associated with the character to send in Manchester coding. */
    unsigned long int manchester_data ;
    
    /** Boolean variable that determines if is stop of sending VLC data or no. True -> Start/Continue with data sending. False -> Total data send. It does not continue to send.*/
    bool vlc_sending;

    /** Data size of the frame to send via VLC. */
    int vlc_size_send;
    
    /** Buffer where the frame will be saved. */
    char frame_buffer [DATA_MAX+6] ;

    /** Index in frame. */
    int frame_index = -1; // index in frame

    /** Size of the frame */
    int frame_size = -1  ;

    /** Buffer where the message will be saved. */
    char message_buffer [DATA_MAX] ;

    /** Variable that determines if a package is being received. */ 
    bool receiving;
    
    /** Variable that determines the reception of a new character. */ 
    int new_character;
    
    /** Variable used to save the received data. */ 
    unsigned char received_data;
    
    /** Variable used to save the detected character. */ 
    unsigned int detected_character;
    
    /** Variable where is saved the ADC conversion in each sampling. */ 
    int read_value;
    
    /** Variable where is saved the old ADC conversion.*/ 
    int old_read_value;
    
    /** Variable that determines the current value from the current and previous samples received.*/ 
    char current_value;
    
    /** Variable that determines the old value from the samples received. */ 
    char old_value;
    
    /*!< Variable that determines of values quantity of values readed. */ 
    int value_counter;
    
    /** Variable that defines the times since the last synchronization of a received data. */ 
    int dist_last_sync;
    
    /** State variable of Manchester decoder. */ 
    long manchester_character;
    
    /** Variable used to save the previous bit. */ 
    unsigned char last_bit;
    
    /** Variable that stores if the received value is a character */ 
    int is_a_character_value;
    
    /** Variable where the value associated to if a new character has been received is stored. */  
    int new_character_insert;
    
    /** Variable where it is stored if the synchronization flag has been detected. */ 
    int sync_character_detect;

      
  protected:
  
};

/****************************************************************************
*                             Objects                                       *
****************************************************************************/

extern VLC vlc_object;

#endif
