//
//  main.m
//  SimpleLogServer
//
//  Created by KingsleyYau on 14-2-24.
//  Copyright (c) 2014年 KingsleyYau. All rights reserved.
//

#import <Cocoa/Cocoa.h>

int main(int argc, const char * argv[])
{
    // 对一个对端已经关闭的tcp socket连续写入2次会产生次错误，忽略掉
    signal(SIGPIPE, SIG_IGN);
    
    return NSApplicationMain(argc, argv);
}
