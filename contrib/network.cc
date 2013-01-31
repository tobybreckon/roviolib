bool Rovio::SetWlan(std::string Mode, std::string essid
					, std::string WepSet, std::string WepAsc
					, std::string Wep64type, std::string Wep64
					, std::string Wep128type, std::string Wep128
					, int Channel)
{

	std::string baseURL = "SetWlan.cgi?";
	bool updated = false;

	// check input

	if(Mode.empty() && essid.empty() && Wep64type.empty() && Wep64.empty() && WepSet.empty() && WepAsc.empty()
		&& Wep128type.empty() && Wep128.empty() && Channel == 0) {
			printf("ERROR SetWlan : bad inputs");
			return false;
	}

	if( !Mode.empty() && !Mode.compare("Ad-Hoc") && !Mode.compare("Managed")) {
		printf("ERROR SetWlan : usage Mode=<Managed|Ad-Hoc>");
		return false;
	}

	if( !WepSet.empty() && !WepSet.compare("Disable") && !WepSet.compare("K64") && !WepSet.compare("K128")) {
		printf("ERROR SetWlan : usage WepSet=<Disable|K64|K128>");
		return false;
	}

	if( !Wep64type.empty() && !Wep64type.compare("Wep64HEX") && !Wep64type.compare("Wep64ASC")) {
		printf("ERROR SetWlan : usage Wep64type=<Wep64HEX|Wep64ASC>");
		return false;
	}

	if( !Wep128type.empty() && !Wep128type.compare("Wep128HEX") && !Wep128type.compare("Wep128ASC")) {
		printf("ERROR SetWlan : usage Wep128type=<Wep128HEX|Wep128ASC>");
		return false;
	}


	if(Channel < 0 || Channel > 13)
		Channel = 0;

	// construct command

	if(!Mode.empty()) {
		baseURL += "Mode=" + Mode;
		updated = true;
	}

	if(Channel != 0) {
		if(updated) baseURL += "&";
		baseURL += "Channel=" + Channel;
		updated = true;
	}

	if(!essid.empty()) {
		if(updated) baseURL += "&";
		baseURL += "ESSID=" + essid;
		updated = true;
	}

	if(!WepSet.empty()) {
		if(updated) baseURL += "&";
		baseURL += "WepSet=" + WepSet;
		updated = true;
	}

	if(!WepAsc.empty()) {
		if(updated) baseURL += "&";
		baseURL += "WepAsc=" + WepAsc;
		updated = true;
	}

	if(!Wep64type.empty()) {
		if(updated) baseURL += "&";
		baseURL += "Wep64type=" + Wep64type;
		updated = true;
	}

	if(!Wep64.empty()) {
		if(updated) baseURL += "&";
		baseURL += "Wep64=" + Wep64;
		updated = true;
	}

	if(!Wep128type.empty()) {
		if(updated) baseURL += "&";
		baseURL += "Wep128type=" + Wep128type;
		updated = true;
	}

	if(!Wep128.empty()) {
		if(updated) baseURL += "&";
		baseURL += "Wep128=" + Wep128;
		updated = true;
	}

    if (verboseMode)
    {
        printf("\nROVIO: SetWlan command : %s\n", baseURL.c_str());
    }

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK
	char *cstr = new char [baseURL.size()+1];
	strcpy (cstr, baseURL.c_str());

	if (sendToRobot(cstr) == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
		delete[] cstr;
        return true;
    }

	delete[] cstr;
    return false;
}

