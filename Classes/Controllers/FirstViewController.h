//
//  FirstViewController.h
//  SmartCard
//
//  Created by Raúl Horacio Valencia Tenorio on 19/03/12.
//  Copyright (c) 2009 RHVT. All rights reserved.
//
//  Class Description: Controller of the First View. Here device
//  connection as well as notifications are handled
//
//	           (\___/)
//             (o\ /o) 
//            /|:.V.:|\
//		      \\::::://
//        -----`"" ""`-----

#import <UIKit/UIKit.h>
#import "wintypes.h"
#import "winscard.h"

//Ennumeration for the card status
typedef enum
{
    NO_READER = 1,
    NO_CARD = 2,
    CARD_READY = 3
}CONEXION_STATUS;

//Bheavior of a text field that displyas information
typedef enum
{
    NO_APPEND = 0,
    APPEND_RESPONSE,
    APPEND_LOG,
    NO_APPEND_RESPONSE,
    NO_APPEND_LOG
} APPEND_TEXFIELD;

//Enummeration for the types of answers recieved
typedef enum
{
    TYPICAL_READ_RESPONSE = 0,
    SFI_2_RECORD,
    SFI_3_RECORD,
    SFI_4_RECORD,
    SFI_5_RECORD
} ReadResponseCases;

//Constant version of tose variables
#define READER_STATUS_NOTIFY @"READER_STATUS_NOTIFY"
#define READER_NAME "iBankSpace Reader0"
#define AID_TYPICAL_OK 0x61
#define SFI_TYPICAL_OK 0x6c

@interface FirstViewController : UIViewController
                                 <UITableViewDataSource, UITableViewDelegate>
{
    IBOutlet __weak UITableView *tblCardData;
    IBOutlet __weak UIButton *btnInfo;
    IBOutlet __weak UIButton *btnEMV;
    IBOutlet __weak UIButton *btnAll;
    IBOutlet __weak UITextView *txtResponse;
    IBOutlet __weak UITextView *txtLog;
    IBOutlet __weak UILabel *lblStatus;
    IBOutlet __weak UISwitch *swtStatus;
    
    //Container for the responses read from the card
    NSMutableArray *arrCardData;
    //Timers for the attached device handling
    NSTimer *tmrReaderStatus;
    NSTimer *tmrCardStatus;
    
    int intCurrentReaderStatus;
    bool bolEnableDisableButtons;
    
    //Context variables of the card reader (address pointer)
	SCARDCONTEXT hContext;
    SCARDHANDLE hCard;
	DWORD dwPref;
	DWORD currentProto;    
}

@property (weak, nonatomic) IBOutlet __weak UITableView *tblCardData;
@property (weak, nonatomic) IBOutlet __weak UIButton *btnInfo;
@property (weak, nonatomic) IBOutlet __weak UIButton *btnEMV;
@property (weak, nonatomic) IBOutlet __weak UIButton *btnAll;
@property (weak, nonatomic) IBOutlet __weak UITextView *txtResponse;
@property (weak, nonatomic) IBOutlet __weak UITextView *txtLog;
@property (weak, nonatomic) IBOutlet __weak UILabel *lblStatus;
@property (weak, nonatomic) IBOutlet __weak UISwitch *swtStatus;
@property (strong, nonatomic) NSMutableArray *arrCardData;
@property (strong, nonatomic) NSTimer *tmrReaderStatus;
@property (strong, nonatomic) NSTimer *tmrCardStatus;
@property (nonatomic) int currentReaderStatus;
@property (nonatomic) SCARDHANDLE hCard;
@property (atomic) SCARDCONTEXT hContext;
@property (nonatomic) DWORD dwPref;
@property (nonatomic) DWORD currentProto;

//Function to localize any string displayed to the user
-(void)localization;
//Here the notifications of any change in the card reader are received (main thread)
-(void)iBSStatsNotify:(NSNotification *)aNotification;
//Here the notifications of any change in the card reader are received (another thread)
-(void)readerStatusThread;
//This enables the context to access the smart card information
-(void)connectCard;
//Disables the access context of the smart card information
-(void)disconnectCard;
//General function to send APDU to the smartcard once connected
-(LONG)sendAPDU:(const uint8_t *)apdu
     lenghtAPDU:(DWORD)apdulen
       response:(uint8_t *)response
  lenthResponse:(DWORD *)reslen;
//General function to send commands to the smartcard once connected
-(NSDictionary *)askToSmartCard:(const uint8_t *)command
                    commandSize:(DWORD)size
                 typeOfResponse:(const char *)statement
                compareResponse:(const uint8_t)aResponse
                responseCommand:(const uint8_t *)readResponseCase
                 responseLenght:(int)responseLenght;
//Reads throughout the list of EMV descriptors to identify which one to use
-(BOOL)selectAID;
//Modifies the Label "lblStatus" in the View
-(void)changeLabelString:(NSString *)str;
//Modifies the switch "lblStatus" in the View
-(void)changeSwitchHidden:(NSNumber *)nmrEnableDisable;
//Modifies all buttons in the View
-(void)changeButtonStatus:(BOOL)enableDisable;
//Modifies the selected textfield in the View
-(void)changeTextFields:(NSString *)anStr
                 append:(APPEND_TEXFIELD)appendString;
//Activates the card  reading if it is inserted
-(IBAction)actionSwitchPowerOn:(id)sender;
//Retrieves the basic information about the smart card only if the AID is known
-(IBAction)actionBasicInfo:(id)sender;
//Retrieves and parses important information about the smart card only if the AID is Visa or Mastercard
-(IBAction)actionEMVInfo:(id)sender;
-(IBAction)actionRetreiveAll:(id)sender;


@end
