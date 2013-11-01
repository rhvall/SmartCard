//
//  FirstViewController.m
//  SmartCard
//
//  Created by Raúl Horacio Valencia Tenorio on 19/03/12.
//  Copyright (c) 2009 RHVT. All rights reserved.
//  More info: http://blog.saush.com/2006/09/08/getting-information-from-an-emv-chip-card/
//  More info: http://www.openscdp.org/scripts/tutorial/emv/reademv.html
//
//  Class Description: Controller of the First View. Here device
//  connection as well as notifications are handled
//
//	           (\___/)
//             (o\ /o) 
//            /|:.V.:|\
//		      \\::::://
//        -----`"" ""`-----


#import "FirstViewController.h"
#import "CardBytesAnalysis.hpp"

#include "TLVDecode.h"
#include "CardList.h"

@implementation FirstViewController

@synthesize tblCardData;
@synthesize btnInfo;
@synthesize btnEMV;
@synthesize btnAll;
@synthesize txtResponse;
@synthesize txtLog;
@synthesize lblStatus;
@synthesize swtStatus;
@synthesize arrCardData;
@synthesize tmrReaderStatus;
@synthesize tmrCardStatus;
@synthesize currentReaderStatus;
@synthesize hCard;
@synthesize hContext;
@synthesize dwPref;
@synthesize currentProto;

//----------------------------------------
//	INITIALIZATION
//----------------------------------------
#pragma mark -
#pragma mark INITIALIZATION

-(id)initWithNibName:(NSString *)nibNameOrNil
              bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil
                           bundle:nibBundleOrNil];
    if (self)
    {
        //This variable holds the response value
        LONG rv = SCARD_S_SUCCESS;
        currentReaderStatus = SCARD_STATE_UNAWARE;
        [self changeLabelString:NSLocalizedString(@"Please insert reader", "Insert status in First View Controller")];

        //There is no context initialized
        if(hContext == 0)
        {
            NSString *aStr;
            //Ask the device if a context can be pointed from this program
            rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
            if(rv != SCARD_S_SUCCESS)
            {
                aStr = [NSString stringWithFormat:NSLocalizedString(@"Error SCardEstablishContext:0x%x", @"Error from Card Stablishing Context in First View Controller"), rv];
            }
            else
            {
                aStr = [NSString stringWithFormat:NSLocalizedString(@"Card Establish Context 0x%x", @"Card Stablishing Context in First View Controller"), hContext];               
            }
            
            [self changeLabelString:aStr];
        }
        
        //Attach this controller to any change in notifications from the card reader
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(iBSStatsNotify:)
                                                     name:READER_STATUS_NOTIFY 
                                                   object:nil];

        //Create a new thread to handle any reader status change
        [NSThread detachNewThreadSelector:@selector(readerStatusThread)
                                 toTarget:self
                               withObject:nil];
        
        //Arrays to hold data from the card
        arrCardData = [[NSMutableArray alloc] init];
    }
    
    return self;
}

//----------------------------------------
//	MEMORY MANAGEMENT
//----------------------------------------
#pragma mark -
#pragma mark MEMORY MANAGEMENT
							
-(void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Release any cached data, images, etc that aren't in use.
}

-(void)dealloc
{
    //Even when ARC is used, it is necessary to invalidate any pointer to the Card reader
    if ([tmrReaderStatus isValid])
    {
        [tmrReaderStatus invalidate];
    }
    
    if ([tmrCardStatus isValid])
    {
        [tmrCardStatus invalidate];
    }
    
    if(hContext !=  0)
	{
		SCardReleaseContext(hContext);
		hContext = 0;
		hCard = 0;
	}
    
    //Remove this controller from the notification center
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:READER_STATUS_NOTIFY
                                                  object:nil];
}

//----------------------------------------
//	VIEW LIFECYCLE
//----------------------------------------
#pragma mark -
#pragma mark VIEW LIFECYCLE

-(void)viewDidLoad
{
    [super viewDidLoad];
    
    [self localization];
    //Set buttons and the switch to hidden
    [self changeButtonStatus:NO];
    [self changeSwitchHidden:[NSNumber numberWithBool:YES]];
    [swtStatus setOn:NO];
    [self changeTextFields:nil
                    append:NO_APPEND];
    
    //This call is required when test on the simulator take place
    [self actionEMVInfo:nil];
}

