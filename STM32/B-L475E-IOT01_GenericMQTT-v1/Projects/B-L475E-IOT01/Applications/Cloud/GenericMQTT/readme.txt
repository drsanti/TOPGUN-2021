/**
  @page Generic MQTT Cloud application

  @verbatim
  ******************************************************************************
  * @file    readme.txt
  * @author  MCD Application Team
  * @brief   Description of the Generic MQTT Cloud application.
  ******************************************************************************
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  @endverbatim

@par Application Description

The GenericMQTTXcubeSample application implements an MQTT client which connects to an MQTT broker
in order to publish telemetry data and receive parameter updates or commands from the cloud.

The application connects to the broker thanks to the options and credentials provided by the user
on the console.
  * The device authentication through MQTT login/password is supported.
  * The TLS encryption and authentication is supported. Several authentication levels are proposed.

The list of available configuration options is detailed in the User Manual UM2347: X-CUBE-CLD-GEN.


It supports the B-L475E-IOT01, 32F413H-DISCO, 32F769I-DISCO and 32L496G-Discovery
boards and connects to the Internet through the on-board or attached network interface.


@par Hardware and Software environment

  - MCU board: B-L475E-IOT01 (MB1297 rev D), with FW "Inventek eS-WiFi ISM43362-M3G-L44-SPI C3.5.2.3.BETA9"
      Note: the FW version is displayed on the board console at boot time.

  - WiFi access point.
      * With a transparent Internet connectivity: No proxy, no firewall blocking the outgoing traffic.
      * Running a DHCP server delivering the IP and DNS configuration to the board.


  - A development PC for building the application, programming through ST-Link, and running the virtual console.

  - A computer allowed to freely connect to the Internet through the port 1883.

      It may be your development PC, a virtual private server, a single-board computer...

      The quickest way to get started is to use a clear connection to a public server like test.mosquitto.org,
      without any authentication.
      The only test dependency in this case is an MQTT client like Mosquitto, or even an MQTT client
      application on your smartphone.

      For other configuration options, connection security, web dashboards,
      or for using MQTT cloud services like Litmus Loop or Ubidots,
      please refer to UM2347: X-CUBE-CLD-GEN.


@par How to use it ?

In order to make the program work, you must follow these steps:

Application build and flash
  - Open and build the project with one of the supported development toolchains (see the release note
    for detailed information about the version requirements).

  - Program the firmware on the STM32 board: you can copy (or drag and drop) the generated ELF
    file to the USB mass storage location created when you plug the STM32
    board to your PC. If the host is a Linux PC, the STM32 device can be found in
    the /media folder with the name e.g. "DIS_L4IOT". For example, if the created mass
    storage location is "/media/DIS_L4IOT", then the command to program the board
    with a binary file named "my_firmware.bin" is simply: cp my_firmware.bin
    /media/DIS_L4IOT.

   Alternatively, you can program the STM32 board directly through one of the
   supported development toolchains.


Application first launch

  - Connect the board to your development PC through USB (ST-LINK USB port).
    Open the console through serial terminal emulator (e.g. TeraTerm), select the ST-LINK COM port of your
    board and configure it with:
    - 8N1, 115200 bauds, no HW flow control;
    - set the line endings to LF or CR-LF (Transmit) and LF (receive).

  - On the console:
    - Enter your Wifi SSID, encryption mode and password.

    - Set the device connection string, without including the enclosing quotes (") or leading/trailing spaces.

      Sample for connecting to the public Mosquitto test server:
      HostName=test.mosquitto.org;HostPort=1883;ConnSecurity=0;MQClientId=my_unique_ID;

      Important: Choose a unique MQClientId value for your MQTT client.
                 For instance you can derive it from the MAC address of your board: c47f510111a7_is_great.

    - Set the TLS root CA certificate:

      Copy/paste the contents Projects/Common/GenericMQTT/Comodo.crt

      The device uses it to authenticate through TLS the HTTPS server which is used to retrieve the current time and date at boot time.


     - After the parameters are configured, it is possible to change them by restarting the board
     and pushing the User button (blue button) when prompted.


Application runtime
  - Makes an HTTPS request to retrieve the current time and date, and configures the RTC.

      Note: HTTPS has the advantage over NTP that the server can be authenticated by the board, preventing
            a possible man-in-the-middle attack.
            However, the first time the board is switched on (and each time it is powered down and up, if the RTC
            is not backed up), the verification of the server certificate will fail as its validity period will
            not match the RTC value.
            The following log will be printed to the console: "x509_verify_cert() returned -9984 (-0x2700)"
            This error is ignored, and the RTC is updated from the "Date:" field of the HTTP response header.

  - Connects to the MQTT broker.

  - Publishes the status of the device on the topic "/devices/<MQClientId>/status".

  - Stays idle, pending on local user, or cloud-initiated events.

    - Possible local user actions:

      - Single push on the user button:
            Trigs a sampling of the sensor values and their publication to the broker through an MQTT publish message
            on the "/sensors/<MQClientId>" telemetry topic.


      - Double push on the user button:
            Starts or stops the sensor values publication loop.
            When the loop is running, the sensor values are published every TelemetryInterval seconds.

      Note: Each sensor values publication is signaled by the user LED blinking quickly for half a second.

    - Possible cloud-initiated events trigged by publishing specific messages to the topic "/devices/<CliendId>/control".

      - LED control command:
        publish the payload {"LedOn": true} or {"LedOn": false}

      - Change of the Telemetry publication interval:
        publish the payload {"TelemetryInterval": <number of seconds>}

      - Reboot command:
        publish the payload {"Reboot": true}


    For instance, use the Mosquitto command lines below on your computer in order to follow the status
    and telemetry data of your device, and to send commands to it:

        To send new device attributes values or commands:
                                                mosquitto_pub -h test.mosquitto.org -t "/devices/<CliendId>/control" -m '<command>'

        Accepted commands:
          {"LedOn": true }
          {"LedOn": false }
          {"TelemetryInterval": <number of seconds> }
          {"Reboot": true }

      To receive the device attributes changes: mosquitto_sub -v -h test.mosquitto.org -t "/devices/<MQClientId>/status"
      To receive the telemetry data:            mosquitto_sub -v -h test.mosquitto.org -t "/sensors/<MQClientId>"


@par Directory contents

---> in .
Inc
  es_wifi_conf.h                     Es_Wifi configuration.
  es_wifi_io.h                       Functions prototypes for es_wifi IO operations.
  flash.h                            Management of the internal flash memory.
  main.h                             Header containing config parameters for the application and globals.
  stm32l4xx_hal_conf.h               HAL configuration file.
  stm32l4xx_it.h                     STM32 interrupt handlers header file.
  vl53l0x_platform.h                 vl53l0x proximity sensor platform include file.
  vl53l0x_proximity.h                vl53l0x proximity sensor include file.
  wifi.h                             Wifi core resources definitions.

Src
  es_wifi_io.c                       Es_Wifi IO porting.
  flash_l4.c                         Flash programming interface.
  main.c                             Main application file.
  stm32l4xx_hal_msp.c                Specific initializations.
  stm32l4xx_it.c                     STM32 interrupt handlers.
  system_stm32l4xx.c                 System initialization.
  vl53l0x_platform.c                 vl53l0x proximity sensor platform file.
  vl53l0x_proximity.c                vl53l0x proximity sensor.
  wifi.c                             WiFi-LL interface.

---> in Projects/Common/GenericMQTT
Comodo.crt                            Root CA certificate to be pasted on the board console at first launch.
flow.nodered                          Example Node-RED flow implementing a web UI control panel for device running the GenericMQTT application.
                                      See the details in UM2347.

Inc
  GenericMQTTXCubeSample.h            Application header.
  genmqtt_mbedtls_config.h            Application-specific mbedTLS middleware configuration.
  paho_mqtt_platform.h                Application-specific Paho middleware configuration.

Src
  GenericMQTTXcubeSample.c            Application implementation.

---> in Projects/Common/Shared
Inc
  cloud.h                             Header for cloud.c
  heap.h                              Header for heap.c
  http_util.h                         Header for http_util.h
  iot_flash_config.h                  Header for iot_flash_config.c
  mbedtls_net.h                       Header for mbedtls_net.c
  msg.h                               Trace message interface
  net.h                               Net abstraction interface at transport layer level
  net_internal.h                      Net internal definitions
  paho_timer.h                        Header for paho_timer.c 
  rfu.h                               Header for rfu.c
  sensors_data.h                      Header for sensor_data.c
  timedate.h                          Header for timedate.c
  timingSystem.h                      Header for timingSystem.c
  version.h                           STM32 X-Cube package version definition

Src
  cloud.c
  entropy_hardware_poll.c             RNG entropy source for mbedTLS.
  heap.c                              Heap check functions
  http_util.c                         Helpers for building HTTP requests, and downloading by chunks.
  iot_flash_config.c                  Dialog and storage management utils for the user-configured settings.
  mbedtls_net.c                       Network adapter for mbedTLS on NET.
  net.c                               Network socket API.
  net_tcp_wifi.c                      NET TCP / WiFi-LL implementation.
  net_tls_mbedtls.c                   NET TLS / mbedTLS implementation.
  paho_timer.c                        Paho middleware timer wrapper.
  rfu.c                               Firmware versioning and change management.
  sensors_data.c                      Board-specific file to retrieve and format sensors data.
  STM32CubeRTCInterface.c             Libc time porting to the RTC.
  timedate.c                          Initialization of the RTC from the network.
  timingSystem.c                      Libc time porting to the RTC.
  wifi_net.c                          WiFi_LL init/deinit functions.

@Par Target-specific notes

@par Caveats

  - The mbedTLS configuration parameter MBEDTLS_SSL_MAX_CONTENT_LEN is tailored down to 5kbytes.
    It commands the size of the TLS read buffer and of the TLS write buffer.
    It is sufficient for connecting to most MQTT brokers, and to the HTTPS server used for retrieving
    the time and date at boot time.
    But the TLS standard may require up to 16kbytes, depending on the server configuration.
    For instance, if the server certificate is 7kbytes large, it will not fit in the device 5kbytes buffer,
    the TLS handshake will fail, and the TLS connection will not be possible.

  - Beware the leading and trailing characters when entering the device connection string on the console.
    The string ends with the ';' separator.

@par Troubleshooting


  - MQTT subscribe callback not called while the broker has sent the acknowledge message?
      * The message size may be larger than the  MQTT max received msg size (MQTT_READ_BUFFER_SIZE).

  - Hardfault during the mbedTLS initialization
    * Check the contents of the root CA and device certificates and key configured by the user.

  - mbedTLS handshake failure
    * on mbedtls_pk_sign()
      - Undetected heap overflow.
    * On first recv()
      - Disconnection from the remote host because no common cipher was found (check the log of the server).

    * After sending the client hello, and before receiving the server hello
      - May happen if the hostname configured by the application through the "tls_server_name" socket option
        (and mbedtls_ssl_set_hostname()) does not match the configuration of the remote host.
        Typically, a remote HTTP server running several HTTP virtual hosts on the same port needs this Server Name Indication (SNI)
        field to select the TLS certificate to send to the client.

    * On server hello parsing
      - The server certificate may be larger than the mbedTLS read buffer (see @Caveats above).

  - mbedTLS read failure
    * A "bad message length" error message may signal that the received TLS chunk does not fit in the mbedTLS
      read buffer (see @Caveats above).


 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */

