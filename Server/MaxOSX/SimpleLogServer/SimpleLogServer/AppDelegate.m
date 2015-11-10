//
//  AppDelegate.m
//  SimpleLogServer
//
//  Created by KingsleyYau on 14-2-24.
//  Copyright (c) 2014年 KingsleyYau. All rights reserved.
//

#import "AppDelegate.h"

@interface AppDelegate() <NSTableViewDataSource>{
    DrUdpSocket *m_pDrUDPSocket;
    DrSnifferServer *m_pDrSnifferServer;
    DrTcpSocket m_CurrentSocket;
    DrSnifferCallback m_DrSnifferCallback;
}

@end

@implementation AppDelegate
#pragma mark - 界面事件
- (IBAction)buttonListenAction:(id)sender {
    if([self startDrSnifferServer]) {
        [self updateUI:@"服务端监听成功!"];
    }
}
- (IBAction)buttonStopAction:(id)sender {
    [self stopDrSnifferServer];
    [self updateUI:@"服务端停止监听成功!"];
    [self reloadData:YES];
}
- (IBAction)buttonSendCmdAction:(id)sender {
    if(m_CurrentSocket.IsConnected()) {
        SCCMD sccmd;
		sccmd.scmdt = ExcuteCommand;
		sccmd.param = [self.textFieldCommand.stringValue UTF8String];
		m_pDrSnifferServer->SendCommand(m_CurrentSocket, sccmd);
        [self.textFieldCommand setStringValue:@""];
    }
}
- (IBAction)buttonGetVersionAction:(id)sender {
    if(m_CurrentSocket.IsConnected()) {
        SCCMD sccmd;
		sccmd.scmdt = SinfferClientTypeVersion;
		m_pDrSnifferServer->SendCommand(m_CurrentSocket, sccmd);
    }
}
- (IBAction)buttonClearAction:(id)sender {
    [self.textView setEditable:YES];
    [self.textView setEditable:NO];
}
- (void)initView {
    [self.textView setString:@"欢迎使用!\n"];
    NSColor *fgColor = [NSColor greenColor];
    NSColor *bgColor = [NSColor blueColor];
    [self.textView setSelectedTextAttributes:@{NSBackgroundColorAttributeName:bgColor, NSForegroundColorAttributeName:fgColor}];
}
#pragma mark - 主程序事件
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Insert code here to initialize your application
    [self initView];
    [self initDrSniffer];
    [self setupTableView];
    [self reloadData:YES];
}
- (void)applicationWillTerminate:(NSNotification *)notification {
    if(m_pDrUDPSocket) {
        delete m_pDrUDPSocket;
        m_pDrUDPSocket = NULL;
    }
    if(m_pDrSnifferServer) {
        delete m_pDrSnifferServer;
        m_pDrSnifferServer = NULL;
    }
}
- (void)onNewClientOnline:(DrTcpSocket)aSocket {
    NSMutableString *text = [NSMutableString stringWithFormat:@"客户端:%s:%d上线\n", aSocket.GetIP().c_str(), aSocket.GetPort()];
    [self performSelectorOnMainThread:@selector(updateUI:) withObject:text waitUntilDone:NO];
    [self reloadData:YES];
}
- (void)onRecvCommand:(DrTcpSocket)aSocket sscmd:(SSCMD)sscmd {
    NSMutableString *text = [NSMutableString stringWithFormat:@"收到来自:%s:%d的请求:\n", aSocket.GetIP().c_str(), aSocket.GetPort()];
    
    switch (sscmd.scmdt) {
        case SinfferServerTypeOffLine:{
            // 与客户端连接已经断开
            [text appendString:@"与客户端连接已经断开"];
            [self reloadData:YES];
        }break;
        case ExcuteCommandResult:{
            [text appendString:[NSString stringWithFormat:@"客户端命令返回:\n%s\n", sscmd.param.c_str()]];
        }break;
        case SnifferServerTypeClientInfo:{
            [text appendString:[NSString stringWithFormat:@"客户端信息返回:\n%s\n", sscmd.param.c_str()]];
            [self reloadData:YES];
        }break;
        default: {
        }break;
    }
    
    [self performSelectorOnMainThread:@selector(updateUI:) withObject:text waitUntilDone:NO];
}
#pragma mark - DrSnifferServer事件
- (void)initDrSniffer {
    if(m_pDrSnifferServer == NULL) {
        m_pDrSnifferServer = new DrSnifferServer();
    }
    
    m_DrSnifferCallback.onNewClientOnline = onNewClientOnline;
    m_DrSnifferCallback.onRecvCommand = onRecvCommand;
    m_pDrSnifferServer->SetCallback(m_DrSnifferCallback);
}
- (BOOL)startDrSnifferServer {
    BOOL bFlag = false;
    bFlag = m_pDrSnifferServer->Start(256, ServerPort);
    return bFlag;
}
- (void)stopDrSnifferServer {
    m_pDrSnifferServer->Stop();
}
void onNewClientOnline(DrTcpSocket aSocket) {
    AppDelegate *delegate = (AppDelegate *)[NSApplication sharedApplication].delegate;
    [delegate onNewClientOnline:aSocket];
}
void onRecvCommand(DrTcpSocket aSocket, SSCMD sscmd) {
    AppDelegate *delegate = (AppDelegate *)[NSApplication sharedApplication].delegate;
    [delegate onRecvCommand:aSocket sscmd:sscmd];
}
#pragma mark - 列表事件
- (void)reloadData:(BOOL)isReloadView {
    [self.tableView reloadData];
}
- (void)setupTableView {
    NSTableColumn *column = [self.tableView tableColumnWithIdentifier:@"IP"];
    [[column headerCell] setStringValue:@"IP地址"];
    
    column = [self.tableView tableColumnWithIdentifier:@"LastOnline"];
    [[column headerCell] setStringValue:@"最后上线时间"];

}
- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    NSInteger count = 0;
    if(m_pDrSnifferServer) {
        count = m_pDrSnifferServer->GetClientOnlineList()->size();
    }
    return count;
}
- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
    NSString *result = @"";
    
    NSInteger i = 0;
    NSInteger index = row;
    
    ClientInfo info;
    ClientList *pClientlList = m_pDrSnifferServer->GetClientOnlineList();
    for(ClientList::iterator itr = pClientlList->begin(); itr != pClientlList->end(); itr++, i++) {
        if(i == index) {
            info = (*itr);
            break;
        }
    }
    
    if([[tableColumn identifier] isEqualToString:@"IP"]) {
        // IP地址列
        DrTcpSocket aSocket = info.aSocket;
        result = [NSString stringWithUTF8String:aSocket.GetIP().c_str()];

    }
    else if([[tableColumn identifier] isEqualToString:@"Model"]){
        result = [NSString stringWithFormat:@"[厂商:%s 型号:%s 手机号:%s]", info.brand.c_str(), info.model.c_str(), info.phoneNumber.c_str()];
    }

    return result;
}
- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row {
    // IP地址列
    NSInteger index = row;
    NSInteger i = 0;

    ClientList *pClientlList = m_pDrSnifferServer->GetClientOnlineList();
    for(ClientList::iterator itr = pClientlList->begin(); itr != pClientlList->end(); itr++, i++) {
        if(i == index) {
            m_CurrentSocket = (*itr).aSocket;
        }
    }
    return YES;
}


