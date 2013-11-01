
#ifndef __winscard_h__
#define __winscard_h__

#include "wintypes.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
    #pragma pack(1)
	#define MAX_ATR_SIZE			33
	typedef int32_t SCARDCONTEXT; 
	typedef SCARDCONTEXT *PSCARDCONTEXT;
	typedef SCARDCONTEXT *LPSCARDCONTEXT;
	typedef int32_t SCARDHANDLE; 
	typedef SCARDHANDLE *PSCARDHANDLE;
	typedef SCARDHANDLE *LPSCARDHANDLE;
	typedef struct
	{
		const char *szReader;
		void *pvUserData;
		uint32_t dwCurrentState;
		uint32_t dwEventState;
		uint32_t cbAtr;
		unsigned char rgbAtr[MAX_ATR_SIZE];
	}
	SCARD_READERSTATE_A;
	
	typedef SCARD_READERSTATE_A SCARD_READERSTATE, *PSCARD_READERSTATE_A,
	*LPSCARD_READERSTATE_A;	
	
	/** Protocol Control Information (PCI) */
	typedef struct _SCARD_IO_REQUEST
	{
		uint32_t dwProtocol;	/**< Protocol identifier */
		uint32_t cbPciLength;	/**< Protocol Control Inf Length */
	}
	SCARD_IO_REQUEST, *PSCARD_IO_REQUEST, *LPSCARD_IO_REQUEST;
	
	typedef const SCARD_IO_REQUEST *LPCSCARD_IO_REQUEST;
	
	extern SCARD_IO_REQUEST g_rgSCardT0Pci, g_rgSCardT1Pci,
	g_rgSCardRawPci;
	
	LONG SCardEstablishContext(DWORD dwScope, LPCVOID pvReserved1,
							   LPCVOID pvReserved2, LPSCARDCONTEXT phContext);
    LONG SCardIsValidContext(SCARDCONTEXT hContext);

	LONG SCardReleaseContext(SCARDCONTEXT hContext);
	
	
	LONG SCardSetTimeout(SCARDCONTEXT hContext, DWORD dwTimeout);
	
	LONG SCardConnect(SCARDCONTEXT hContext, LPCSTR szReader,
					  DWORD dwShareMode, DWORD dwPreferredProtocols, LPSCARDHANDLE phCard,
					  LPDWORD pdwActiveProtocol);
	
	LONG SCardReconnect(SCARDHANDLE hCard, DWORD dwShareMode,
						DWORD dwPreferredProtocols, DWORD dwInitialization,
						LPDWORD pdwActiveProtocol);
	
	LONG SCardDisconnect(SCARDHANDLE hCard, DWORD dwDisposition);
	
	LONG SCardBeginTransaction(SCARDHANDLE hCard);
	
	LONG SCardEndTransaction(SCARDHANDLE hCard, DWORD dwDisposition);
	
	LONG SCardCancelTransaction(SCARDHANDLE hCard);
	
	LONG SCardStatus(SCARDHANDLE hCard, LPSTR mszReaderNames,
					 LPDWORD pcchReaderLen, LPDWORD pdwState,
					 LPDWORD pdwProtocol, LPBYTE pbAtr, LPDWORD pcbAtrLen);
	
	LONG SCardGetStatusChange(SCARDCONTEXT hContext, DWORD dwTimeout,
							  LPSCARD_READERSTATE_A rgReaderStates, DWORD cReaders);
	
    LONG SCardControl(SCARDHANDLE hCard, DWORD dwControlCode,
                      const void *pbSendBuffer, DWORD cbSendLength,
                      void *pbRecvBuffer, DWORD cbRecvLength, LPDWORD lpBytesReturned);	
	LONG SCardTransmit(SCARDHANDLE hCard, LPCSCARD_IO_REQUEST pioSendPci,
					   LPCBYTE pbSendBuffer, DWORD cbSendLength,
					   LPSCARD_IO_REQUEST pioRecvPci, LPBYTE pbRecvBuffer,
					   LPDWORD pcbRecvLength);
    LONG SCardSecTransmit(SCARDHANDLE hCard,LPCBYTE pbSendBuffer, DWORD cbSendLength,LPBYTE pbRecvBuffer,LPDWORD pcbRecvLength);
	LONG SCardListReaderGroups(DWORD hContext,
							   LPCBYTE *mszGroups, LPDWORD *pcchGroups);
	
	LONG SCardListReaders(SCARDCONTEXT hContext, LPCSTR mszGroups,
						  LPSTR mszReaders, LPDWORD pcchReaders);
	
	LONG SCardCancel(SCARDCONTEXT hContext);
	
	
