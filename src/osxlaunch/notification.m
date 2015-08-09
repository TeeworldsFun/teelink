#include <NSString.h>
#include <NSUserNotification.h>

#include "notification.h"

void CNotification::notify(const char *pTitle, const char *pMsg)
{
	NSString* title = [NSString stringWithCString:pTitle encoding:NSUTF8StringEncoding];
	NSString* msg = [NSString stringWithCString:pMsg encoding:NSUTF8StringEncoding];

	NSUserNotification *notification = [[NSUserNotification alloc] autorelease];
	notification.title = title;
	notification.informativeText = msg;
	notification.soundName = NSUserNotificationDefaultSoundName;

	[[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification:notification];
}
