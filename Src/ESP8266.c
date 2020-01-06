#include "ESP8266.h"
#include "ESP8266Config.h"
//#########################################################################################################
bool Wifi_SendRaw(uint8_t *data,uint16_t len)
{
	if(len <= _WIFI_TX_SIZE)
	{
		// Send the information in data through the UART of the ESP8266
		memcpy(Wifi.TxBuffer,data,len);
		if(HAL_UART_Transmit(&_WIFI_USART,data,len,900) == HAL_OK)
			return true;
		else
			return false;
	}
	else
		return false;
}
//#########################################################################################################
bool Wifi_SendString(char *data)
{
	return Wifi_SendRaw((uint8_t*)data,strlen(data));
}
//#########################################################################################################
bool Wifi_SendStringAndWait(char *data,uint16_t DelayUs)
{
	if(Wifi_SendRaw((uint8_t*)data,strlen(data))==false)
		return false;
	DWT_Delay_us(DelayUs);
	return true;
}
//#########################################################################################################
bool Wifi_WaitForString(uint32_t TimeOut_ms,uint8_t *result,uint8_t CountOfParameter,...)
{
	/*
	 * It uses the CountOfParameter and the Parameters after that to compare with the
	 * information that it was received in the UART RX. If the parameter is in the
	 * received string the functions return a true value and the position of the
	 * parameter (according to the position in the function)
	 *
	 * Ex:
	 * Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")
	 *
	 * If the ESP8266 return a AT+OK after the last command, the function is going to
	 * return a true value and the result number would be 1.
	 */

	if(result == NULL)
		return false;
	if(CountOfParameter == 0)
		return false;

	*result=0;

	va_list tag;
		va_start (tag,CountOfParameter);
		char *arg[CountOfParameter];
		for(uint8_t i=0; i<CountOfParameter ; i++)
			arg[i] = va_arg (tag, char *);
	va_end (tag);

	for(uint32_t t=0 ; t<TimeOut_ms ; t+=20)
	{
		DWT_Delay_us(20000);
		for(uint8_t	mx=0 ; mx<CountOfParameter ; mx++)
		{
			if(strstr((char*)Wifi.RxBuffer,arg[mx])!=NULL)
			{
				*result = mx+1;
				return true;
			}
		}
	}
	// timeout
	return false;

}
//#########################################################################################################
bool Wifi_ReturnString(char *result,uint8_t WantWhichOne,char *SplitterChars)
{
	if(result == NULL)
		return false;
	if(WantWhichOne==0)
		return false;

	char *str = (char*)Wifi.RxBuffer;


	str = strtok (str,SplitterChars);
	if(str == NULL)
	{
		strcpy(result,"");
		return false;
	}
	while (str != NULL)
  {
    str = strtok (NULL,SplitterChars);
		if(str != NULL)
			WantWhichOne--;
		if(WantWhichOne==0)
		{
			strcpy(result,str);
			return true;
		}
  }
	strcpy(result,"");
	return false;
}
//#########################################################################################################
bool Wifi_ReturnStrings(char *InputString,char *SplitterChars,uint8_t CountOfParameter,...)
{
	if(CountOfParameter == 0)
		return false;
	va_list tag;
		va_start (tag,CountOfParameter);
		char *arg[CountOfParameter];
		for(uint8_t i=0; i<CountOfParameter ; i++)
			arg[i] = va_arg (tag, char *);
	va_end (tag);

	char *str;
	str = strtok (InputString,SplitterChars);
	if(str == NULL)
		return false;
	uint8_t i=0;
	while (str != NULL)
  {
    str = strtok (NULL,SplitterChars);
		if(str != NULL)
			CountOfParameter--;
		strcpy(arg[i],str);
		i++;
		if(CountOfParameter==0)
		{
			return true;
		}
  }
	return false;

}
//#########################################################################################################
bool Wifi_ReturnInteger(int32_t	*result,uint8_t WantWhichOne,char *SplitterChars)
{
	if((char*)Wifi.RxBuffer == NULL)
		return false;
	if(Wifi_ReturnString((char*)Wifi.RxBuffer,WantWhichOne,SplitterChars)==false)
		return false;
	*result = atoi((char*)Wifi.RxBuffer);
	return true;
}
//#########################################################################################################