#define SCARD_PCI_T0	(&g_rgSCardT0Pci) /**< protocol control information (PCI) for T=0 */
#define SCARD_PCI_T1	(&g_rgSCardT1Pci) /**< protocol control information (PCI) for T=1 */
#define SCARD_PCI_RAW	(&g_rgSCardRawPci) /**< protocol control information (PCI) for RAW protocol */
	
	/** error codes from http://msdn.microsoft.com/en-us/library/aa924526.aspx
	 */
#define SCARD_S_SUCCESS				0x00000000 /**< No error was encountered. */
#define SCARD_F_INTERNAL_ERROR		0x80100001 /**< An internal consistency check failed. */
#define SCARD_E_CANCELLED			0x80100002 /**< The action was cancelled by an SCardCancel request. */
#define SCARD_E_INVALID_HANDLE		0x80100003 /**< The supplied handle was invalid. */
#define SCARD_E_INVALID_PARAMETER	0x80100004 /**< One or more of the supplied parameters could not be properly interpreted. */
#define SCARD_E_INVALID_TARGET		0x80100005 /**< Registry startup information is missing or invalid. */
#define SCARD_E_NO_MEMORY			0x80100006 /**< Not enough memory available to complete this command. */
#define SCARD_F_WAITED_TOO_LONG		0x80100007 /**< An internal consistency timer has expired. */
#define SCARD_E_INSUFFICIENT_BUFFER	0x80100008 /**< The data buffer to receive returned data is too small for the returned data. */
#define SCARD_E_UNKNOWN_READER		0x80100009 /**< The specified reader name is not recognized. */
#define SCARD_E_TIMEOUT				0x8010000A /**< The user-specified timeout value has expired. */
#define SCARD_E_SHARING_VIOLATION	0x8010000B /**< The smart card cannot be accessed because of other connections outstanding. */
#define SCARD_E_NO_SMARTCARD		0x8010000C /**< The operation requires a Smart Card, but no Smart Card is currently in the device. */
#define SCARD_E_UNKNOWN_CARD		0x8010000D /**< The specified smart card name is not recognized. */
#define SCARD_E_CANT_DISPOSE		0x8010000E /**< The system could not dispose of the media in the requested manner. */
#define SCARD_E_PROTO_MISMATCH		0x8010000F /**< The requested protocols are incompatible with the protocol currently in use with the smart card. */
#define SCARD_E_NOT_READY			0x80100010 /**< The reader or smart card is not ready to accept commands. */
#define SCARD_E_INVALID_VALUE		0x80100011 /**< One or more of the supplied parameters values could not be properly interpreted. */
#define SCARD_E_SYSTEM_CANCELLED	0x80100012 /**< The action was cancelled by the system, presumably to log off or shut down. */
#define SCARD_F_COMM_ERROR			0x80100013 /**< An internal communications error has been detected. */
#define SCARD_F_UNKNOWN_ERROR		0x80100014 /**< An internal error has been detected, but the source is unknown. */
#define SCARD_E_INVALID_ATR			0x80100015 /**< An ATR obtained from the registry is not a valid ATR string. */
#define SCARD_E_NOT_TRANSACTED		0x80100016 /**< An attempt was made to end a non-existent transaction. */
#define SCARD_E_READER_UNAVAILABLE	0x80100017 /**< The specified reader is not currently available for use. */
#define SCARD_P_SHUTDOWN			0x80100018 /**< The operation has been aborted to allow the server application to exit. */
#define SCARD_E_PCI_TOO_SMALL		0x80100019 /**< The PCI Receive buffer was too small. */
#define SCARD_E_READER_UNSUPPORTED	0x8010001A /**< The reader driver does not meet minimal requirements for support. */
#define SCARD_E_DUPLICATE_READER	0x8010001B /**< The reader driver did not produce a unique reader name. */
#define SCARD_E_CARD_UNSUPPORTED	0x8010001C /**< The smart card does not meet minimal requirements for support. */
#define SCARD_E_NO_SERVICE			0x8010001D /**< The Smart card resource manager is not running. */
#define SCARD_E_SERVICE_STOPPED		0x8010001E /**< The Smart card resource manager has shut down. */
#define SCARD_E_UNEXPECTED			0x8010001F /**< An unexpected card error has occurred. */
#define SCARD_E_ICC_INSTALLATION	0x80100020 /**< No primary provider can be found for the smart card. */
#define SCARD_E_ICC_CREATEORDER		0x80100021 /**< The requested order of object creation is not supported. */
	/* #define SCARD_E_UNSUPPORTED_FEATURE	0x80100022 / **< This smart card does not support the requested feature. */
