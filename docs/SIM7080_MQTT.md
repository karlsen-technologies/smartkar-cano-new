## AT Commands for MQTT(S)

The following table outlines the commands used to configure and manage MQTT and MQTTS sessions.

| Command | Description |
| --- | --- |
| `AT+CSSLCFG` | Configure SSL parameters of a context identifier 

 |
| `AT+SMCONF` | Set MQTT Parameter 

 |
| `AT+SMSSL` | Select SSL Configure 

 |
| `AT+SMCONN` | MQTT Connection 

 |
| `AT+SMPUB` | Send Packet 

 |
| `AT+SMSUB` | Subscribe Packet 

 |
| `AT+SMUNSUB` | Unsubscribe Packet 

 |
| `AT+SMSTATE` | Inquire MQTT Connection Status 

 |
| `AT+SMPUBHEX` | Set SMPUB Data Format to Hex 

 |
| `AT+SMDISC` | Disconnection MQTT 

 |
| `AT+SMALIAUTH` | Set Alibaba Cloud Parameter (One device One Secret) 

 |
| `AT+SMALIDYNA` | Set Alibaba Cloud Dynamic Register Parameter (One Product One Secret) 

 |
| `+SMSUB` | MQTT Receive Subscribe Data (URC) 

 |

---

## MQTT Function Example

This example demonstrates a standard MQTT session, including network activation, server connection, subscribing to a topic, and publishing a message.

### 1. Network Activation

```bash
// Open wireless connection (PDP Index 0, Action 1 = Active)
AT+CNACT=0,1
OK
+APP PDP: 0,ACTIVE

// Verify local IP address
AT+CNACT?
+CNACT: 0,1,"10.94.36.44"
OK

```



### 2. MQTT Configuration & Connection

```bash
// Set up server URL and Port
AT+SMCONF="URL",117.131.85.139,6000
OK

// Set Keep-alive time (seconds)
AT+SMCONF="KEEPTIME",60
OK

// Enable Clean Session
AT+SMCONF="CLEANSS",1
OK

// Set Client ID
AT+SMCONF="CLIENTID","simmqtt"
OK

// Establish Connection
AT+SMCONN
OK

```



### 3. Subscribe and Publish

```bash
// Subscribe to the topic "information" with QoS 1
AT+SMSUB="information",1
OK

// Publish "hello" to the topic "information" (length 5, QoS 1, Retain 1)
AT+SMPUB="information",5,1,1
>hello
OK

// Receive data from the subscribed topic
+SMSUB: "information","hello"

```



### 4. Termination

```bash
// Unsubscribe from topic
AT+SMUNSUB="information"
OK

// Disconnect from MQTT Server
AT+SMDISC
OK

// Deactivate wireless connection
AT+CNACT=0,0
OK
+APP PDP: 0,DEACTIVE

```