// Machine generated IDispatch wrapper class(es) created with Add Class from Typelib Wizard

// CWMPNetwork wrapper class

class CWMPNetwork : public COleDispatchDriver
{
public:
	CWMPNetwork(){} // Calls COleDispatchDriver default constructor
	CWMPNetwork(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CWMPNetwork(const CWMPNetwork& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

	// Attributes
public:

	// Operations
public:


	// IWMPNetwork methods
public:
	long get_bandWidth()
	{
		long result;
		InvokeHelper(0x321, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	long get_recoveredPackets()
	{
		long result;
		InvokeHelper(0x322, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	CString get_sourceProtocol()
	{
		CString result;
		InvokeHelper(0x323, DISPATCH_PROPERTYGET, VT_BSTR, (void*)&result, NULL);
		return result;
	}
	long get_receivedPackets()
	{
		long result;
		InvokeHelper(0x324, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	long get_lostPackets()
	{
		long result;
		InvokeHelper(0x325, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	long get_receptionQuality()
	{
		long result;
		InvokeHelper(0x326, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	long get_bufferingCount()
	{
		long result;
		InvokeHelper(0x327, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	long get_bufferingProgress()
	{
		long result;
		InvokeHelper(0x328, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	long get_bufferingTime()
	{
		long result;
		InvokeHelper(0x329, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	void put_bufferingTime(long newValue)
	{
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x329, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
	}
	long get_frameRate()
	{
		long result;
		InvokeHelper(0x32a, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	long get_maxBitRate()
	{
		long result;
		InvokeHelper(0x32b, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	long get_bitRate()
	{
		long result;
		InvokeHelper(0x32c, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	long getProxySettings(LPCTSTR bstrProtocol)
	{
		long result;
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x32d, DISPATCH_METHOD, VT_I4, (void*)&result, parms, bstrProtocol);
		return result;
	}
	void setProxySettings(LPCTSTR bstrProtocol, long lProxySetting)
	{
		static BYTE parms[] = VTS_BSTR VTS_I4 ;
		InvokeHelper(0x32e, DISPATCH_METHOD, VT_EMPTY, NULL, parms, bstrProtocol, lProxySetting);
	}
	CString getProxyName(LPCTSTR bstrProtocol)
	{
		CString result;
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x32f, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, bstrProtocol);
		return result;
	}
	void setProxyName(LPCTSTR bstrProtocol, LPCTSTR bstrProxyName)
	{
		static BYTE parms[] = VTS_BSTR VTS_BSTR ;
		InvokeHelper(0x330, DISPATCH_METHOD, VT_EMPTY, NULL, parms, bstrProtocol, bstrProxyName);
	}
	long getProxyPort(LPCTSTR bstrProtocol)
	{
		long result;
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x331, DISPATCH_METHOD, VT_I4, (void*)&result, parms, bstrProtocol);
		return result;
	}
	void setProxyPort(LPCTSTR bstrProtocol, long lProxyPort)
	{
		static BYTE parms[] = VTS_BSTR VTS_I4 ;
		InvokeHelper(0x332, DISPATCH_METHOD, VT_EMPTY, NULL, parms, bstrProtocol, lProxyPort);
	}
	CString getProxyExceptionList(LPCTSTR bstrProtocol)
	{
		CString result;
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x333, DISPATCH_METHOD, VT_BSTR, (void*)&result, parms, bstrProtocol);
		return result;
	}
	void setProxyExceptionList(LPCTSTR bstrProtocol, LPCTSTR pbstrExceptionList)
	{
		static BYTE parms[] = VTS_BSTR VTS_BSTR ;
		InvokeHelper(0x334, DISPATCH_METHOD, VT_EMPTY, NULL, parms, bstrProtocol, pbstrExceptionList);
	}
	BOOL getProxyBypassForLocal(LPCTSTR bstrProtocol)
	{
		BOOL result;
		static BYTE parms[] = VTS_BSTR ;
		InvokeHelper(0x335, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms, bstrProtocol);
		return result;
	}
	void setProxyBypassForLocal(LPCTSTR bstrProtocol, BOOL fBypassForLocal)
	{
		static BYTE parms[] = VTS_BSTR VTS_BOOL ;
		InvokeHelper(0x336, DISPATCH_METHOD, VT_EMPTY, NULL, parms, bstrProtocol, fBypassForLocal);
	}
	long get_maxBandwidth()
	{
		long result;
		InvokeHelper(0x337, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	void put_maxBandwidth(long newValue)
	{
		static BYTE parms[] = VTS_I4 ;
		InvokeHelper(0x337, DISPATCH_PROPERTYPUT, VT_EMPTY, NULL, parms, newValue);
	}
	long get_downloadProgress()
	{
		long result;
		InvokeHelper(0x338, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	long get_encodedFrameRate()
	{
		long result;
		InvokeHelper(0x339, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}
	long get_framesSkipped()
	{
		long result;
		InvokeHelper(0x33a, DISPATCH_PROPERTYGET, VT_I4, (void*)&result, NULL);
		return result;
	}

	// IWMPNetwork properties
public:

};
