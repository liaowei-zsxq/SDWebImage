/*
 * This file is part of the SDWebImage package.
 * (c) Olivier Poitrey <rs@dailymotion.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */


#import "SDCallbackQueue.h"

@interface SDCallbackQueue ()

@property (nonatomic, strong, nonnull) dispatch_queue_t queue;

@end

static void * SDCallbackQueueKey = &SDCallbackQueueKey;
static void SDReleaseBlock(void *context) {
    CFRelease(context);
}

static void inline SDSafeExecute(dispatch_queue_t _Nonnull queue, dispatch_block_t _Nonnull block, BOOL async) {
    // Special handle for main queue, faster
    const char *currentLabel = dispatch_queue_get_label(DISPATCH_CURRENT_QUEUE_LABEL);
    if (currentLabel && currentLabel == dispatch_queue_get_label(dispatch_get_main_queue())) {
        block();
        return;
    }
    // Check specific to detect queue equal
    void *specific = dispatch_queue_get_specific(queue, SDCallbackQueueKey);
    void *currentSpecific = dispatch_get_specific(SDCallbackQueueKey);
    if (specific && currentSpecific && CFGetTypeID(specific) == CFUUIDGetTypeID() && CFGetTypeID(currentSpecific) == CFUUIDGetTypeID() && CFEqual(specific, currentSpecific)) {
        block();
    } else {
        if (async) {
            dispatch_async(queue, block);
        } else {
            dispatch_sync(queue, block);
        }
    }
}

@implementation SDCallbackQueue

- (instancetype)initWithDispatchQueue:(dispatch_queue_t)queue {
    self = [super init];
    if (self) {
        NSCParameterAssert(queue);
        CFUUIDRef UUID = CFUUIDCreate(kCFAllocatorDefault);
        dispatch_queue_set_specific(queue, SDCallbackQueueKey, (void *)UUID, SDReleaseBlock);
        _queue = queue;
    }
    return self;
}

+ (SDCallbackQueue *)mainQueue {
    static dispatch_once_t onceToken;
    static SDCallbackQueue *queue;
    dispatch_once(&onceToken, ^{
        queue = [[SDCallbackQueue alloc] initWithDispatchQueue:dispatch_get_main_queue()];
    });
    return queue;
}

+ (SDCallbackQueue *)currentQueue {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    SDCallbackQueue *queue = [[SDCallbackQueue alloc] initWithDispatchQueue:dispatch_get_current_queue()];
#pragma clang diagnostic pop
    return queue;
}

+ (SDCallbackQueue *)globalQueue {
    SDCallbackQueue *queue = [[SDCallbackQueue alloc] initWithDispatchQueue:dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0)];
    return queue;
}

- (void)sync:(nonnull NS_NOESCAPE dispatch_block_t)block {
    switch (self.policy) {
        case SDCallbackPolicySafeExecute:
            SDSafeExecute(self.queue, block, NO);
            break;
        case SDCallbackPolicyDispatch:
            dispatch_sync(self.queue, block);
            break;
        case SDCallbackPolicyInvoke:
            block();
            break;
    }
}

- (void)async:(nonnull NS_NOESCAPE dispatch_block_t)block {
    switch (self.policy) {
        case SDCallbackPolicySafeExecute:
            SDSafeExecute(self.queue, block, YES);
            break;
        case SDCallbackPolicyDispatch:
            dispatch_async(self.queue, block);
            break;
        case SDCallbackPolicyInvoke:
            block();
            break;
    }
}

@end
