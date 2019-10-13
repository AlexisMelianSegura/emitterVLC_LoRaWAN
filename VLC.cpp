/**
 * \file VLC.cpp
 * \author Alexis Melian Segura
 * \date 09/08/19
 * \brief Program that define the VLC emitter.
 */

/****************************************************************************
*                             Includes                                     *
****************************************************************************/
#include "VLC.h"

/****************************************************************************
*                             Objects                                       *
****************************************************************************/

VLC vlc_object = VLC();

/****************************************************************************
*                           Enumerations                                    *
****************************************************************************/

enum receiver_state frame_state = WAITING_SYNCHRONIZE;

/****************************************************************************
*                             Functions                                     *
****************************************************************************/

ISR( TIMER3_COMPA_vect ){
  if (VLC_TRANSCEIVER){ // Module defined as transmitter.
    vlc_object.send_half_bit();
  }else{ // Module defined as receiver. 
    vlc_object.sample_data();
  }
}

VLC::VLC(){
  
}

VLC::~VLC(){
  
}

void VLC::start_timer() {
  // Disable all interruptions to proceed to its configuration.
  cli();
  // The interruption records are set to zero
  TCCR3A = 0;
  TCCR3B = 0;
  // The value associated with the frequency is given.
  int comparator_value;
  if (VLC_TRANSCEIVER){ // Module defined as transmitter.
    comparator_value = (BOARD_FREQUENCY/8)/COMMUNICATION_FREQUENCY;
  }else{ // Module defined as receiver. The frequency will go in relation to the oversampling capacity to be applied.
    comparator_value = ((BOARD_FREQUENCY/8)/COMMUNICATION_FREQUENCY)/NUMBER_OF_SAMPLES;
  }
  OCR3A = comparator_value;
  // The timer interrupt is activated.
  TCCR3B |= ( 1 << WGM32 );
  // The bits that indicate to the micro the type of prescaler to be used are activated.
  TCCR3B |= ( 1 << CS31 );
  // The bit indicating the microphone to be notified when it reaches the counter is activated
  TIMSK3 |= ( 1 << OCIE3A );
  // Interruptions are activated.
  sei();
}

void VLC::stop_timer() {
  // Disable all interruptions to proceed to its configuration.
  cli();
  // The records associated with the interrupt are set to zero to deactivate the timer interrupt.
  TCCR3A = 0;
  TCCR3B = 0;
   // Interruptions are activated.
  sei();
}

void VLC::send_half_bit(){ 
   if(manchester_data & 0x01){
     PIN_ON();
   }else{
     PIN_OFF();
   }
   bit_counter-- ;
   manchester_data = (manchester_data >> 1);
   if(bit_counter == 0){   
      manchester_data = 0xAAAAAAAA ;
      if(frame_index >= 0 ){
        if(frame_index < frame_size){
          data_to_manchester(frame_buffer[frame_index], &manchester_data);
          frame_index ++ ;
        }else{
          frame_index = -1 ;
          frame_size = -1 ;
        }
      }
      bit_counter = WORD_LENGTH * 2 ;
    }
}

void VLC::data_to_manchester(unsigned char data, unsigned long int * data_manchester){
  unsigned int i ;
 (*data_manchester) = 0x02 ; // STOP symbol
 (*data_manchester) = (*data_manchester) << 2 ;
  for(i = 0 ; i < 8; i ++){
    if(data & 0x80) (*data_manchester) |=  0x02  ; // data LSB first
    else (*data_manchester) |= 0x01 ;
    (*data_manchester) = (*data_manchester) << 2 ;
    data = data << 1 ; // to next bit
  }
  (*data_manchester) |= 0x01 ; //START symbol
}

void VLC::int_frame(char * frame){
  memset(frame, 0xAA, 3);
  frame[3] = SYNCHRONIZE_SYMBOL ;
  frame[4] = START_FLAG;
  frame_index = -1 ;
  frame_size = -1 ;
}

void VLC::int_emitter(){
  manchester_data = 0xFFFFFFFF ;
  bit_counter = WORD_LENGTH * 2 ;
}

int VLC::create_frame(char * data, int data_size){
  write_data_frame(data, data_size, frame_buffer);
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
    frame_index = 0 ;
    frame_size = data_size + 6 ;
  }
  return 0 ;
}

void VLC::write_data_frame(char * data, int data_size, char * frame){
  memcpy(&(frame[5]), data, data_size);
  frame[5+data_size] = END_FLAG;
}

void VLC::init_VLC_emitter(){
  // Initialization frame.
  int_frame(frame_buffer);
  // Initialization emitter variables.
  int_emitter();
}

