

// Not implemented yet - from the existing .NET API

        /// <summary>
        /// Change homing, docking and driving parameters â speed for driving commands.
        /// <remarks>NOT IMPLEMENTED</remarks>
        /// </summary>
        /// <returns></returns>
		//        public string SetTuningParameters()
		//      {
		//        return rwc.Request("rev.cgi?Cmd=nav&action=15");
  		//  }

        
        /// <summary>
        /// Returns homing, docking and driving parameters.
        /// </summary>
        /// <returns></returns>
        // public string GetTuningParameters()
        // {
        //     return rwc.Request("rev.cgi?Cmd=nav&action=16");
        // }
       
        /// <summary>
        /// Get email settings.
        /// </summary>
        /// <returns></returns>
        //public string GetMail() 
        //{
        //     return rwc.Request("GetMail.cgi");
        // }

        /// <summary>
        /// Send an email with IPCam images.
        /// </summary>
        /// <returns></returns>
        //public string SendMail() 
        //{
        //    return rwc.Request("SendMail.cgi");
        //}

        /// <summary>
        /// Stores parameter in the robotâs Flash memory.
        /// </summary>
        /// <param name="index">0 â 19</param>
        /// <param name="value">32bit signed integer</param>
        /// <returns></returns>
        //public string SaveParameter(long index, long value)
        // {
        //    return rwc.Request("rev.cgi?Cmd=nav&action=23&index="+index.ToString()+"&value="+value.ToString());
        //}


        /// <summary>
        /// Read parameter in the robotâs Flash memory.
        /// </summary>
        /// <param name="index">0 â 19</param>
        /// <returns></returns>
        // public string ReadParameter(long index)
        //{
        //    return rwc.Request("rev.cgi?Cmd=nav&action=24&index="+index.ToString());
        //}

        /// <summary>
        /// Set server time zone and time.
        /// </summary>
        /// <param name="Sec1970">seconds since "00:00:00 1/1/1970".</param>
        /// <param name="TimeZone">Time zone in minutes. (e.g. Beijing is GMT+08:00, TimeZone = -480)</param>
        /// <returns></returns>
        // public string SetTime(long Sec1970, int TimeZone)
        // {
        //    return rwc.Request("SetTime.cgi?Sec1970=" + Sec1970.ToString() + "&TimeZone="+TimeZone.ToString());
        // }


        /// <summary>
        /// Get current IP Camera's time zone and time.
        /// </summary>
        /// <returns></returns>
        // public string GetTime() 
        // {
        //    return rwc.Request("GetTime.cgi");
        // }

	    /// <summary>
        /// Get Rovioâs system logs information.
        /// </summary>
        /// <returns></returns>
        // public string GetLog() 
        // {
        //     return rwc.Request("GetLog.cgi");
        // }
        
        
    // set/change robot wireless/wired network settings

		// get IP address
		// return value : IP address as string

		char* getIP();

		// get MAC address
		// return value : MAC address as string
		
		char* getMAC();

		// set MAC address 
		// - address - MAC address as string of form XX:XX:XX:XX:XX
		//             (then sent to robot as XXXXXXXXXX)
		// return value : success/failure
		
 		bool setMAC(char* address);

		// get wireless network ID (ESSID)
		// return value : string text of network ESSID

		char* getWirelessESSID();

		// set wireless network ID (ESSID)
		// - essid - ESSID as string
		// return value : success/failure
		
 		bool setWirelessESSID(char* essid);
		
 		// set wireless network mode to managed
		// - essid - ESSID as string
		// return value : success/failure
		
		bool setWirelessModeManaged(char* essid);

		// set wireless network mode to ad-hoc
		// N.B. ESSID is set to hostname of robot
		// return value : success/failure
		
		bool setWirelessModeAdHoc();

		// set DHCP use
		// return value : success/failure
		
		bool useDHCP();

		// set manual ip address use
		// - address - IP address as string (e.g. "192.168.0.1")

		bool useManualIP(char* address);        
        
        // set camera saturation level
		// - level = range 0 (low) - 11 (high); -1 for default 
		// return value: success/failure
		
		bool setSaturation(int level);
		
		// set camera hue level
		// - level = range 0 (low) - 11 (high); -1 for default 
		// return value: success/failure
		
		bool setHue(int level);

		// set camera contrast level
		// - level = range 0 (low) - 5 (high); -1 for default 
		// return value: success/failure
		
		bool setContrast(int level);

		// set camera sharpness level
		// - level = range 0 (low) - 5 (high); -1 for default 
		// return value: success/failure
		
		bool setSharpness(int level);
        
        // list usernames
        // - usernames = returned list of usernames
		// return value: number of users, -1 on error
		
		int listUsers(char ** usernames);
		
		// set authentication
		// - status = true (user/pass req'd) or false (open access)
		
		bool setAuthentication(bool status);
		
		// set rovio name
		// - name = name of rovio 
		// return value: success/failure
		
		bool setName(char * name);
		
		// reset all settings to factory defaults
		
		bool resetToFactory();
        
        
