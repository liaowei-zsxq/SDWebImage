/*
 * This file is part of the SDWebImage package.
 * (c) Olivier Poitrey <rs@dailymotion.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#import "SDWebImageCompat.h"
#import "SDWebImageCoder.h"


/**
 This is the protocol for SDAnimatedImage class only but not for SDWebImageAnimatedCoder. If you want to provide a custom animated image class with full advanced function, you can conform to this instead of the base protocol.
 */
@protocol SDAnimatedImage <SDAnimatedImageProvider>

@required
/**
 Initializes the image with an animated coder. You can use the coder to decode the image frame later.
 @note Normally we use `initWithData:scale:` to create custom animated image class. However, for progressive image decoding, we will use this with animated coder which conforms to `SDWebImageProgressiveCoder` as well instead.
 
 @param animatedCoder An animated coder which conform `SDWebImageAnimatedCoder` protocol
 @param scale The scale factor to assume when interpreting the image data. Applying a scale factor of 1.0 results in an image whose size matches the pixel-based dimensions of the image. Applying a different scale factor changes the size of the image as reported by the `size` property.
 @return An initialized object
 */
- (nullable instancetype)initWithAnimatedCoder:(nonnull id<SDWebImageAnimatedCoder>)animatedCoder scale:(CGFloat)scale;

@optional
/**
 Pre-load all animated image frame into memory. Then later frame image request can directly return the frame for index without decoding.
 This method may be called on background thread.
 
 @note If one image instance is shared by lots of imageViews, the CPU performance for large animated image will drop down because the request frame index will be random (not in order) and the decoder should take extra effort to keep it re-entrant. You can use this to reduce CPU usage if need. Attention this will consume more memory usage.
 */
- (void)preloadAllFrames;

/**
 Unload all animated image frame from memory if are already pre-loaded. Then later frame image request need decoding. You can use this to free up the memory usage if need.
 */
- (void)unloadAllFrames;

/**
 Returns a Boolean value indicating whether all animated image frames are already pre-loaded into memory.
 */
- (BOOL)isAllFramesLoaded;

@end

@interface SDAnimatedImage : UIImage <SDAnimatedImage>

// This class override these methods from UIImage(NSImage), and it supports NSSecureCoding.
// You should use these methods to create a new animated image. Use other methods just call super instead.
+ (nullable instancetype)imageNamed:(nonnull NSString *)name; // Cache in memory, no Asset Catalog support
#if __has_include(<UIKit/UITraitCollection.h>)
+ (nullable instancetype)imageNamed:(nonnull NSString *)name inBundle:(nullable NSBundle *)bundle compatibleWithTraitCollection:(nullable UITraitCollection *)traitCollection; // Cache in memory, no Asset Catalog support
#endif
+ (nullable instancetype)imageWithContentsOfFile:(nonnull NSString *)path;
+ (nullable instancetype)imageWithData:(nonnull NSData *)data;
+ (nullable instancetype)imageWithData:(nonnull NSData *)data scale:(CGFloat)scale;
- (nullable instancetype)initWithContentsOfFile:(nonnull NSString *)path;
- (nullable instancetype)initWithData:(nonnull NSData *)data;
- (nullable instancetype)initWithData:(nonnull NSData *)data scale:(CGFloat)scale;
- (nullable instancetype)initWithAnimatedCoder:(nonnull id<SDWebImageAnimatedCoder>)animatedCoder scale:(CGFloat)scale;

/**
 Current animated image format.
 */
@property (nonatomic, assign, readonly) SDImageFormat animatedImageFormat;

/**
 Current animated image data, you can use this instead of CGImage to create another instance.
 If the current image is not animated image, this value is nil.
 */
@property (nonatomic, copy, readonly, nullable) NSData *animatedImageData;

/**
 The scale factor of the image.
 
 @note For UIKit, this just call super instead.
 @note For AppKit, `NSImage` can contains multiple image representations with different scales. However, this class does not do that from the design. We processs the scale like UIKit and store it as a extra information for correctlly rendering in `SDAnimatedImageView`.
 */
@property (nonatomic, readonly) CGFloat scale;

// By default, animated image frames are returned by decoding just in time without keeping into memory. But you can choose to preload them into memory as well, See the decsription in `SDAnimatedImage` protocol.
// After preloaded, there is no huge difference on performance between this and UIImage's `animatedImageWithImages:duration:`. But UIImage's animation have some issues such like blanking and pausing during segue when using in `UIImageView`. It's recommend to use only if need.
- (void)preloadAllFrames;
- (void)unloadAllFrames;
@property (nonatomic, assign, readonly, getter=isAllFramesLoaded) BOOL allFramesLoaded;

@end