-(void)viewDidUnload
{
    [super viewDidUnload];

    [self changeButtonStatus:NO];
    [self changeSwitchHidden:[NSNumber numberWithBool:YES]];
    [swtStatus setOn:NO];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

-(void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
}

-(void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
}

-(void)viewWillDisappear:(BOOL)animated
{
	[super viewWillDisappear:animated];
    
    [self changeButtonStatus:NO];
    [self changeSwitchHidden:[NSNumber numberWithBool:YES]];
    [swtStatus setOn:NO];
}

-(void)viewDidDisappear:(BOOL)animated
{
	[super viewDidDisappear:animated];
}

//----------------------------------------
//	ORIENTATION AWARENESS
//----------------------------------------
#pragma mark -
#pragma mark ORIENTATION AWARENESS

-(BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone)
    {
        return (interfaceOrientation != UIInterfaceOrientationPortraitUpsideDown);
    }
    else
    {
        return YES;
    }
}

//----------------------------------------
//	LOCALIZATION
//----------------------------------------
#pragma mark -
#pragma mark LOCALIZATION

-(void)localization
{
    self.title = NSLocalizedString(@"Card Reader Example", @"Title in First View Controller");
    
    [lblStatus setText:NSLocalizedString(@"Status: None", @"Initial text to label Status in First View Controller")];
    [btnInfo setTitle:NSLocalizedString(@"Basic Information", @"Information button in First View Controller")
               forState:UIControlStateNormal];
    [btnEMV setTitle:NSLocalizedString(@"EMV Details", @"Balance button in First View Controller")
               forState:UIControlStateNormal];
    [btnAll setTitle:NSLocalizedString(@"*Everything*", @"Credit button in First View Controller")
             forState:UIControlStateNormal];
}

//----------------------------------------
//  TABLEVIEW LIFECYCLE
//----------------------------------------
#pragma mark -
#pragma mark TABLEVIEW LIFECYCLE

-(UITableViewCell *)tableView:(UITableView *)tableView
        cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *cellIdentifier = @"elementCell";
    
    UITableViewCell *aCellElement = [tableView dequeueReusableCellWithIdentifier:cellIdentifier];
    
    if (aCellElement == nil)
    {
        aCellElement = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1
                                              reuseIdentifier:@"cell"];
    }
    
    aCellElement.textLabel.text = [arrCardData objectAtIndex:indexPath.row];
    aCellElement.accessoryType = UITableViewCellAccessoryNone;
    
    return aCellElement;
}

-(NSInteger)tableView:(UITableView *)tableView
numberOfRowsInSection:(NSInteger)section
{
    return [arrCardData count];
}

//----------------------------------------
//	NOTIFICATIONS
//----------------------------------------
#pragma mark -
#pragma mark NOTIFICATIONS

-(void)iBSStatsNotify:(NSNotification *)aNotification
{
    int status = [[[aNotification userInfo] objectForKey:@"status"] intValue];
    BOOL bolCardReader = NO;
    NSString *strStatus;
    
    //It makes the selection according to the received notification (LibiBSreader)
    switch (status)
    {
        case NO_READER:
        {
			strStatus = NSLocalizedString(@"Please insert reader", "No reader in First View Controller");
            bolCardReader = NO;
            break;
        }
        case NO_CARD:
        {
			strStatus = NSLocalizedString(@"Please insert card", "No card in First View Controller");
            bolCardReader = NO;
            break;
        }
        case CARD_READY:
        {
            strStatus = NSLocalizedString(@"Switch Power button to start\n Switch Power button to off when remove card", @"Card inserted in First View Controller");
            bolCardReader = YES;
            break;
        }
        default:
        {
            strStatus = NSLocalizedString(@"Unknown card status", "Unknown card status in First View Controller");
            bolCardReader = NO;
            break;
        }
    }
    
    //It is not certain that this function runs on the main thread.
    if (status != CARD_READY)
    {
        [self performSelectorOnMainThread:@selector(actionSwitchPowerOn:)
                               withObject:nil
                            waitUntilDone:NO];
    }
    
    [self performSelectorOnMainThread:@selector(changeLabelString:)
                           withObject:strStatus
                        waitUntilDone:NO];
    [self performSelectorOnMainThread:@selector(changeSwitchHidden:)
                           withObject:[NSNumber numberWithBool:bolCardReader]
                        waitUntilDone:NO];
}

