//
//  NSData+Conversion.m
//  SHWebCameraSocket
//
//  Created by shouian on 13/4/9.
//  Copyright (c) 2013年 Sail. All rights reserved.
//

#import "NSData+Conversion.h"

@implementation NSData (Conversion)
- (NSString *)hexadecimalString
{
    const unsigned char *dataBuffer = (const unsigned char *)[self bytes];
    
    if (!dataBuffer)
        return [NSString string];
    
    NSUInteger          dataLength  = [self length];
    NSMutableString     *hexString  = [NSMutableString stringWithCapacity:(dataLength * 2)];
    
    for (int i = 0; i < dataLength; ++i)
        [hexString appendString:[NSString stringWithFormat:@"%02x", (unsigned long)dataBuffer[i]]];
    
    return [NSString stringWithString:hexString];
}


@end
