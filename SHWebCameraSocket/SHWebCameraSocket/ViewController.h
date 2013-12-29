//
//  ViewController.h
//  SHWebCameraSocket
//
//  Created by shouian on 13/4/5.
//  Copyright (c) 2013年 Sail. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface ViewController : UIViewController <NSStreamDelegate>
{
    NSInputStream *iStream;
    NSOutputStream *oStream;
    
    int jpegHead, jpegEnd;
    UILabel *connectionLabel;
}

@property (nonatomic, strong) UIImageView *imgView;

@end