bool Wifi_ReturnFloat(float	*result,uint8_t WantWhichOne,char *SplitterChars)
{
	if((char*)Wifi.RxBuffer == NULL)
		return false;
	if(Wifi_ReturnString((char*)Wifi.RxBuffer,WantWhichOne,SplitterChars)==false)
		return false;
	*result = atof((char*)Wifi.RxBuffer);
	return true;
}
//#########################################################################################################
void Wifi_RemoveChar(char *str, char garbage)
{
	char *src, *dst;
  for (src = dst = str; *src != '\0'; src++)
	{
		*dst = *src;
		if (*dst != garbage)
			dst++;
  }
  *dst = '\0';
}
//#########################################################################################################
void Wifi_RxClear(void)
{
	memset(Wifi.RxBuffer,0,_WIFI_RX_SIZE);
	Wifi.RxIndex=0;
	HAL_UART_Receive_IT(&_WIFI_USART,&Wifi.usartBuff,1);
}
//#########################################################################################################
void Wifi_TxClear(void)
{
	memset(Wifi.TxBuffer,0,_WIFI_TX_SIZE);
}
//#########################################################################################################
void Wifi_RxCallBack(void)
{
	Wifi.RxBuffer[Wifi.RxIndex] = Wifi.usartBuff;
	if(Wifi.RxIndex < _WIFI_RX_SIZE)
	  Wifi.RxIndex++;
	HAL_UART_Receive_IT(&_WIFI_USART,&Wifi.usartBuff,1);
}
//#########################################################################################################
//#########################################################################################################
//#########################################################################################################
//#########################################################################################################
bool Wifi_Init(void)
{
	uint8_t result;
	bool returnVal=false;
	// Clean the variables and start the interruption to work with the UART
	do
	{
		Wifi_RxClear();

		if(Wifi_SendString("AT\r\n")==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
			break;
		if(result==2)
			break;
		returnVal = true;
		Wifi_RxClear();
		Wifi_TxClear();
		HAL_UART_Receive_IT(&_WIFI_USART,&Wifi.usartBuff,1);
	}while(0);
	return returnVal;
}
//#########################################################################################################
void Wifi_Enable(void){
	// It is necessary to set HIGH both pins of the module to work correctly
	HAL_GPIO_WritePin (_BANK_WIFI_BUTTONS,_BUTTON_ENABLE, GPIO_PIN_RESET);
	HAL_GPIO_WritePin (_BANK_WIFI_BUTTONS,_BUTTON_RST, GPIO_PIN_RESET);
	DWT_Delay_us(100);
	HAL_GPIO_WritePin (_BANK_WIFI_BUTTONS,_BUTTON_ENABLE, GPIO_PIN_SET);
	HAL_GPIO_WritePin (_BANK_WIFI_BUTTONS,_BUTTON_RST, GPIO_PIN_SET);
	HAL_Delay(2000);
}
//#########################################################################################################
void Wifi_Disable(void){
	// It is necessary to set LOW both pins of the module to disable correctly
	HAL_GPIO_WritePin (_BANK_WIFI_BUTTONS,_BUTTON_ENABLE, GPIO_PIN_SET);
	HAL_GPIO_WritePin (_BANK_WIFI_BUTTONS,_BUTTON_RST, GPIO_PIN_SET);
	DWT_Delay_us(100);
	HAL_GPIO_WritePin (_BANK_WIFI_BUTTONS,_BUTTON_ENABLE, GPIO_PIN_RESET);
	HAL_GPIO_WritePin (_BANK_WIFI_BUTTONS,_BUTTON_RST, GPIO_PIN_RESET);
}
//#########################################################################################################
//#########################################################################################################
//#########################################################################################################
//#########################################################################################################
bool	Wifi_Restart(void)
{
	// Make a restart of the ESP8266 using the AT Commands

	uint8_t result;
	bool	returnVal=false;
	do
	{
		Wifi_RxClear();
		sprintf((char*)Wifi.TxBuffer,"AT+RST\r\n");
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
			break;			// The timeout was completed and the string was not there
		if(result == 2)		// It was find the "ERROR" String in the receiving information
			break;
		returnVal=true;
	}while(0);
	return returnVal;
}
//#########################################################################################################
bool	Wifi_DeepSleep(uint16_t DelayMs)
{
	// The ESP8266 enters in Deep-Sleep Mode for the time, which is indicated

	uint8_t result;
	bool	returnVal=false;
	do
	{
		Wifi_RxClear();
		sprintf((char*)Wifi.TxBuffer,"AT+GSLP=%d\r\n",DelayMs);
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
			break;			// The timeout was completed and the string was not there
		if(result == 2)		// It was find the "ERROR" String in the receiving information
			break;
		returnVal=true;
	}while(0);
	return returnVal;
}
//#########################################################################################################
bool	Wifi_FactoryReset(void)
{
	uint8_t result;
	bool	returnVal=false;
	do
	{
		Wifi_RxClear();
		sprintf((char*)Wifi.TxBuffer,"AT+RESTORE\r\n");
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
			break;			// The timeout was completed and the string was not there
		if(result == 2)		// It was find the "ERROR" String in the receiving information
			break;
		returnVal=true;
	}while(0);
	return returnVal;
}
//#########################################################################################################
bool	Wifi_Update(void)
{
	// Update the version of AT Commands when the device is connected to Internet

	uint8_t result;
	bool	returnVal=false;
	do
	{
		Wifi_RxClear();
		sprintf((char*)Wifi.TxBuffer,"AT+CIUPDATE\r\n");
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(1000*60*5,&result,2,"OK","ERROR")==false)
			break;			// The timeout was completed and the string was not there
		if(result == 2)		// It was find the "ERROR" String in the receiving information
			break;
		returnVal=true;
	}while(0);
	return returnVal;
}
//#########################################################################################################
bool	Wifi_SetRfPower(uint8_t Power_0_to_82)
{
	uint8_t result;
	bool	returnVal=false;
	do
	{
		Wifi_RxClear();
		sprintf((char*)Wifi.TxBuffer,"AT+RFPOWER=%d\r\n",Power_0_to_82);
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
			break;			// The timeout was completed and the string was not there
		if(result == 2)		// It was find the "ERROR" String in the receiving information
			break;
		returnVal=true;
	}while(0);
	return returnVal;
}
//#########################################################################################################
//#########################################################################################################
//#########################################################################################################
//#########################################################################################################
bool	Wifi_SetMode(WifiMode_t	WifiMode_)
{
	uint8_t result;
	bool	returnVal=false;
	do
	{
		Wifi_RxClear();
		sprintf((char*)Wifi.TxBuffer,"AT+CWMODE_CUR=%d\r\n",WifiMode_);
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
			break;			// The timeout was completed and the string was not there
		if(result == 2)		// It was find the "ERROR" String in the receiving information
			break;
		Wifi.Mode = WifiMode_;
		returnVal=true;
	}while(0);
	return returnVal;
}
//#########################################################################################################
bool	Wifi_GetMode(void)
{
	uint8_t result;
	bool	returnVal=false;
	do
	{
		Wifi_RxClear();
		sprintf((char*)Wifi.TxBuffer,"AT+CWMODE_CUR?\r\n");
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
			break;			// The timeout was completed and the string was not there
		if(result == 2)		// It was find the "ERROR" String in the receiving information
			break;

		if(Wifi_ReturnInteger((int32_t*)&result,1,":"))
			Wifi.Mode = (WifiMode_t)result ;
		else
			Wifi.Mode = WifiMode_Error;
		returnVal=true;
	}while(0);
	return returnVal;
}
bool	Wifi_GetMyIp(void)
{
	uint8_t result;
	bool	returnVal=false;
	do
	{
		Wifi_RxClear();
		sprintf((char*)Wifi.TxBuffer,"AT+CIFSR\r\n");
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
			break;			// The timeout was completed and the string was not there
		if(result == 2)		// It was find the "ERROR" String in the receiving information
			break;
		sscanf((char*)Wifi.RxBuffer,"AT+CIFSR\r\r\n+CIFSR:APIP,\"%[^\"]",Wifi.MyIP);
		sscanf((char*)Wifi.RxBuffer,"AT+CIFSR\r\r\n+CIFSR:STAIP,\"%[^\"]",Wifi.MyIP);
		returnVal=true;
	}while(0);
	return returnVal;
}
//#########################################################################################################
//#########################################################################################################
//#########################################################################################################
//#########################################################################################################
bool	Wifi_Station_ConnectToAp(char *SSID,char *Pass,char *MAC)
{
	uint8_t result;
	bool	returnVal=false;
	do
	{
		/*
		 * It connects to a WiFi network who has all the parameters correctly
		 */
		Wifi_RxClear();
		if(MAC==NULL)
			sprintf((char*)Wifi.TxBuffer,"AT+CWJAP_CUR=\"%s\",\"%s\"\r\n",SSID,Pass);
		else
			sprintf((char*)Wifi.TxBuffer,"AT+CWJAP_CUR=\"%s\",\"%s\",\"%s\"\r\n",SSID,Pass,MAC);
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_VERYHIGH,&result,3,"\r\nOK\r\n","\r\nERROR\r\n","\r\nFAIL\r\n")==false)
			break;
		if(result > 1)		// If the result is higher to 1 is because there were an error
			break;			// in the communication
		returnVal=true;
	}while(0);
	return returnVal;
}
//#########################################################################################################
bool	Wifi_Station_Disconnect(void)
{
	uint8_t result;
	bool		returnVal=false;
	do
	{
		Wifi_RxClear();
		sprintf((char*)Wifi.TxBuffer,"AT+CWQAP\r\n");
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
			break;			// The timeout was completed and the string was not there
		if(result == 2)		// It was find the "ERROR" String in the receiving information
			break;
		returnVal=true;
	}while(0);
	return returnVal;
}
//#########################################################################################################
bool	Wifi_Station_SetIp(char *IP,char *GateWay,char *NetMask)
{
	uint8_t result;
	bool	returnVal=false;
	do
	{
		Wifi_RxClear();
		sprintf((char*)Wifi.TxBuffer,"AT+CIPSTA_CUR=\"%s\",\"%s\",\"%s\"\r\n",IP,GateWay,NetMask);
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
			break;			// The timeout was completed and the string was not there
		if(result == 2)		// It was find the "ERROR" String in the receiving information
			break;
		Wifi.StationDhcp=false;
		returnVal=true;
	}while(0);
	return returnVal;
}
//#########################################################################################################
bool	Wifi_Station_DhcpEnable(bool Enable)
{
	uint8_t result;
	bool	returnVal=false;
	do
	{
		/*
		 * It makes the DHCP from the Station Mode enable
		 */
		Wifi_RxClear();
		sprintf((char*)Wifi.TxBuffer,"AT+CWDHCP_CUR=1,%d\r\n",Enable);
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
			break;			// The timeout was completed and the string was not there
		if(result == 2)		// It was find the "ERROR" String in the receiving information
			break;
		Wifi.StationDhcp=Enable;
		returnVal=true;
	}while(0);
	return returnVal;
}
//#########################################################################################################
bool	Wifi_Station_DhcpIsEnable(void)
{
	uint8_t result;
	bool	returnVal=false;
	do
	{
		/*
		 * It makes a question to the micro-controller to know how is the status of the
		 * DHCP process inside of it
		 */
		Wifi_RxClear();
		sprintf((char*)Wifi.TxBuffer,"AT+CWDHCP_CUR?\r\n");
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
			break;			// The timeout was completed and the string was not there
		if(result == 2)		// It was find the "ERROR" String in the receiving information
			break;
		if(Wifi_ReturnInteger((int32_t*)&result,1,":")==false)
			break;			// It searches for a ':' Character to know
							// the next integer value after that
		switch(result)
		{
			case 0:
				Wifi.StationDhcp=false;
				Wifi.SoftApDhcp=false;
			break;
			case 1:
				Wifi.StationDhcp=false;
				Wifi.SoftApDhcp=true;
			break;
			case 2:
				Wifi.StationDhcp=true;
				Wifi.SoftApDhcp=false;
			break;
			case 3:
				Wifi.StationDhcp=true;
				Wifi.SoftApDhcp=true;
			break;
		}
		returnVal=true;
	}while(0);	return returnVal;
}
//#########################################################################################################
//#########################################################################################################
//#########################################################################################################
//#########################################################################################################
bool  Wifi_SoftAp_Create(char *SSID,char *password,uint8_t channel,WifiEncryptionType_t WifiEncryptionType,uint8_t MaxConnections_1_to_4,bool HiddenSSID)
{
	uint8_t result;
	bool	returnVal=false;
	do
	{
		/*
		 * It creates the SoftAP (Small WiFi Network) that it is possible with the ESP8266
		 * the data of this network is defined by the inputs of the function
		 */
		Wifi_RxClear();
		sprintf((char*)Wifi.TxBuffer,"AT+CWSAP=\"%s\",\"%s\",%d,%d,%d,%d\r\n",SSID,password,channel,WifiEncryptionType,MaxConnections_1_to_4,HiddenSSID);
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
			break;			// The timeout was completed and the string was not there
		if(result == 2)		// It was find the "ERROR" String in the receiving information
			break;

		returnVal=true;
	}while(0);
	return returnVal;
}
//#########################################################################################################
bool Wifi_GetApConnection(void){
	bool returnVal = false;
	uint8_t result;
	do
	{
		Wifi_RxClear();
		// Get the name of the AP Connection
		sprintf((char*)Wifi.TxBuffer,"AT+CWJAP_CUR?\r\n");
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
			break;			// The timeout was completed and the string was not there
		if(result == 2)		// It was find the "ERROR" String in the receiving information
			break;
		/*
		 * The previous AT Command has a response like the next example:
		 * 			AT+CWJAP_CUR?
		 * 			+CWJAP_CUR:<ssid>,<bssid>,<channel>,<rssi>
		 * 			OK
		 * The previous function checks the OK Response and the next lines search for the
		 * information of the SSID, taking in count the position of that information in
		 * the string. It uses pointers to get the start and the final position and the
		 * value is saved in the structure of the Wifi Connection.
		 */

		char *str1 = strstr((char*)Wifi.RxBuffer,":\"");
		if(str1==NULL){
			str1 = strstr((char*)Wifi.RxBuffer,"No AP");
			if(str1 == NULL)
				break;
			else{
				memset(Wifi.SSID_Connected, (uint8_t)'\0', 20);
				returnVal = true;
			}
		}
		else
		{
			str1 = str1+2;
			char *str2 = strstr(str1,"\"");
			size_t len = str2-str1;
			strncpy(Wifi.SSID_Connected, str1, len);
			Wifi.SSID_Connected[len] = '\0';
			returnVal=true;
		}
	}while(0);

	return returnVal;
}

