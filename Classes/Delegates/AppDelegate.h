//
//  AppDelegate.h
//  SmartCard
//
//  Created by Raúl Horacio Valencia Tenorio on 19/03/12.
//  Copyright (c) 2009 RHVT. All rights reserved.
//
//  Class Description: Application delegate, here the most important
//  notifications about its lifecycle will be received
//
//	           (\___/)
//             (o\ /o) 
//            /|:.V.:|\
//		      \\::::://
//        -----`"" ""`-----

#import <UIKit/UIKit.h>

@interface AppDelegate : UIResponder <UIApplicationDelegate, UINavigationControllerDelegate>
{
    UIWindow *window;
    UINavigationController *navController;
}

@property (strong, nonatomic) UIWindow *window;
@property (strong, nonatomic) UINavigationController *navController;

@end