// *****************************************************************************

char* Rovio::getIP(){

	char * returnValue = NULL;	// default error return

	// set up a memory write function
	
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ROVIO_CURL_WriteMemoryCallback);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA, (void *) &buffer);

	// send the command

	if (sendToRobot((char *) "GetIP.cgi?&IP") == ROVIO_CGI_RETURN_CODE_ALL_OK)
	{
	
		// return pointer to buffer
	
		returnValue = buffer.memory;
		
		// reset buffer for re-allocation on next call
		// (previous memory now belongs outside of call)
		
		buffer.memory = NULL;
		buffer.size = 0;
	
		if (verboseMode){
			printf("ROVIO: IP address query : %s\n\n", returnValue);
		} 
		return returnValue;
	}
	
	if (verboseMode){
			printf("ROVIO: IP address query : FAILURE\n");
	} 
	return returnValue;

}

// *****************************************************************************

char* Rovio::getMAC(){

	char * returnValue = NULL;	// default error return

	// set up a memory write function
	
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ROVIO_CURL_WriteMemoryCallback);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA, (void *) &buffer);

	// send the command

	if (sendToRobot((char *) "GetMac.cgi") == ROVIO_CGI_RETURN_CODE_ALL_OK)
	{
	
		// return pointer to buffer
	
		returnValue = buffer.memory;
		
		// reset buffer for re-allocation on next call
		// (previous memory now belongs outside of call)
		
		buffer.memory = NULL;
		buffer.size = 0;
	
		if (verboseMode){
			printf("ROVIO: MAC address query : %s\n\n", returnValue);
		} 
		return returnValue;
	}
	
	if (verboseMode){
			printf("ROVIO: MAC address query : FAILURE\n");
	} 
	return returnValue;

}

// *****************************************************************************

bool Rovio::setMAC(char * address){
	
	char commandStringStem[255];
	bool returnValue = false;
	char packedMAC[15];
	unsigned int i, j;
	
	// check input
	
	if(!address)
		return false;
	
	// set up command

	// REMOVE all ":" from string
    // to send MAC XX:XX:XX:XX:XX as XXXXXXXXXX)
	// and check length is 14
	
	j = 0;
	for(i = 0; i < strlen(address); i++)
	{
		if (((!isdigit(address[i])) && 
		      (!(( 'A' <= address[i]) && ( address[i] <= 'F'))) &&
		      (!(address[i] == ':'))) ||
		     (strlen(address) != 14)
		    )
		{
			if (verboseMode){
				printf("ROVIO: set MAC as %s - FAILED (invalid MAC #1)\n", address);
			} 
			return returnValue;
		} else if (!(address[i] == ':')) {
			packedMAC[j] = address[i];
			j++;
		}
			
	}

	// check MAC length (now = 10 without ":" chars)

	if (j != 10)
	{
		if (verboseMode){
			printf("ROVIO: set MAC as %s - FAILED (invalid MAC #2)\n", address);
		} 
		return returnValue;
	}

	// set final char of packed MAC to null
	
	packedMAC[j] = '\0';

	// send command to robot
	
	sprintf(commandStringStem,
	       "SetMac.cgi?MAC=%s", packedMAC);
	
	// set up a NULL write function
	
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &Rovio::NULLWriteFunction);
	
	// if return code is zero, then all OK
	
	if (sendToRobot(commandStringStem) == ROVIO_CGI_RETURN_CODE_ALL_OK)
	{
		returnValue = true;
	}
	
	if (verboseMode){
			printf("ROVIO: set MAC as %s - OK = %d\n", address, returnValue);
	} 
	return returnValue;
}

// *****************************************************************************

char* Rovio::getWirelessESSID(){

	char *essid = NULL;

	// set up a memory write function
	
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ROVIO_CURL_WriteMemoryCallback);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA, (void *) &buffer);

	// send the command

	sendToRobot((char *) "GetWlan.cgi");
	
	// extract ESSID information from the buffer

	if (buffer.memory)
	{
		char* ptr = strstr(buffer.memory, "ESSID = ");
		if (ptr){
			ptr += strlen("ESSID = ");
			if (ptr < (buffer.memory + buffer.size)){

				// allocate the memory only if we get this 
				// far. This memory now belongs to the caller
				
				essid = (char *) malloc(20);
				sscanf(ptr, "%s\n", essid);
			}
	    }
	}
	
	if (verboseMode){
		printf("ROVIO: ESSID get command returned : %s\n", essid);
	}

	free(buffer.memory);
	buffer.memory = NULL;
	buffer.size = 0;
	
	return essid;

}
// *****************************************************************************

