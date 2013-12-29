//
//  ViewController.m
//  SHWebCameraSocket
//
//  Created by shouian on 13/4/5.
//  Copyright (c) 2013年 Sail. All rights reserved.
//

#import "ViewController.h"

@interface ViewController ()
{
    int count;
}
@end

@implementation ViewController

@synthesize imgView = _imgView; 

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    _imgView = [[UIImageView alloc] initWithFrame:CGRectMake(0, 0, self.view.frame.size.width, self.view.frame.size.height)];
    [self.view addSubview:_imgView];
    count = 0;
    
    connectionLabel = [[UILabel alloc] initWithFrame:CGRectMake(0, 0, 200, 60)];
    connectionLabel.text = @"Try to connect....";
    connectionLabel.center = self.view.center;
    [connectionLabel setBackgroundColor:[UIColor clearColor]];
    connectionLabel.textColor = [UIColor colorWithRed:0.968 green:1.000 blue:0.879 alpha:1.000];
    [self.view addSubview:connectionLabel];
    
    CFReadStreamRef readStream = NULL;
    CFWriteStreamRef writeStream = NULL;
    
//    NSString *ip = @"192.168.1.2";
    NSString *ip = @"172.20.10.2";
//    NSString *ip = @"192.168.1.54";
//    NSString *ip = @"10.1.1.26";
    NSInteger port = 9899;
    
    CFStreamCreatePairWithSocketToHost(kCFAllocatorDefault, (__bridge CFStringRef)ip, port, &readStream, &writeStream);
    
    if (readStream && writeStream) {
        CFReadStreamSetProperty(readStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);
        CFWriteStreamSetProperty(writeStream, kCFStreamPropertyShouldCloseNativeSocket, kCFBooleanTrue);

        iStream = (__bridge NSInputStream *)readStream;
        [iStream setDelegate:self];
        [iStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        [iStream open];
        
    }
    
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - Delegate
- (void)stream:(NSStream *)aStream handleEvent:(NSStreamEvent)eventCode
{
    if (eventCode == NSStreamEventHasBytesAvailable) {
        
        connectionLabel.text = @"Succeed to connect";
        
        NSMutableData *imgdata = [[NSMutableData alloc] init];
        
        /*Stream*/
        uint8_t buff[4];
        [(NSInputStream *)aStream read:buff maxLength:4]; // Read Length
        uint32_t dataLength = (buff[0] << 8) |(buff[1] << 8) |(buff[2] << 8) | buff[3];
        uint8_t tcpbuff[dataLength];

        dataLength = [(NSInputStream *)aStream read:tcpbuff maxLength:dataLength]; // Read Data
        [imgdata appendBytes:(const void *)tcpbuff length:dataLength];
        /*END*/
        
        if ([self dataIsValidJPEG:imgdata]) {
            [_imgView setImage:[UIImage imageWithData:imgdata]];
        }
    }
}
#pragma mark - Helper
-(BOOL)dataIsValidJPEG:(NSData *)data
{
    if (!data || data.length < 2) return NO;
    
    NSInteger totalBytes = data.length;
    const char *bytes = (const char*)[data bytes];
    
    return (bytes[0] == (char)0xff &&
            bytes[1] == (char)0xd8 &&
            bytes[totalBytes-2] == (char)0xff &&
            bytes[totalBytes-1] == (char)0xd9);
}
@end
