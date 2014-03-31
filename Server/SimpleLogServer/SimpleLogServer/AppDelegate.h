//
//  AppDelegate.h
//  SimpleLogServer
//
//  Created by KingsleyYau on 14-2-24.
//  Copyright (c) 2014年 KingsleyYau. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#include "DrUDPSocket.h"
#include "DrRunnable.h"

#include "drsniffer/DrSnifferServer.h"
#include "drsniffer/DrSnifferCallback.h"

@interface AppDelegate : NSObject <NSApplicationDelegate>

@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet NSTextView *textView;
@property (assign) IBOutlet NSTextField *textFieldCommand;
@property (assign) IBOutlet NSTableView *tableView;
@property (assign) IBOutlet NSTextField *textField;

@property (strong) NSString *commanddString;

- (IBAction)buttonListenAction:(id)sender;
- (IBAction)buttonStopAction:(id)sender;
- (IBAction)buttonSendCmdAction:(id)sender;
- (IBAction)buttonGetVersionAction:(id)sender;
- (IBAction)buttonClearAction:(id)sender;

- (IBAction)buttonAction:(id)sender;
- (void)updateUI:(NSString *)logString;

- (void)onRecvCommand:(DrTcpSocket)aSocket sscmd:(SSCMD)sscmd;
@end
