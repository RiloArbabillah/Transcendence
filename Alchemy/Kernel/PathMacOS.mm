//	PathMacOS.mm
//
//	macOS-specific path utilities using Objective-C++
//	Copyright (c) 2019 Kronosaur Productions, LLC. All Rights Reserved.

#import <Foundation/Foundation.h>

extern "C" int macosTrashFile(const char *pPath)
	{
	if (pPath == NULL)
		return 1;

	@autoreleasepool
		{
		NSString *sPath = [NSString stringWithUTF8String:pPath];
		NSURL *fileURL = [NSURL fileURLWithPath:sPath];

		NSError *error = nil;
		BOOL bResult = [[NSFileManager defaultManager] trashItemAtURL:fileURL
												   resultingItemURL:nil
															  error:&error];
		if (!bResult)
			return 1;
		}

	return 0;
	}