/*
 * UdpServer
 */
- (void)startUdpServer {
    m_pDrUDPSocket->Bind(61440);
    
    NSThread *recvThread = [[NSThread alloc] initWithTarget:self selector:@selector(recvUdpThreadProc:) object:nil];
    [recvThread start];
}
- (void)recvUdpThreadProc:(id)argument {
    NSLog(@"recvThreadProc start ");
    char recvBuff[1024];
    bzero(recvBuff, sizeof(recvBuff));
    
    int len = 0;
    while (1) {
        len = m_pDrUDPSocket->RecvData(recvBuff, sizeof(recvBuff));
        if(len > 0) {
            NSString *text = [NSString stringWithUTF8String:recvBuff];
            [self performSelectorOnMainThread:@selector(updateUI:) withObject:text waitUntilDone:NO];
            NSLog(@"recvBuff:%s", recvBuff);
        }
    }
}
- (void)updateUI:(NSString *)logString {
    [self.textView setEditable:YES];
    [[[self.textView textStorage] mutableString] appendString:logString];
    [[[self.textView textStorage] mutableString] appendString:@"\n"];

    NSRange range;
    range = NSMakeRange ([[self.textView string] length], 0);
    [self.textView scrollRangeToVisible:range];
    
    [self.textView setEditable:NO];
}

- (IBAction)buttonAction:(id)sender {
    m_pDrUDPSocket->SendData((char *)"127.0.0.1", 61440, (char *)"hello world", sizeof("hello world"));
}
@end