#define SCARD_E_DIR_NOT_FOUND		0x80100023 /**< The identified directory does not exist in the smart card. */
#define SCARD_E_FILE_NOT_FOUND		0x80100024 /**< The identified file does not exist in the smart card. */ 
#define SCARD_E_NO_DIR				0x80100025 /**< The supplied path does not represent a smart card directory. */
#define SCARD_E_NO_FILE				0x80100026 /**< The supplied path does not represent a smart card file. */
#define SCARD_E_NO_ACCESS			0x80100027 /**< Access is denied to this file. */
#define SCARD_E_WRITE_TOO_MANY		0x80100028 /**< The smart card does not have enough memory to store the information. */
#define SCARD_E_BAD_SEEK			0x80100029 /**< There was an error trying to set the smart card file object pointer. */
#define SCARD_E_INVALID_CHV			0x8010002A /**< The supplied PIN is incorrect. */
#define SCARD_E_UNKNOWN_RES_MNG		0x8010002B /**< An unrecognized error code was returned from a layered component. */ 
#define SCARD_E_NO_SUCH_CERTIFICATE	0x8010002C /**< The requested certificate does not exist. */
#define SCARD_E_CERTIFICATE_UNAVAILABLE	0x8010002D /**< The requested certificate could not be obtained. */
#define SCARD_E_NO_READERS_AVAILABLE    0x8010002E /**< Cannot find a smart card reader. */
#define SCARD_E_COMM_DATA_LOST		0x8010002F /**< A communications error with the smart card has been detected. Retry the operation. */
#define SCARD_E_NO_KEY_CONTAINER	0x80100030 /**< The requested key container does not exist on the smart card. */
#define SCARD_E_SERVER_TOO_BUSY		0x80100031 /**< The Smart Card Resource Manager is too busy to complete this operation. */
	
#define SCARD_W_UNSUPPORTED_CARD	0x80100065 /**< The reader cannot communicate with the card, due to ATR string configuration conflicts. */
#define SCARD_W_UNRESPONSIVE_CARD	0x80100066 /**< The smart card is not responding to a reset. */
#define SCARD_W_UNPOWERED_CARD		0x80100067 /**< Power has been removed from the smart card, so that further communication is not possible. */
#define SCARD_W_RESET_CARD			0x80100068 /**< The smart card has been reset, so any shared state information is invalid. */
#define SCARD_W_REMOVED_CARD		0x80100069 /**< The smart card has been removed, so further communication is not possible. */
	
#define SCARD_W_SECURITY_VIOLATION	0x8010006A /**< Access was denied because of a security violation. */
#define SCARD_W_WRONG_CHV			0x8010006B /**< The card cannot be accessed because the wrong PIN was presented. */
#define SCARD_W_CHV_BLOCKED			0x8010006C /**< The card cannot be accessed because the maximum number of PIN entry attempts has been reached. */
#define SCARD_W_EOF					0x8010006D /**< The end of the smart card file has been reached. */
#define SCARD_W_CANCELLED_BY_USER	0x8010006E /**< The user pressed "Cancel" on a Smart Card Selection Dialog. */
#define SCARD_W_CARD_NOT_AUTHENTICATED	0x8010006F /**< No PIN was presented to the smart card. */
	
#define SCARD_SCOPE_USER			0x0000	/**< Scope in user space */
#define SCARD_SCOPE_TERMINAL		0x0001	/**< Scope in terminal */
#define SCARD_SCOPE_SYSTEM			0x0002	/**< Scope in system */
	
#define SCARD_PROTOCOL_UNDEFINED	0x0000	/**< protocol not set */
#define SCARD_PROTOCOL_UNSET SCARD_PROTOCOL_UNDEFINED	/* backward compat */
#define SCARD_PROTOCOL_T0		0x0001	/**< T=0 active protocol. */
#define SCARD_PROTOCOL_T1		0x0002	/**< T=1 active protocol. */
#define SCARD_PROTOCOL_RAW		0x0004	/**< Raw active protocol. */
#define SCARD_PROTOCOL_T15		0x0008	/**< T=15 protocol. */
#define SCARD_PROTOCOL_ANY		(SCARD_PROTOCOL_T0|SCARD_PROTOCOL_T1)	/**< IFD determines prot. */
	
#define SCARD_SHARE_EXCLUSIVE	0x0001	/**< Exclusive mode only */
#define SCARD_SHARE_SHARED		0x0002	/**< Shared mode only */
#define SCARD_SHARE_DIRECT		0x0003	/**< Raw mode only */
	
