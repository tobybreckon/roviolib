

	bool getWlanScan(ScanResults *result);

	// WARNING modifications made by this function are saved by the robot, meaning that rebooting the robot won't cancel those modifications
	// WARNING connection to the robot might be lost after calling this function
	// Tell IP Camera how to set an initial WiFi.
	// - Mode = <Managed|Ad-Hoc> Managed on a wifi router or directly connect to Rovio using ad-hoc
	// - Channel = between 1 and 13 Wifi channel
	// - ESSID = ESSID id of Rovio in ad-hoc mode or the wifi router in managed mode
	// - WepSet = <Disable|K64|K128> Activation of a security encryption
	// - WepAsc = ???
	// - Wep64type = <Wep64HEX|Wep64ASC> set 64bit encryption key either in Hexadecimal or Ascii mode
	// - Wep64 = network 64bit key
	// - Wep128type = <Wep128HEX|Wep128ASC> set 128bit encryption key either in Hexadecimal or Ascii mode
	// - Wep128 = network 128bit key
	// - RedirectUrl = URL to redirect after the call to this page
	// return value : true on sucess, false on failure

	bool SetWlan(std::string Mode = "Ad-Hoc", std::string essid = "ROVIO_WOWWEE"
					, std::string WepSet = "Disable", std::string WepAsc = ""
					, std::string Wep64type = "", std::string Wep64 = ""
					, std::string Wep128type = "", std::string Wep128 = ""
					, int Channel = 0);

	// WARNING modifications made by this function are saved by the robot, meaning that rebooting the robot won't cancel those modifications
	// WARNING connection to the robot might be lost after calling this function
	// Tell IP Camera how to set an initial IP.
	// - IPWay = <manually|dhcp> Either set the IP configuration manualy or get it from a dhcp serveur
	// - IP = IP address of the IP camera
	// - Netmask = Netmask address of the IP camera
	// - Gateway = Gateway address of the IP camera
	// - DNS0 = IP address of the 1st DNS
	// - DNS1 = IP address of the 2nd DNS
	// - DNS2 = IP address of the 3rd DNS
	// - RedirectUrl = URL to redirect after the call to this page
	// return value : true on sucess, false on failure

	bool SetIP(std::string IPWay = "manually", std::string IP = "192.168.10.18"
					, std::string Netmask = "255.255.255.0", std::string Gateway = "192.168.10.1"
					, std::string DNS0 = "0.0.0.0", std::string DNS1 = "0.0.0.0", std::string DNS2 = "0.0.0.0");

	// WARNING modifications made by this function are saved by the robot, meaning that rebooting the robot won't cancel those modifications
	// WARNING connection to the robot might be lost after calling this function
	// Set dyndns.org service for IPCam
	// - Service : DDNS service provider
	// - User : username
	// - Pass : password
	// - IP : IP address (null for auto detect)
	// - Proxy : name of the proxy
	// - ProxyPort : port of the proxy
	// - ProxyUser : username of the proxy
	// - ProxyPass : password of the proxy
	// return value : true on sucess, false on failure

	bool setDDNS(bool Enable = false, std::string Service = "", std::string User = "", std::string Pass = "", std::string DomainName = ""
				, std::string IP = "", std::string Proxy = "", std::string ProxyPort = "", std::string ProxyUser = "", std::string ProxyPass = "");


	// WARNING modifications made by this function are saved by the robot, meaning that rebooting the robot won't cancel those modifications
	// WARNING connection to the robot might be lost after calling this function
	// Combine the function SetWlan and SetIP and send both command at the same time to the IP camera

	bool SetNetworkConfiguration(std::string Mode = "Ad-Hoc", std::string essid = "ROVIO_WOWWEE"
							, std::string WepSet = "Disable", std::string WepAsc = ""
							, std::string Wep64type = "", std::string Wep64 = ""
							, std::string Wep128type = "", std::string Wep128 = ""
							, int Channel = 0
							, std::string IPWay = "manually", std::string IP = "192.168.10.18"
							, std::string Netmask = "255.255.255.0", std::string Gateway = "192.168.10.1"
							, std::string DNS0 = "0.0.0.0", std::string DNS1 = "0.0.0.0", std::string DNS2 = "0.0.0.0");