void VLC::send_VLC(char * msg, int msg_size, int fragment_size){
  // Variable Inicialization.
  vlc_sending = true;
  if(fragment_size>0){
    vlc_size_send = msg_size;  
  }else{
    vlc_size_send = 0;
  }
  

  // The sending function is called 
  while(vlc_sending){
    if(fragment_size == 0){
      VLC_send(msg, msg_size);
      vlc_sending = false;
      #if DEBUG_VLC == 1
        USB.print("Data 1: ");
        USB.println(msg);
      #endif
    }else if(vlc_size_send <= fragment_size){
      VLC_send(msg, vlc_size_send);
      vlc_sending = false;
      #if DEBUG_VLC == 1
        USB.print("Data 2: ");
        USB.println(msg);
      #endif
    }else if(vlc_size_send > fragment_size){
      VLC_send(msg, fragment_size);
      vlc_sending = true;
      #if DEBUG_VLC == 1
        USB.print("Data 3: ");
        for(int i=0; i<fragment_size; i++){
          USB.print(msg[i]);  
        }      
        USB.println();
      #endif
      vlc_size_send -= fragment_size;
      msg = msg + fragment_size;
    }
  }

}


void VLC::VLC_send(char * msg, int msg_size){
  // Timer is started.
  start_timer();
  memcpy(message_buffer, msg, msg_size);
  create_frame(message_buffer, msg_size);
  while(frame_index != -1){
    delay(10);
  }
  // The timer is stopped.
  stop_timer();
  // The lamp is turned on.
  PIN_ON();
}

void VLC::init_VLC_receptor(){

  // Initializtion of variables
  init_variables();
  
  USB.println("Receive data: ");
  // The ADC is configured for analog readings obtained from the VLC receiver circuit.
  init_ADC();

}

void VLC::init_variables(){
  frame_index = -1; 
  frame_size = -1; 
  new_character = 0;
  detected_character = 0;
  old_read_value = 0;
  old_value = 0;
  value_counter = 0;
  manchester_character = 0;
  dist_last_sync = 0;
  is_a_character_value = 0;
  new_character_insert = 0;
  sync_character_detect = 0;
  receiving = true;
}

void VLC::init_ADC(){
  // It setup the prescaler of 128.
  ADCSRA |= (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2);

  // ADC reference voltage is set.
  #ifdef ADC_REF_1.1
    ADMUX  = (1<<REFS1);
  #endif

  #ifdef ADC_REF_2.56
    ADMUX  = (1<<REFS1) | (1<<REFS0);  
  #endif
  
  #ifdef ADC_REF_5
    ADMUX  = (1<<REFS0);  
  #endif

  // The ADC channel relationship to the pin used in the reception is established.
  ADMUX |= (ANALOG_RECEPTION_PIN & 0x07) ;

  // ADC is turned on.
  ADCSRA |= (1<<ADEN);
}

void VLC::start_ADC(){
  // The ADC convertion is started.
  bitSet (ADCSRA, ADSC) ;
}

int VLC::read_ADC(){
  // It remains on hold until the ADC completes the conversion.
  while(bit_is_set(ADCSRA, ADSC));
  return ADC ;
}


void VLC::receive_VLC(char * data_received){
  int actual_size = 0;
  do{
    VLC_receive();
    memcpy(data_received+actual_size, frame_buffer+1, frame_size);
    actual_size += strlen(data_received);
  }while(conversions_object.char_to_uint8t(frame_buffer[3],frame_buffer[4]) & 0x80 );
  
}

int VLC::add_byte_to_buffer(char * frame_buffer, int * frame_index, int * frame_size, enum receiver_state * frame_state ,unsigned char data){
  // The synchronization flag has been received.
  if(data == SYNCHRONIZE_SYMBOL){
    (*frame_index) = 0 ;
    (*frame_size) = 0 ;
    (*frame_state) = SYNCHRONIZE ;
    return 0 ;
  }
  if((*frame_state) != WAITING_SYNCHRONIZE){ // Check that the synchronization symbol has been received
  frame_buffer[*frame_index] = data ;
  (*frame_index) ++ ;
    if(data == START_FLAG){  // The flag of the beginning of the data has been received.
      (*frame_state) = START ;
       return 0 ;
    }else if(data == END_FLAG){ // The end of data flag has been received.
      (*frame_size) = (*frame_index) ;
      (*frame_index) = -1 ;
      (*frame_state) = WAITING_SYNCHRONIZE ;
       return 1 ;
    }else if((*frame_index) >= 56){ // It is checked that the maximum that the frame can occupy is not exceeded.
      (*frame_index) = -1 ;
      (*frame_size) = -1 ;
      (*frame_state) = WAITING_SYNCHRONIZE ;
      return -1 ;
    }else{
      (*frame_state) = RECEIVING ;
    }
    return 0 ;
  }
  return -1 ;
}