#define SCARD_LEAVE_CARD		0x0000	/**< Do nothing on close */
#define SCARD_RESET_CARD		0x0001	/**< Reset on close */
#define SCARD_UNPOWER_CARD		0x0002	/**< Power down on close */
#define SCARD_EJECT_CARD		0x0003	/**< Eject on close */
	
#define SCARD_UNKNOWN			0x0001	/**< Unknown state */
#define SCARD_ABSENT			0x0002	/**< Card is absent */
#define SCARD_PRESENT			0x0004	/**< Card is present */
#define SCARD_SWALLOWED			0x0008	/**< Card not powered */
#define SCARD_POWERED			0x0010	/**< Card is powered */
#define SCARD_NEGOTIABLE		0x0020	/**< Ready for PTS */
#define SCARD_SPECIFIC			0x0040	/**< PTS has been set */
	
#define SCARD_STATE_UNAWARE		0x0000	/**< App wants status */
#define SCARD_STATE_IGNORE		0x0001	/**< Ignore this reader */
#define SCARD_STATE_CHANGED		0x0002	/**< State has changed */
#define SCARD_STATE_UNKNOWN		0x0004	/**< Reader unknown */
#define SCARD_STATE_UNAVAILABLE	0x0008	/**< Status unavailable */
#define SCARD_STATE_EMPTY		0x0010	/**< Card removed */
#define SCARD_STATE_PRESENT		0x0020	/**< Card inserted */
#define SCARD_STATE_ATRMATCH	0x0040	/**< ATR matches card */
#define SCARD_STATE_EXCLUSIVE	0x0080	/**< Exclusive Mode */
#define SCARD_STATE_INUSE		0x0100	/**< Shared Mode */
#define SCARD_STATE_MUTE		0x0200	/**< Unresponsive card */
#define SCARD_STATE_UNPOWERED	0x0400	/**< Unpowered card */
	
	/** PC/SC Lite specific extensions */
#define SCARD_W_INSERTED_CARD		0x8010006A
#define SCARD_E_UNSUPPORTED_FEATURE	0x8010001F
	
#define SCARD_SCOPE_GLOBAL		0x0003	/**< Scope is global */
	
#define SCARD_RESET				0x0001	/**< Card was reset */
#define SCARD_INSERTED			0x0002	/**< Card was inserted */
#define SCARD_REMOVED			0x0004	/**< Card was removed */
	
#define	MAX_READERNAME		1
#ifndef INFINITE
#define INFINITE			0xFFFFFFFF	/**< Infinite timeout */
#endif
    
    typedef struct
    {
        uint8_t tag;
        uint8_t length;
        uint32_t value;	/**< This value is always in BIG ENDIAN format as documented in PCSC v2 part 10 ch 2.2 page 2. You can use ntohl() for example */
    } PCSC_TLV_STRUCTURE;
#define SCARD_CTL_CODE(code) (0x42000000 + (code))
    
#define CM_IOCTL_GET_FEATURE_REQUEST SCARD_CTL_CODE(3400)
    
#define FEATURE_VERIFY_PIN_START         0x01
#define FEATURE_VERIFY_PIN_FINISH        0x02
#define FEATURE_MODIFY_PIN_START         0x03
#define FEATURE_MODIFY_PIN_FINISH        0x04
#define FEATURE_GET_KEY_PRESSED          0x05
#define FEATURE_VERIFY_PIN_DIRECT        0x06 /**< Verify PIN */
#define FEATURE_MODIFY_PIN_DIRECT        0x07 /**< Modify PIN */
#define FEATURE_MCT_READERDIRECT         0x08 /**< deprecated */
#define FEATURE_MCT_READER_DIRECT        0x08
#define FEATURE_MCT_UNIVERSAL            0x09
#define FEATURE_IFD_PIN_PROPERTIES       0x0A /**< retrieve properties of the IFD regarding PIN handling */
#define FEATURE_ABORT                    0x0B
#define FEATURE_SET_SPE_MESSAGE          0x0C
#define FEATURE_VERIFY_PIN_DIRECT_APP_ID 0x0D
#define FEATURE_MODIFY_PIN_DIRECT_APP_ID 0x0E
#define FEATURE_WRITE_DISPLAY            0x0F
#define FEATURE_GET_KEY                  0x10
#define FEATURE_IFD_DISPLAY_PROPERTIES   0x11
#define FEATURE_GET_TLV_PROPERTIES       0x12
#define FEATURE_CCID_ESC_COMMAND         0x13
    
#ifdef __cplusplus
}
#endif

#endif