-(void)readerStatusThread
{
@autoreleasepool
{
	LONG rv;
	const char *mszGroups;
	int readerCount = 0;
	SCARD_READERSTATE_A rgReaderStates[1];
    //This function reads the pointer to the context to get updates on the Card
    //Here all posible states are considered
	rv = SCardGetStatusChange(hContext,
                              INFINITE,
                              0,
                              readerCount);
    
	NSMutableDictionary *dicInformation = [[NSMutableDictionary alloc] init];

	if(rv != SCARD_S_SUCCESS)
    {
        [dicInformation setObject:[NSNumber numberWithInt:NO_READER]
                           forKey:@"status"];
    }
    else
    {
        [dicInformation setObject:[NSNumber numberWithInt:NO_CARD]
                           forKey:@"status"];
    }
	
    [[NSNotificationCenter defaultCenter] postNotificationName:READER_STATUS_NOTIFY
                                                        object:nil
                                                      userInfo:dicInformation];

	//Run this thread while a valid context is in place
    while(hContext)
	{


        mszGroups = 0;
        readerCount = 1;
        rgReaderStates[0].szReader = READER_NAME;//&mszReaders[0];
        rgReaderStates[0].dwCurrentState = SCARD_STATE_EMPTY;
        rgReaderStates[0].dwEventState = SCARD_STATE_EMPTY;
        
        //Here only the states before metioned are considered to tigger a notification, with a
        //timer of 10 to the primary reader.
		rv = SCardGetStatusChange(hContext,
                                  10,
                                  rgReaderStates,
                                  readerCount);
		
		if(rv != SCARD_S_SUCCESS)
		{
			readerCount = 0;
			rgReaderStates[0].dwCurrentState = SCARD_STATE_EMPTY;
            
			[dicInformation setObject:[NSNumber numberWithInt:NO_READER]
                               forKey:@"status"];
			[[NSNotificationCenter defaultCenter] postNotificationName:READER_STATUS_NOTIFY
                                                                object:nil
                                                              userInfo:dicInformation];
		}
        else
        {
			readerCount = 1;
            
            NSString  *aStr = [NSString stringWithFormat:@"\nEvent: %x\tCurrent:%x", rgReaderStates[0].dwEventState, rgReaderStates[0].dwCurrentState];
            
            [self performSelectorOnMainThread:@selector(changeTextFields:append:)
                                   withObject:aStr
                                waitUntilDone:NO];
            
            //Whether it is present or not, the notification is published
            
			if(rgReaderStates[0].dwEventState & SCARD_STATE_PRESENT)
			{
				rgReaderStates[0].dwCurrentState = SCARD_STATE_PRESENT;
                
				[dicInformation setObject:[NSNumber numberWithInt:CARD_READY]
                                   forKey:@"status"];
				[[NSNotificationCenter defaultCenter] postNotificationName:READER_STATUS_NOTIFY
                                                                    object:nil
                                                                  userInfo:dicInformation];
//                [self performSelectorOnMainThread:@selector(changeTextFields:append:)
//                                       withObject:@"SCARD STATE PRESENT"
//                                    waitUntilDone:NO];
			}
            else
            {
				rgReaderStates[0].dwCurrentState = SCARD_STATE_EMPTY;
				
				[dicInformation setObject:[NSNumber numberWithInt:NO_CARD]
                                   forKey:@"status"];
				[[NSNotificationCenter defaultCenter] postNotificationName:READER_STATUS_NOTIFY
                                                                    object:nil
                                                                  userInfo:dicInformation];
                [self performSelectorOnMainThread:@selector(changeTextFields:append:)
                                       withObject:@"SCARD STATE EMPTY"
                                    waitUntilDone:NO];
			}
		}
		
		usleep(1000);
	}
}
}

//----------------------------------------
//	CARD READER HANDLING
//----------------------------------------
#pragma mark -
#pragma mark CARD READER HANDLING