void VLC::sample_data(){
  // The value of the analog input pin is read and the ADC is activated again for the next conversion.
  read_value  = read_ADC();
  start_ADC();

  // The current and previous values read from the analog pin are checked to verify the value obtained.
  if((read_value - old_read_value) > DIFFERENCE_THRESHOLD){
    current_value = 1;
  }else if(old_read_value - read_value > DIFFERENCE_THRESHOLD){
    current_value = -1;
  }else{
    current_value = 0;
  }

  // The old value is updated.
  old_read_value = read_value;

  // It is checked that the new and old values are different and that the minimum established samples have been taken to determine the arrival of a new character.
  if(current_value == 0 || current_value == old_value || (current_value != old_value && value_counter < 2)){
    if( value_counter < (4 * NUMBER_OF_SAMPLES)){
      value_counter ++ ;
    }
  }else{  
      new_character = insert_character(current_value, value_counter, &(dist_last_sync), &detected_character); 
      if(dist_last_sync > (8 * NUMBER_OF_SAMPLES)){ // The variable dist_last_sync is limited to avoid overflow problems
        dist_last_sync = 32 ;
      }
        value_counter = 0 ;
    }
    // The old value is updated.
    old_value = current_value;
}

inline int VLC::insert_character(char current_value, int value_period, int * time_from_last_sync, unsigned int * detected_character){
   // The variables used are initialized.
   new_character_insert = 0;
   is_a_character_value = 0;
   sync_character_detect = 0;
   if( (manchester_character & 0x01) != current_value ){ // It checked that it is not the same value.
         if(value_period > (NUMBER_OF_SAMPLES+1)){
            last_bit = manchester_character & 0x01 ;
            manchester_character = (manchester_character << 1) | last_bit ; // The signal was steady for longer than a single symbol. 
            (*time_from_last_sync) += 1 ;
            is_a_character_value = is_a_character((*time_from_last_sync), detected_character);
            if(is_a_character_value > 0){ // A data frame with the defined format was found.
               new_character_insert = 1 ;
              (*time_from_last_sync) =  0 ;
              if(is_a_character_value > 1){
                 sync_character_detect = 1 ; // It is detected framing and sync word in manchester format.
              }
            }
         }
         // The received character is saved.
         if(current_value < 0){
          manchester_character = ( manchester_character << 1) | 0x00 ;
         }else{
          manchester_character = ( manchester_character << 1) | 0x01 ;
         }
         (*time_from_last_sync) += 1 ;
         is_a_character_value = is_a_character((*time_from_last_sync), detected_character);
         if(sync_character_detect == 0 && is_a_character_value > 0){ // If sync flag was detected, it doesn't take character detection into account
           new_character_insert = 1 ;
           (*time_from_last_sync) =  0 ;
         }
      }else{
        new_character_insert = -1;
      }
      return new_character_insert;
}

inline int VLC::is_a_character(int time_from_last_sync, unsigned int * detected_character){
  if(time_from_last_sync >= 20  || frame_state == WAITING_SYNCHRONIZE){ 
      // It is checked that the specified data have the defined format.   
      if((manchester_character & START_STOP_MASK) == (START_STOP_MASK)){
            // The received character is stored.
            (*detected_character) = (manchester_character >> 2) & 0xFFFF;
            if(frame_state == WAITING_SYNCHRONIZE){
              // It is checked if the synchronization flag has arrived.
               if((*detected_character) == SYNC_SYMBOL_MANCHESTER){
                return 2 ;
               }
            }
            return 1 ;
      // If it has been synchronized and all the data that defines a character has been received, the received data is saved.
      }else if(frame_state != WAITING_SYNCHRONIZE && time_from_last_sync == 20){
         (*detected_character)= (manchester_character >> 2) & 0xFFFF;
         return 1 ;
      }
    }
    return 0 ;
}

void VLC::VLC_receive(){
  // The ADC conversion is started.
  start_ADC();

  // Timer is started.
  start_timer();
  
  receiving = true;
  while (receiving){
    if(new_character == 1){
    received_data = 0 ;
    // The decoding of the data is carried out, taking into account the use of Manchester coding.
    for(int i = 0 ; i < 16 ; i = i + 2){
             received_data = received_data << 1 ;
             if(((detected_character >> i) & 0x03) == 0x01){
                 received_data |= 0x01 ;
             }else{
                 received_data &= ~0x01 ;
             }
    }
    new_character = 0 ;
    if((add_byte_to_buffer(frame_buffer, &frame_index, &frame_size, &frame_state,received_data)) > 0){
      frame_buffer[frame_size-1] = '\0';
      // It has finished receiving the data.
      stop_timer();
      receiving = false;
      #if DEBUG == 1
        USB.println(frame_size);
        USB.println(&(frame_buffer[1]));
      #endif
    }
  }
 }
}
