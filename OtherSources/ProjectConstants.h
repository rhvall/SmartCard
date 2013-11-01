//
//  ProjecConstans.h
//  SmartCard
//
//  Created by Raúl Horacio Valencia Tenorio on 19/03/12.
//  Copyright (c) 2009 RHVT. All rights reserved.
//
//  Class Description: This is a container for the most common
//  constants used throughout this project
//
//	           (\___/)
//             (o\ /o) 
//            /|:.V.:|\
//		      \\::::://
//        -----`"" ""`-----

#import "AppDelegate.h"

#ifdef DEBUG
//#    define DLog(...) NSLog(__VA_ARGS__)
//#define DLog(fmt, ...) NSLog((@"%s [Line %d] " fmt), __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__);
#define DLog(fmt, ...) NSLog((@"[%d] " fmt),__LINE__, ##__VA_ARGS__);
#else
#define DLog(...) /* */
#endif

#define SLog(obj) DLog(@"%@",obj);
#define ILog(obj) DLog(@"%d",obj);
#define FLog(obj) DLog(@"%f",obj);
#define SSLog(obj,obj2) DLog(@"%@: %@", obj, obj2);
#define isPad		(UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
#define isPod		(UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone)
#define screenSize	[[UIScreen mainScreen] bounds].size
#define barPosition [UIApplication sharedApplication].statusBarOrientation
#define landscape	UIInterfaceOrientationIsLandscape(barPosition)
#define CODIFICATION NSUTF8StringEncoding

#define userDefaults [NSUserDefaults standardUserDefaults]

#define NavCller [(AppDelegate *)[[UIApplication sharedApplication] delegate] navController]

//Constants used in this project to define string name variables
#define LOGSTRING @"LogString"
#define RESPONSEDATA @"Response"
#define CORRECTRESPONSE @"CorrectResponse"

#define CARDHOLDERNAME @"CardHolderName"
#define CARDEXPDATE @"CardExpirationDate"
#define CARDNUMBER @"CardNumber"
#define CARDCVV @"CardCVV"


//find -E . -iregex '.*\.(m|h|mm)$' -print0 | xargs -0 genstrings -a -o Resources/Localization/es.lproj
//find -E . -iregex '.*\.(m|h|mm)$' -print0 | xargs -0 genstrings -a -o Resources/Localization/en.lproj
//find -E . -iregex '.*\.(m|h|mm)$' -print0 | xargs -0 genstrings -a -o Resources/Localization/fr.lproj