-(void)connectCard
{
	LONG rv;
    NSMutableString *str = [[NSMutableString alloc] init];
    [str appendString:NSLocalizedString(@"\nPower on Smartcard", @"Power the card in First View Controller")];
    
    //Connect to the smart card and get the ATR
    //Sets the pointers to the Card and its preferences
	rv = SCardConnect(hContext,
                      READER_NAME,
					  //SCARD_SHARE_EXCLUSIVE,  //Types of conecction (libiBSreader)
					  //SCARD_SHARE_DIRECT,
					  SCARD_SHARE_SHARED,
                      SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
					  &hCard,
                      &dwPref);
    
	if(rv != SCARD_S_SUCCESS)
	{
        [str appendFormat:NSLocalizedString(@"\nSCardConnect Error: %x", @"Error powering the card in First View Controller"),rv];
	}
	else
	{
        if(hCard)
		{
			DWORD dwReaderLen = MAX_READERNAME;
			char *pcReaders = (char *)malloc(sizeof(char) * MAX_READERNAME);
			DWORD dwAtrLen = MAX_ATR_SIZE;
			DWORD dwState;
            DWORD dwProt;
			unsigned char pbAtr[MAX_ATR_SIZE];
			
            //With a successfull connection, it is important to get the status of the smart card
            //In other words, the ATR and the protocol of communication
			rv = SCardStatus(hCard,
                             pcReaders,
                             &dwReaderLen,
                             &dwState,
                             &dwProt,
							 pbAtr,
                             &dwAtrLen);
			currentProto = dwProt;
            [str appendFormat:@"\nATR: %@",[NSData dataWithBytes:pbAtr
                                                          length:dwAtrLen]];
            [str appendFormat:@"\nProtocol: %d",(int)(currentProto - 1)];

            if([self selectAID] == YES)
            {
                //If the card falls into the list of AID, then it is possible to stablish
                //further communication
                [self changeButtonStatus:YES];
            }
            
            free(pcReaders);
		}
    }

    [self changeTextFields:str
                    append:APPEND_RESPONSE];
}

-(void)disconnectCard
{
    if(hCard)
	{
		SCardDisconnect(hCard, SCARD_RESET_CARD);
		hCard=0;
	}
    
    [self changeButtonStatus:NO];
    [self changeTextFields:nil
                    append:NO_APPEND];
}

-(LONG)sendAPDU:(const uint8_t *)apdu
     lenghtAPDU:(DWORD)apdulen
       response:(uint8_t *)response
  lenthResponse:(DWORD *)reslen
{
	LONG rv;
	SCARD_IO_REQUEST pioRecvPci;
 	SCARD_IO_REQUEST pioSendPci;
    
    //Stablish the protocol of communication
    switch(currentProto)
	{
		case SCARD_PROTOCOL_T0:
        {
			pioSendPci = *SCARD_PCI_T0;
			break;
		}
        case SCARD_PROTOCOL_T1:
        {
			pioSendPci = *SCARD_PCI_T1;
			break;
		}
        default:
		{
            break;
        }
	}

    NSString *aStr;
    //Sends the APDU command and waits for the smart card response
	rv = SCardTransmit(hCard,
                       &pioSendPci,
                       apdu,
                       apdulen,
                       &pioRecvPci,
                       response,
                       reslen);

    if(rv != SCARD_S_SUCCESS)
    {
        aStr = [NSString stringWithFormat:NSLocalizedString(@"\nSCardTransmit Error:%x", @"Error from Card Reading in First View Controller"), rv];
    }
    else
    {   
        aStr = [NSString stringWithFormat:NSLocalizedString(@"\nCard Read Sucessfully: %x", @"Card Stablishing Context in First View Controller"), reslen];
    }
    
    [self changeTextFields:aStr
                    append:APPEND_LOG];
//    [self changeLabelString:aStr];

    return rv;
}

-(NSDictionary *)askToSmartCard:(const uint8_t *)command
                    commandSize:(DWORD)size
                 typeOfResponse:(const char *)statement
                compareResponse:(const uint8_t)aResponse
                responseCommand:(const uint8_t *)readResponseCase
                 responseLenght:(int)responseLenght