//#########################################################################################################
bool  Wifi_SoftAp_GetConnectedDevices(void)
{
	uint8_t result;
	bool	returnVal=false;
	do
	{
		/*
		 * The ESP8266 has the possibility to have in a SoftAP (Small WiFi Network) four
		 * devices connected at the same time. This function save the information about
		 * the devices, which is the IP address and the MAC Address of every device
		 */
		Wifi_RxClear();
		sprintf((char*)Wifi.TxBuffer,"AT+CWLIF\r\n");
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
			break;			// The timeout was completed and the string was not there
		if(result == 2)		// It was find the "ERROR" String in the receiving information
			break;

		Wifi_RemoveChar((char*)Wifi.RxBuffer,'\r');
		Wifi_ReturnStrings((char*)Wifi.RxBuffer,"\n,",10,Wifi.SoftApConnectedDevicesIp[0],Wifi.SoftApConnectedDevicesMac[0],Wifi.SoftApConnectedDevicesIp[1],Wifi.SoftApConnectedDevicesMac[1],Wifi.SoftApConnectedDevicesIp[2],Wifi.SoftApConnectedDevicesMac[2],Wifi.SoftApConnectedDevicesIp[3],Wifi.SoftApConnectedDevicesMac[3],Wifi.SoftApConnectedDevicesIp[4],Wifi.SoftApConnectedDevicesMac[4]);

		// Search if there is any device or it does not have any
		for(uint8_t i=0 ; i<6 ; i++)
		{
		  if( (Wifi.SoftApConnectedDevicesIp[i][0]<'0') || (Wifi.SoftApConnectedDevicesIp[i][0]>'9'))
			Wifi.SoftApConnectedDevicesIp[i][0]=0;
		  if( (Wifi.SoftApConnectedDevicesMac[i][0]<'0') || (Wifi.SoftApConnectedDevicesMac[i][0]>'9'))
			Wifi.SoftApConnectedDevicesMac[i][0]=0;
		}

		returnVal=true;
	}while(0);
	return returnVal;
}
//#########################################################################################################
//#########################################################################################################
//#########################################################################################################
//#########################################################################################################
bool  Wifi_TcpIp_GetConnectionStatus(void)
{
	uint8_t result;
	bool	returnVal=false;
	do
	{
		Wifi_RxClear();
		// Get the connections status of all the possible connection with the ESP8266
		sprintf((char*)Wifi.TxBuffer,"AT+CIPSTATUS\r\n");
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
			break;			// The timeout was completed and the string was not there
		if(result == 2)		// It was find the "ERROR" String in the receiving information
			break;
		/*
		 * it searches for the string STATUS: to know what is the status of the connection
		 * and save this information in the different variables of the structure. The ESP8266
		 * has five possible connection at the same time, therefore it uses a for to save
		 * all the needed information in the structure. If there are not other connection
		 * the str return a NULL value and the loop is broken.
		 */
		char *str = strstr((char*)Wifi.RxBuffer,"\nSTATUS:");
		if(str==NULL)
		  break;
		str = strchr(str,':');
		str++;
		for(uint8_t i=0 ; i<5 ;i++)
		  Wifi.TcpIpConnections[i].status=(WifiConnectionStatus_t)atoi(str);
		str = strstr((char*)Wifi.RxBuffer,"+CIPSTATUS:");
		for(uint8_t i=0 ; i<5 ;i++)
		{
		  sscanf(str,"+CIPSTATUS:%d,\"%3s\",\"%[^\"]\",%d,%d,%d",(int*)&Wifi.TcpIpConnections[i].LinkId,Wifi.TcpIpConnections[i].Type,Wifi.TcpIpConnections[i].RemoteIp,(int*)&Wifi.TcpIpConnections[i].RemotePort,(int*)&Wifi.TcpIpConnections[i].LocalPort,(int*)&Wifi.TcpIpConnections[i].RunAsServer);
		  str++;
		  str = strstr(str,"+CIPSTATUS:");
		  if(str==NULL)
			break;
		}
		returnVal=true;

	}while(0);
	return returnVal;
}
//#########################################################################################################
bool  Wifi_TcpIp_Ping(char *PingTo)
{
	uint8_t result;
	bool	returnVal=false;
	do
	{
		Wifi_RxClear();
		sprintf((char*)Wifi.TxBuffer,"AT+PING=\"%s\"\r\n",PingTo);
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_MED,&result,3,"OK","ERROR")==false)
			break;			// The timeout was completed and the string was not there
		if(result == 2)		// It was find the "ERROR" String in the receiving information
			break;
    if(Wifi_ReturnInteger((int32_t*)&Wifi.TcpIpPingAnswer,2,"+")==false)
      break;
		returnVal=true;
	}while(0);
	return returnVal;
}
//#########################################################################################################
bool  Wifi_TcpIp_SetMultiConnection(bool EnableMultiConnections)
{
	uint8_t result;
	bool	returnVal=false;
	do
	{
		// Enable or Disable the multiconnection possibility
		Wifi_RxClear();
		sprintf((char*)Wifi.TxBuffer,"AT+CIPMUX=%d\r\n",EnableMultiConnections);
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
			break;
		if(result == 2)
			break;
    Wifi.TcpIpMultiConnection=EnableMultiConnections;
		returnVal=true;
	}while(0);
	return returnVal;
}
//#########################################################################################################
bool  Wifi_TcpIp_GetMultiConnection(void)
{
	uint8_t result;
	bool	returnVal=false;
	do
	{
		Wifi_RxClear();
		sprintf((char*)Wifi.TxBuffer,"AT+CIPMUX?\r\n");
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
			break;			// The timeout was completed and the string was not there
		if(result == 2)		// It was find the "ERROR" String in the receiving information
			break;
    if(Wifi_ReturnInteger((int32_t*)&result,1,":")==false)
      break;
    Wifi.TcpIpMultiConnection=(bool)result;
		returnVal=true;
	}while(0);
	return returnVal;
}
//#########################################################################################################
bool  Wifi_TcpIp_StartTcpConnection(uint8_t LinkId,char *RemoteIp,uint16_t RemotePort,uint16_t TimeOut)
{
	uint8_t result;
	bool	returnVal=false;
	do
	{
		/*
		 * It makes a TCP server and then it creates a TCP Connection according to the
		 * settings in the function. It uses a very high time of waiting because the
		 * ESP8266 takes a lot of time to create a connection with a TCP the first time.
		 */
		Wifi_RxClear();
		if(Wifi.TcpIpMultiConnection==true){
			sprintf((char*)Wifi.TxBuffer,"AT+CIPSERVER=1,%d\r\n",RemotePort);
			if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
				break;
			if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
				break;			// The timeout was completed and the string was not there
			if(result == 2)		// It was find the "ERROR" String in the receiving information
				break;
		}
		Wifi_RxClear();
		if(Wifi.TcpIpMultiConnection==false)
		  sprintf((char*)Wifi.TxBuffer,"AT+CIPSTART=\"TCP\",\"%s\",%d,%d\r\n",RemoteIp,RemotePort,TimeOut);
		else
		  sprintf((char*)Wifi.TxBuffer,"AT+CIPSTART=%d,\"TCP\",\"%s\",%d,%d\r\n",LinkId,RemoteIp,RemotePort,TimeOut);
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_HIGH,&result,3,"OK","CONNECT","ERROR")==false)
			break;
		if(result == 3)
			break;
		returnVal=true;
	}while(0);
	return returnVal;
}
//#########################################################################################################
bool  Wifi_TcpIp_StartUdpConnection(uint8_t LinkId,char *RemoteIp,uint16_t RemotePort,uint16_t LocalPort)
{
	uint8_t result;
	bool	returnVal=false;
	do
	{
		Wifi_RxClear();
		if(Wifi.TcpIpMultiConnection==false)
		  sprintf((char*)Wifi.TxBuffer,"AT+CIPSTART=\"UDP\",\"%s\",%d,%d\r\n",RemoteIp,RemotePort,LocalPort);
		else
		  sprintf((char*)Wifi.TxBuffer,"AT+CIPSTART=%d,\"UDP\",\"%s\",%d,%d\r\n",LinkId,RemoteIp,RemotePort,LocalPort);
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_HIGH,&result,3,"OK","ALREADY","ERROR")==false)
			break;
		if(result == 3)		// If the RX String returns a ERROR the function breaks and
			break;			// send a false
		returnVal=true;
	}while(0);
	return returnVal;
}
//#########################################################################################################
bool  Wifi_TcpIp_Close(uint8_t LinkId)
{
	uint8_t result;
	bool	returnVal=false;
	do
	{
		Wifi_RxClear();
		if(Wifi.TcpIpMultiConnection==false)
		  sprintf((char*)Wifi.TxBuffer,"AT+CIPCLOSE\r\n");
		else
		  sprintf((char*)Wifi.TxBuffer,"AT+CIPCLOSE=%d\r\n",LinkId);
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
			break;			// The timeout was completed and the string was not there
		if(result == 2)		// It was find the "ERROR" String in the receiving information
			break;
		returnVal=true;
	}while(0);
	return returnVal;
}
//#########################################################################################################
bool  Wifi_TcpIp_SetEnableTcpServer(uint16_t PortNumber)
{
	uint8_t result;
	bool	returnVal=false;
	do
	{
		Wifi_RxClear();
		if(Wifi.TcpIpMultiConnection==false)
		{
			// it actives the Multiconnection first to make a TCP Server
			sprintf((char*)Wifi.TxBuffer,"AT+CIPMUX=1\r\n");
			if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
				break;
			if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
				break;			// The timeout was completed and the string was not there
			if(result == 2)		// It was find the "ERROR" String in the receiving information
				break;
			Wifi.TcpIpMultiConnection=true;
			Wifi_RxClear();
		}
		else
		  sprintf((char*)Wifi.TxBuffer,"AT+CIPSERVER=1,%d\r\n",PortNumber);
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
			break;			// The timeout was completed and the string was not there
		if(result == 2)		// It was find the "ERROR" String in the receiving information
			break;
		returnVal=true;
	}while(0);
	return returnVal;
}
//#########################################################################################################
bool  Wifi_TcpIp_SetDisableTcpServer(uint16_t PortNumber)
{
	uint8_t result;
	bool	returnVal=false;
	do
	{
		Wifi_RxClear();
		sprintf((char*)Wifi.TxBuffer,"AT+CIPSERVER=0,%d\r\n",PortNumber);
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
			break;			// The timeout was completed and the string was not there
		if(result == 2)		// It was find the "ERROR" String in the receiving information
			break;
		returnVal=true;
	}while(0);
	return returnVal;
}
//#########################################################################################################
bool  Wifi_TcpIp_SendDataUdp(uint8_t LinkId,uint16_t dataLen,uint8_t *data)
{
	uint8_t result;
	bool	returnVal=false;
	do
	{
		Wifi_RxClear();
		if(Wifi.TcpIpMultiConnection==false)
			sprintf((char*)Wifi.TxBuffer,"AT+CIPSEND=%d\r\n",dataLen);
		else
			sprintf((char*)Wifi.TxBuffer,"AT+CIPSEND=%d,%d\r\n",LinkId,dataLen);
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,">","ERROR")==false)
			break;			// The timeout was completed and the string was not there
		if(result == 2)		// It was find the "ERROR" String in the receiving information
			break;
		Wifi_RxClear();
		// Send in this time the information through the connection
		Wifi_SendRaw(data,dataLen);
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
			break;			// The timeout was completed and the string was not there
		if(result == 2)		// It was find the "ERROR" String in the receiving information
			break;
		returnVal=true;
	}while(0);
	return returnVal;

}
//#########################################################################################################
bool  Wifi_TcpIp_SendDataTcp(uint8_t LinkId,uint16_t dataLen,uint8_t *data)
{
	uint8_t result;
	bool	returnVal=false;
	do
	{
		Wifi_RxClear();
		if(Wifi.TcpIpMultiConnection==false)
			sprintf((char*)Wifi.TxBuffer,"AT+CIPSENDBUF=%d\r\n",dataLen);
		else
			sprintf((char*)Wifi.TxBuffer,"AT+CIPSENDBUF=%d,%d\r\n",LinkId,dataLen);
		if(Wifi_SendString((char*)Wifi.TxBuffer)==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
			break;
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,3,">","ERROR","busy")==false)
			break;
		if(result > 1)
			break;
		Wifi_RxClear();
		Wifi_SendRaw(data,dataLen);
		if(Wifi_WaitForString(_WIFI_WAIT_TIME_LOW,&result,2,"OK","ERROR")==false)
			break;
		returnVal=true;
	}while(0);
	return returnVal;
}
//#########################################################################################################