bool Rovio::setName(char * name){
	
	char commandStringStem[255];
	char* urlEncoded = NULL;
	bool returnValue = false;
	
	// check input
	
	if(!name)
		return false;
	
	// set up command
	
	urlEncoded = curl_easy_escape(curl , name, 0);
	sprintf(commandStringStem,
	       "DelUser.cgi?User=%s", urlEncoded);
	free(urlEncoded);
	
	// set up a NULL write function
	
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &Rovio::NULLWriteFunction);
	
	// if return code is zero, then all OK
	
	if (sendToRobot(commandStringStem) == ROVIO_CGI_RETURN_CODE_ALL_OK)
	{
		returnValue = true;
	}
	
	if (verboseMode){
			printf("ROVIO: set name as %s - OK = %d\n", name, returnValue);
	} 
	return returnValue;
}

// *****************************************************************************

bool Rovio::resetToFactory(){
	
	bool returnValue = false;
	
	// set up a NULL write function
	
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &Rovio::NULLWriteFunction);
	
	// if return code is zero, then all OK
	
	if (sendToRobot((char *) "SetFactoryDefault.cgi") == ROVIO_CGI_RETURN_CODE_ALL_OK)
	{
		returnValue = true;
	}
	
	if (verboseMode){
			printf("ROVIO: reset to factory defaults - OK = %d\n", returnValue);
	} 
	
	return returnValue;
}

// *****************************************************************************

// *****************************************************************************

char* Rovio::getName(){

	char * returnValue = NULL;	// default error return

	// set up a memory write function
	
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ROVIO_CURL_WriteMemoryCallback);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA, (void *) &buffer);

	// send the command

	if (sendToRobot((char *) "GetName.cgi") == ROVIO_CGI_RETURN_CODE_ALL_OK)
	{
	
		// return pointer to buffer
	
		returnValue = buffer.memory;
		
		// reset buffer for re-allocation on next call
		// (previous memory now belongs outside of call)
		
		buffer.memory = NULL;
		buffer.size = 0;
	
		if (verboseMode){
			printf("ROVIO: robot name : %s\n", returnValue);
		} 
		return returnValue;
	}
	
	if (verboseMode){
			printf("ROVIO: get name : FAILURE\n");
	} 
	return returnValue;

}

		// set video format
		// - format = one of 0 – H263, 1 – MPEG4 (guessed)
		// return value: success/failure
		
		bool setFormat(int format);
		
		// get video format
		// return value: 1 – H263, 2 – MPEG4, -1 - failure
		
		int getFormat();


// *****************************************************************************

// DOES NOT WORK - seems to fail with format > 0

bool Rovio::setFormat(int format){

	char commandStringStem[255];

	// check input
	
	if ((format != 0) && (format != 1))
		return false;	

	// construct command

	sprintf(commandStringStem,
	       "SetMediaFormat.cgi?Audio=0&Video=%d", format);


	if (verboseMode){
		printf("ROVIO: media format command @ setting : %d\n", format);
	}
	
	// set up a NULL write function
	
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &Rovio::NULLWriteFunction);
	
	// if return code is zero, then all OK
	
	if (sendToRobot(commandStringStem) == ROVIO_CGI_RETURN_CODE_ALL_OK)
	{
		return true;
	}

	return false;
}

// *****************************************************************************

int Rovio::getFormat(){

	int setting = - 1;

	// set up a memory write function
	
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ROVIO_CURL_WriteMemoryCallback);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA, (void *) &buffer);

	// send the command

	sendToRobot((char *) "GetMediaFormat.cgi");
	
	// extract format information from the buffer

	if (buffer.memory)
	{
		char* ptr = strstr(buffer.memory, "Video = ");
		if (ptr){
			ptr += strlen("Video = ");
			if (ptr < (buffer.memory + buffer.size)){
				sscanf(ptr, "%d\n", &setting);
			}
	    }
	}
	
	if (verboseMode){
		printf("ROVIO: format get command returned : %d\n", setting);
	}

	// reset buffer
	
	free(buffer.memory);
	buffer.memory = NULL;
	buffer.size = 0;
	
	return setting;

}