{
    NSMutableString *logString = [[NSMutableString alloc] init];
    NSMutableDictionary *aDic = [[NSMutableDictionary alloc] init];
    NSData *responseData = nil;

    uint8_t response[256];
    uint8_t *readResponse;
    uint8_t readResponseSize = 5;
    
    //It is possible to modify which is the expected response
    if (readResponseCase != NULL)
    {
        readResponseSize = responseLenght;
        readResponse = (uint8_t *)malloc(readResponseSize * sizeof(uint8_t));
        
        for (int i = 0; i < responseLenght; i++)
        {
            readResponse[i] = readResponseCase[i];
        }
    }
    else
    {
        //This is the general case response
        readResponse = (uint8_t *)malloc(readResponseSize * sizeof(uint8_t));
        memset(readResponse, 0, readResponseSize * sizeof(uint8_t));
        readResponse[1] = 0xc0;
    }
    
    DWORD reslen = 256;
    LONG rv;
    
    BOOL somethingFound = NO;

    //Calls the APDU function of the controller
    //Reference passing of the response and its lenght
    rv = [self sendAPDU:command
             lenghtAPDU:size
               response:response
          lenthResponse:&reslen];
    
    if(rv == SCARD_S_SUCCESS)
    {
        //This is a successful response
        if(response[reslen - 2] == 0x90 && response[reslen - 1] == 0x00)
        {
            responseData = [NSData dataWithBytes:response
                                                  length:reslen];
            [logString appendFormat:NSLocalizedString(@"\nCard %s first Response:\n%@", "Card second Response in First View Controller"),
             statement,
             responseData];
            somethingFound = YES;
            [aDic setObject:responseData
                     forKey:RESPONSEDATA];
        }
        //The response is not successful, but it says the correct lenght of it
        else if(response[reslen - 2] == aResponse)
        {
            readResponse[4] = response[reslen - 1];
            reslen = 256;
            
            //Call again with the updated response expected command
            rv = [self sendAPDU:readResponse
                     lenghtAPDU:readResponseSize //(DWORD)sizeof(readResponse)
                       response:response
                  lenthResponse:&reslen];
            
            if(rv == SCARD_S_SUCCESS)
            {
                if(response[reslen - 2] == 0x90 && response[reslen - 1] == 0x00)
                {
                    responseData = [NSData dataWithBytes:response
                                                  length:reslen];
                    [logString appendFormat:NSLocalizedString(@"\nCard %s Second Response:\n%@", "Card second Response in First View Controller"),
                     statement,
                     responseData];
                    somethingFound = YES;
                    [aDic setObject:responseData
                             forKey:RESPONSEDATA];
                }
            }
        }
    }
    
    if (somethingFound == NO)
    {
        responseData = [NSData dataWithBytes:response
                                      length:reslen];
        [logString appendFormat:NSLocalizedString(@"\nNo format Found, Response:\n%@", @"Error Searching for Card format in First View Controller"),
                     responseData];
    }
    
    //Dictionary with the variables result from the query of the command
    //It does not matter if it was successfull
    [aDic setObject:logString
             forKey:LOGSTRING];
    [aDic setObject:responseData
             forKey:RESPONSEDATA];
    [aDic setObject:[NSNumber numberWithBool:somethingFound]
             forKey:CORRECTRESPONSE];
    
    free(readResponse);
    return aDic;
}

-(BOOL)selectAID
{
    CardList aCardList;
    NSDictionary *aDic = nil;
 
    //Iterates through the known AID
    for (int i = 0; i < CARD_LIST_NUMBER; i++)
    {
        DWORD size = (DWORD)aCardList.cardAIDSize[i];
        const uint8_t *cardAID = aCardList.cardAID[i];
        aDic = [self askToSmartCard:cardAID
                        commandSize:size
                     typeOfResponse:aCardList.cardAIDStatement[i]
                    compareResponse:AID_TYPICAL_OK
                    responseCommand:NULL
                     responseLenght:0];
        if ([[aDic objectForKey:CORRECTRESPONSE] boolValue] == YES)
        {   
            [self changeTextFields:[aDic objectForKey:LOGSTRING]
                            append:APPEND_RESPONSE];
            return YES;
        }
    }
    return NO;
}

//----------------------------------------
//	FUNCTIONALITY
//----------------------------------------
#pragma mark -
#pragma mark FUNCTIONALITY

-(void)changeLabelString:(NSString *)str
{
    [lblStatus setText:str];
}

-(void)changeSwitchHidden:(NSNumber *)nmrEnableDisable
{
    BOOL enableDisable = [nmrEnableDisable boolValue];
    [swtStatus setHidden:!enableDisable];
}

-(void)changeButtonStatus:(BOOL)enableDisable
{
    enableDisable = !enableDisable;
    [btnInfo setHidden:enableDisable];
    [btnEMV setHidden:enableDisable];
    [btnAll setHidden:enableDisable];
}

-(void)changeTextFields:(NSString *)anStr
                 append:(APPEND_TEXFIELD)appendString
{
    if (anStr == nil)
    {
        [txtLog setText:@""];
        [txtResponse setText:@""];
    }
    else
    {
        //Selects which Text field the text should be added
        switch (appendString)
        {
            case APPEND_RESPONSE:
            {
                NSString *appendedString = [[txtResponse text] stringByAppendingString:anStr];
                [txtResponse setText:appendedString];
                break;
            }
            case APPEND_LOG:
            {
                NSString *appendedString = [[txtLog text] stringByAppendingString:anStr];
                [txtLog setText:appendedString];
                break;
            }
            case NO_APPEND_RESPONSE:
            {
                [txtResponse setText:anStr];
                break;
            }
            case NO_APPEND_LOG:
            {
                [txtLog setText:anStr];
                break;
            }
            default:
            {
                NSString *appendedString = [[txtLog text] stringByAppendingString:anStr];
                [txtLog setText:appendedString];
                break;
            }
        }
    }
 }