bool Rovio::SetIP(std::string IPWay, std::string IP
				, std::string Netmask, std::string Gateway
				, std::string DNS0, std::string DNS1, std::string DNS2)
{
	std::string baseURL = "SetIP.cgi?";
	bool updated = false;

	// check input

	if(IPWay.empty() && IP.empty() && Netmask.empty() && Gateway.empty()
		&& DNS0.empty() && DNS1.empty() && DNS2.empty() ) {
			printf("ERROR SetIP : empty inputs");
			return false;
	}

	if( !IPWay.empty() && !IPWay.compare("manually") && !IPWay.compare("dhcp")) {
		printf("ERROR SetIP : usage IPWay=<manually|dhcp>");
		return false;
	}


	// construct command

	if(!IPWay.empty()) {
		baseURL += "IPWay=" + IPWay;
		updated = true;
	}

	if(!IP.empty()) {
		if(updated) baseURL += "&";
		baseURL += "IP=" + IP;
		updated = true;
	}

	if(!Netmask.empty()) {
		if(updated) baseURL += "&";
		baseURL += "Netmask=" + Netmask;
		updated = true;
	}

	if(!Gateway.empty()) {
		if(updated) baseURL += "&";
		baseURL += "Gateway=" + Gateway;
		updated = true;
	}

	if(!DNS0.empty()) {
		if(updated) baseURL += "&";
		baseURL += "DNS0=" + DNS0;
		updated = true;
	}

	if(!DNS1.empty()) {
		if(updated) baseURL += "&";
		baseURL += "DNS1=" + DNS1;
		updated = true;
	}

	if(!DNS2.empty()) {
		if(updated) baseURL += "&";
		baseURL += "DNS2=" + DNS2;
		updated = true;
	}

    if (verboseMode)
    {
        printf("\nROVIO: SetIP command : %s\n", baseURL.c_str());
    }

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK
	char *cstr = new char [baseURL.size()+1];
	strcpy (cstr, baseURL.c_str());

	if (sendToRobot(cstr) == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
		delete[] cstr;
        return true;
    }

	delete[] cstr;
    return false;
}

bool Rovio::setDDNS(bool Enable, std::string Service, std::string User, std::string Pass, std::string DomainName
				, std::string IP, std::string Proxy, std::string ProxyPort, std::string ProxyUser, std::string ProxyPass)
{
	std::string baseURL = "setDDNS.cgi?";
	bool updated = false;

	// check input

	if(Service.empty() && User.empty() && Pass.empty() && DomainName.empty()
		&& IP.empty() && Proxy.empty() && ProxyPort.empty() && ProxyUser.empty() && ProxyPass.empty()) {
			printf("ERROR setDDNS : empty inputs");
			return false;
	}

	if( !Service.empty() && !Service.compare("dyndns") && !Service.compare("no-ip") && !Service.compare("dnsomatic")) {
		printf("ERROR setDDNS : usage Service=<dyndns|noip|dnsomatic>");
		return false;
	}


	// construct command

	baseURL += "Enable=" ;
	baseURL += Enable ? "true&" : "false&";


	if(!IP.empty()) {
		baseURL += "IP=" + IP;
		updated = true;
	}

	if(!Service.empty()) {
		if(updated) baseURL += "&";
		baseURL += "Service=" + Service;
		updated = true;
	}

	if(!User.empty()) {
		if(updated) baseURL += "&";
		baseURL += "User=" + User;
		updated = true;
	}

	if(!Pass.empty()) {
		if(updated) baseURL += "&";
		baseURL += "Pass=" + Pass;
		updated = true;
	}

	if(!DomainName.empty()) {
		if(updated) baseURL += "&";
		baseURL += "DomainName=" + DomainName;
		updated = true;
	}

	if(!IP.empty()) {
		if(updated) baseURL += "&";
		baseURL += "IP=" + IP;
		updated = true;
	}

	if(!Proxy.empty()) {
		if(updated) baseURL += "&";
		baseURL += "Proxy=" + Proxy;
		updated = true;
	}

	if(!ProxyPort.empty()) {
		if(updated) baseURL += "&";
		baseURL += "ProxyPort=" + ProxyPort;
		updated = true;
	}

	if(!ProxyUser.empty()) {
		if(updated) baseURL += "&";
		baseURL += "ProxyUser=" + ProxyUser;
		updated = true;
	}

	if(!ProxyPass.empty()) {
		if(updated) baseURL += "&";
		baseURL += "ProxyPass=" + ProxyPass;
		updated = true;
	}

    if (verboseMode)
    {
        printf("\nROVIO: SetIP command : %s\n", baseURL.c_str());
    }

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK
	char *cstr = new char [baseURL.size()+1];
	strcpy (cstr, baseURL.c_str());

	if (sendToRobot(cstr) == ROVIO_HTTP_RETURN_CODE_ALL_OK)
    {
		delete[] cstr;
        return true;
    }

	delete[] cstr;
    return false;
}