-(IBAction)actionSwitchPowerOn:(id)sender
{
    if([swtStatus isOn] == YES && sender == swtStatus)
	{
		[self connectCard];
	}
    else
    {
        //When the card is disconnected, many functions must be called to
        //disable UI elements as well as the connection to it
		[self disconnectCard];
        [self changeTextFields:nil
                        append:NO_APPEND];
        [self changeButtonStatus:NO];
        [arrCardData removeAllObjects];
        [tblCardData reloadData];
	}
}

-(IBAction)actionBasicInfo:(id)sender
{
    //Command to get the basic information (AIP & APL)
    uint8_t cmd[] = {0x80,0xa8,0x00,0x00,0x02,0x83,0x00};
    NSDictionary *aDic = nil;
    
    aDic =  [self askToSmartCard:cmd
                     commandSize:(DWORD)sizeof(cmd)
                  typeOfResponse:"AIP and APL"
                 compareResponse:AID_TYPICAL_OK
                 responseCommand:NULL
                  responseLenght:0];
    
    if ([[aDic objectForKey:CORRECTRESPONSE] boolValue] == YES)
    {
        [self changeTextFields:[aDic objectForKey:LOGSTRING]
                        append:APPEND_RESPONSE];
    }
    
    //Example of an SFI record with 0x02 number
    uint8_t cmd1[] = {0x00,0xb2,0x02,0x0c,0x00};
    aDic =  [self askToSmartCard:cmd1
                     commandSize:(DWORD)sizeof(cmd)
                  typeOfResponse:"SFI 2 Record"
                 compareResponse:SFI_TYPICAL_OK
                 responseCommand:cmd1
                  responseLenght:sizeof(cmd1)];
    
    if ([[aDic objectForKey:CORRECTRESPONSE] boolValue] == YES)
    {
        [self changeTextFields:[aDic objectForKey:LOGSTRING]
                        append:APPEND_RESPONSE];
    }
}

-(IBAction)actionEMVInfo:(id)sender
{
    NSMutableData *dtaResponses = [[NSMutableData alloc] init];
    CardList aCardList;
    
#if TARGET_IPHONE_SIMULATOR
    uint8_t cmd[] = {0x70,0x2d,0x57,0x10,0x40,0x59,0x30,0x35,0x71,0x46,0x39,0x91,0xd1,0x60,0x32,0x01,0x00,0x00,0x07,0x12,0x9f,0x1f,0x18,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x37,0x31,0x32,0x30,0x30,0x30,0x30,0x30,0x30,0x90,0x00,0x70,0x0c,0x5f,0x25,0x03,0x11,0x03,0x01,0x5f,0x24,0x03,0x16,0x03,0x31,0x90,0x00,0x70,0x0e,0x5a,0x08,0x40,0x59,0x30,0x35,0x71,0x46,0x39,0x91,0x5f,0x34,0x01,0x00,0x90,0x00,0x70,0x3b,0x9f,0x4a,0x01,0x82,0x9f,0x42,0x02,0x04,0x84,0x9f,0x51,0x02,0x04,0x84,0x9f,0x08,0x02,0x00,0x8c,0x9f,0x44,0x01,0x02,0x9f,0x57,0x02,0x04,0x84,0x5f,0x30,0x02,0x02,0x01,0x5f,0x20,0x17,0x4f,0x52,0x54,0x49,0x5a,0x20,0x48,0x45,0x52,0x4e,0x41,0x4e,0x44,0x45,0x5a,0x2f,0x41,0x4c,0x42,0x45,0x52,0x54,0x4f,0x90,0x00,0x70,0x05,0x9f,0x07,0x02,0xff,0xc0,0x90,0x00,0x70,0x12,0x8e,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1e,0x03,0x42,0x01,0x02,0x04,0x1f,0x03,0x90,0x00,0x70,0x18,0x9f,0x0d,0x05,0xf0,0x50,0x04,0x08,0x00,0x9f,0x0e,0x05,0x00,0x00,0x88,0x00,0x00,0x9f,0x0f,0x05,0xf0,0x70,0x04,0x98,0x00,0x90,0x00,0x70,0x05,0x5f,0x28,0x02,0x04,0x84,0x90,0x00,0x70,0x08,0x9f,0x14,0x01,0x00,0x9f,0x23,0x01,0x00,0x90,0x00,0x70,0x30,0x8c,0x15,0x9f,0x02,0x06,0x9f,0x03,0x06,0x9f,0x1a,0x02,0x95,0x05,0x5f,0x2a,0x02,0x9a,0x03,0x9c,0x01,0x9f,0x37,0x04,0x8d,0x17,0x8a,0x02,0x9f,0x02,0x06,0x9f,0x03,0x06,0x9f,0x1a,0x02,0x95,0x05,0x5f,0x2a,0x02,0x9a,0x03,0x9c,0x01,0x9f,0x37,0x04,0x90,0x00,0x70,0x23,0x9f,0x05,0x20,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x90,0x00,0x70,0x81,0xb3,0x90,0x81,0xb0,0x92,0x60,0xcf,0xfd,0x1e,0xf0,0x0a,0x7a,0x20,0x63,0x68,0x6d,0x57,0x52,0xfd,0xc1,0x51,0x0e,0x1a,0x25,0x37,0xb2,0x02,0x09,0xd1,0xb8,0x22,0x18,0xb3,0xbb,0x6c,0x1e,0x7b,0xd6,0x5c,0x63,0xc1,0x6f,0xfd,0xce,0x84,0x34,0x0b,0x6c,0x4a,0xdb,0xaa,0x59,0x99,0xea,0xcc,0xfd,0x35,0xc2,0x6a,0x16,0xa8,0x37,0xeb,0x38,0x7f,0xa5,0x79,0x84,0xf2,0xb3,0x92,0x40,0xb8,0x23,0xd4,0x53,0x70,0xf2,0x65,0x43,0x58,0x60,0x0b,0xc8,0x5b,0x55,0x9e,0xec,0xbd,0x34,0x10,0x72,0xb5,0x1f,0x72,0xea,0x9a,0xd5,0x89,0x68,0x60,0x9a,0x12,0xfd,0xc2,0x6f,0x89,0x0e,0xa1,0x49,0x36,0xb0,0xe4,0xe1,0x1f,0x22,0xf4,0x01,0x70,0x25,0xba,0xe1,0xd7,0x4b,0x4a,0x6c,0x64,0x09,0x20,0x97,0x15,0x73,0xc0,0x42,0xb6,0xa1,0x30,0x36,0x76,0xdb,0xe4,0x2f,0x78,0x5c,0x73,0xb0,0x34,0xf5,0x7d,0x5e,0x9b,0x72,0x19,0xd0,0xc0,0xe8,0x17,0x13,0xdc,0x2a,0xaa,0x90,0x3d,0x02,0xa5,0xf1,0xaf,0x23,0xbe,0xe4,0xb0,0xc3,0xc2,0xb6,0x08,0x04,0x0c,0x99,0x2b,0xfe,0x90,0x00,0x70,0x2d,0x8f,0x01,0x08,0x92,0x24,0x09,0xea,0x0f,0x22,0x72,0x0e,0x9b,0x31,0x17,0x70,0x1a,0x42,0xeb,0xcd,0x81,0x8a,0x08,0x06,0x4b,0x5c,0x66,0xaa,0x85,0x98,0x94,0x7f,0xea,0x11,0x4f,0x90,0x7e,0xbd,0xf9,0x0b,0xaa,0xc9,0x9f,0x32,0x01,0x03,0x90,0x00,0x70,0x81,0xb3,0x93,0x81,0xb0,0x72,0xa8,0xf5,0xa6,0x81,0x00,0x85,0x2b,0x56,0x59,0x60,0x20,0xc7,0x83,0xc3,0x79,0x12,0xa3,0xba,0x30,0x04,0x75,0xea,0xb1,0xba,0x01,0xef,0x0b,0x72,0x73,0x1b,0x57,0x5f,0x7d,0x38,0xc9,0x87,0xc5,0xc1,0x83,0xd7,0xd9,0x61,0xfd,0x10,0xbc,0x24,0x92,0x46,0x93,0xbd,0xe7,0x80,0x50,0xda,0xae,0xd7,0x9f,0xea,0x2f,0xba,0x4f,0x73,0xa3,0xda,0x20,0xde,0x51,0x08,0x12,0xc8,0x43,0xfa,0x3e,0x09,0x0b,0xa2,0x7a,0x04,0xad,0x8d,0x9a,0x36,0xd5,0x6d,0xa0,0x02,0x75,0xff,0xf5,0xe4,0xe7,0x3c,0x7b,0x33,0x44,0x0a,0x78,0x04,0x05,0x3d,0x75,0x02,0x8e,0xbc,0xda,0x88,0x1f,0xc3,0xaa,0xd6,0xf4,0x45,0xed,0x53,0xa1,0x69,0xfc,0x41,0x32,0x3b,0xed,0xc9,0xea,0xaf,0xfe,0xa0,0x14,0x79,0xe7,0xe3,0xe2,0xd5,0x7a,0xde,0xb1,0xa8,0x94,0xb8,0xfd,0x52,0xc9,0xb4,0xbf,0xdd,0xbd,0x07,0xb3,0x90,0xd6,0xb7,0xf2,0xb0,0x36,0xd0,0xa2,0x55,0x9f,0xf1,0xc3,0xf0,0x90,0x85,0x91,0x40,0xe0,0x32,0xec,0x1f,0x23,0xd3,0xca,0xdc,0xc8,0x1f,0x13,0x90,0x00};
    [dtaResponses appendBytes:cmd
                       length:sizeof(cmd)];
#else
    //Loop through the number of SFI which has been detected information
    //for Visa and Mastercard File descriptors
    for (int i = 0; i < SFI_LIST_NUMBER; i++)
    {
        const uint8_t *cmd = aCardList.sfiCommand[i];
        DWORD cmdSize = (DWORD)aCardList.sfiSize;
        //uint8_t cmd[] = {0x00,0xb2,0x01,0x0C,0x00};
        //uint8_t cmd[] = {0x00,0xb2,0x01,0x0C,0x00};
        
        NSDictionary *aDic = nil;
        
        aDic =  [self askToSmartCard:cmd
                         commandSize:cmdSize
                      typeOfResponse:"SFI Record"
                     compareResponse:SFI_TYPICAL_OK
                     responseCommand:cmd
                      responseLenght:cmdSize];
        
        if ([[aDic objectForKey:CORRECTRESPONSE] boolValue] == YES)
        {
            NSData *dtaSingle = [aDic objectForKey:RESPONSEDATA];
            [dtaResponses appendBytes:[dtaSingle bytes]
                               length:[dtaSingle length]];
        }
    }
#endif
    
    //If we get more than 0 responses of the file descriptor...
    if ([dtaResponses length] > 0)
    {
        [self changeTextFields:[NSString stringWithFormat:@"%@", dtaResponses]
                        append:APPEND_RESPONSE];
        //Parse data into readable or understandable manner
        vector<string> cardDetails = getCardData([dtaResponses bytes], [dtaResponses length]);
        
        int cdSize = cardDetails.size();
        if (cdSize > 0)
        {
            for (int i = 0; i < cdSize; i++)
            {
                string cardName = cardDetails[i];
                
                NSString *str = [NSString stringWithCString:cardName.c_str()
                                                   encoding:NSUTF8StringEncoding];
                if (str != nil)
                {
                    //Add the understandable data got from the Hexadecimal response
                    [arrCardData addObject:str];
                }
            }
            
            //Display new data
            [tblCardData reloadData];
        }

    }
}

-(IBAction)actionRetreiveAll:(id)sender
{
    for (int i = 1; i <= 31; i++)
    {
        for (int j = 1; j <= 16; j++)
        {
            //Loop through all possible SFI that the card could response
            uint8_t cmd[] = {0x00, 0xb2, j, (i << 3) | 4, 0x00};
            NSDictionary *aDic = nil;
            NSData *aDat = [NSData dataWithBytes:cmd
                                          length:sizeof(cmd)];
            NSString *aStr = [NSString stringWithFormat:@"\n\nSFI: %i; record #%i \n Command:%@", i, j, aDat];
            [self changeTextFields:aStr
                            append:APPEND_RESPONSE];
    
            aDic =  [self askToSmartCard:cmd
                             commandSize:(DWORD)sizeof(cmd)
                          typeOfResponse:"SFI Record"
                         compareResponse:SFI_TYPICAL_OK
                         responseCommand:cmd
                          responseLenght:sizeof(cmd)];
            
            if ([[aDic objectForKey:CORRECTRESPONSE] boolValue] == YES)
            {
                [self changeTextFields:[aDic objectForKey:LOGSTRING]
                                append:APPEND_RESPONSE];
            }
        }
    }
}

@end