bool Rovio::SetNetworkConfiguration(std::string Mode, std::string essid
							, std::string WepSet, std::string WepAsc
							, std::string Wep64type, std::string Wep64
							, std::string Wep128type, std::string Wep128
							, int Channel
							, std::string IPWay, std::string IP
							, std::string Netmask, std::string Gateway
							, std::string DNS0, std::string DNS1, std::string DNS2)
{
	std::string baseURL = "Cmd.cgi?";
	std::string baseURLSetWLan = "Cmd=SetWlan.cgi&";
    std::string baseURLSetIP = "Cmd=SetIP.cgi&";

	// check input

	if(Channel < 0 || Channel > 13)
		Channel = 0;

	if(Mode.empty() && essid.empty() && Wep64type.empty() && Wep64.empty() && WepSet.empty() && WepAsc.empty()
		&& Wep128type.empty() && Wep128.empty() && Channel == 0
		&& IPWay.empty() && IP.empty() && Netmask.empty() && Gateway.empty()
		&& DNS0.empty() && DNS1.empty() && DNS2.empty()) {
			printf("ERROR SetNetworkConfiguration : bad inputs");
			return false;
	}

	if( !Mode.empty() && !Mode.compare("Ad-Hoc") && !Mode.compare("Managed")) {
		printf("ERROR SetNetworkConfiguration : usage Mode=<Managed|Ad-Hoc>");
		return false;
	}

	if( !WepSet.empty() && !WepSet.compare("Disable") && !WepSet.compare("K64") && !WepSet.compare("K128")) {
		printf("ERROR SetNetworkConfiguration : usage WepSet=<Disable|K64|K128>");
		return false;
	}

	if( !Wep64type.empty() && !Wep64type.compare("Wep64HEX") && !Wep64type.compare("Wep64ASC")) {
		printf("ERROR SetNetworkConfiguration : usage Wep64type=<Wep64HEX|Wep64ASC>");
		return false;
	}

	if( !Wep128type.empty() && !Wep128type.compare("Wep128HEX") && !Wep128type.compare("Wep128ASC")) {
		printf("ERROR SetNetworkConfiguration : usage Wep128type=<Wep128HEX|Wep128ASC>");
		return false;
	}

	if( !IPWay.empty() && !IPWay.compare("manually") && !IPWay.compare("dhcp")) {
		printf("ERROR SetNetworkConfiguration : usage IPWay=<manually|dhcp>");
		return false;
	}

	// construct command SetWlan

	if(!Mode.empty())
		baseURLSetWLan += "Mode=" + Mode + "&";

	if(Channel != 0) {
		baseURLSetWLan += "Channel=" + Channel;
		baseURLSetWLan += "&";
	}

	if(!essid.empty()) {

		// replace empty space by %20

		std::string s = "%20";
		for (int i=0; i < essid.size(); i++ )
			if(essid[i] == ' ')
				essid.replace(i, 1, s, 0, 3);

		baseURLSetWLan += "ESSID=" + essid + "&";
	}

	if(!WepSet.empty())
		baseURLSetWLan += "WepSet=" + WepSet + "&";

	if(!WepAsc.empty())
		baseURLSetWLan += "WepAsc=" + WepAsc + "&";

	if(!Wep64type.empty())
		baseURLSetWLan += "Wep64type=" + Wep64type + "&";

	if(!Wep64.empty())
		baseURLSetWLan += "Wep64=" + Wep64 + "&";

	if(!Wep128type.empty())
		baseURLSetWLan += "Wep128type=" + Wep128type + "&";

	if(!Wep128.empty())
		baseURLSetWLan += "Wep128=" + Wep128 + "&";

	// construct command SetIP

	if(!IPWay.empty())
		baseURLSetIP += "IPWay=" + IPWay + "&";

	if(!IP.empty())
		baseURLSetIP += "IP=" + IP + "&";

	if(!Netmask.empty())
		baseURLSetIP += "Netmask=" + Netmask + "&";

	if(!Gateway.empty())
		baseURLSetIP += "Gateway=" + Gateway + "&";

	if(!DNS0.empty())
		baseURLSetIP += "DNS0=" + DNS0 + "&";

	if(!DNS1.empty())
		baseURLSetIP += "DNS1=" + DNS1 + "&";

	if(!DNS2.empty())
		baseURLSetIP += "DNS2=" + DNS2 + "&";

	// create final command

	baseURL += baseURLSetWLan + baseURLSetIP;

    if (verboseMode)
    {
		printf("\nROVIO: SetNetworkConfiguration command : %s\n", baseURL.c_str());
    }

    // set up a NULL write function

    curl_easy_setopt(curlCom, CURLOPT_WRITEFUNCTION, NULLWriteFunction);

    // if return code is zero, then all OK
	char *cstr = new char [baseURL.size()+1];
	strcpy (cstr, baseURL.c_str());

	sendToRobot(cstr);

	delete[] cstr;
    return true;
}
