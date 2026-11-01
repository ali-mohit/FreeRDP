/* FreeRDP: Licensed under the Apache License, Version 2.0. */
#include <freerdp/config.h>
#include <stdio.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#ifdef WITH_XEXT
#include <X11/extensions/shape.h>
#endif
#include <freerdp/window.h>
#include <winpr/collections.h>
#include <winpr/sysinfo.h>
#include "../xfreerdp.h"
#include "../xf_rail.h"
#include "../xf_window.h"
#include "../xf_event.h"

static xfAppWindow appWindow;
static unsigned translations;
static unsigned moves;
static unsigned xMoves;
static BOOL pixmapFailure;
static unsigned pixmapFrees;
static UINT moveResult = CHANNEL_RC_OK;
static int rootX = 700;
static int rootY = 245;
static Status querySuccess = 1;
static Window windowParent = 1;
static RAIL_WINDOW_MOVE_ORDER lastMove;
static unsigned releases;
static int releaseX;
static int releaseY;
static Bool pointerSuccess = True;
static unsigned int pointerMask;
static BOOL inputSuccess = TRUE;
static KeySym lookupKey = XK_Return;
static Status attributesSuccess = 1;
static Bool translateSuccess = True;
static unsigned attributeQueries;
static BOOL changingAttributes;
static int currentWidth = 500;
static int currentHeight = 420;
static BOOL explicitPointerPosition;
static int pointerX;
static int pointerY;
static XSizeHints sizeHints;
static unsigned activations;
static UINT activateResult = CHANNEL_RC_OK;
static unsigned inputSequence;
static unsigned activationSequence;
static unsigned channelFlushSequence;
static unsigned buttonSequence;

int freerdp_channels_process_pending_messages(freerdp* instance)
{
	WINPR_UNUSED(instance);
	channelFlushSequence = ++inputSequence;
	return 1;
}

void XSetWMNormalHints(Display* display, Window window, XSizeHints* hints)
{
	WINPR_UNUSED(display);
	WINPR_UNUSED(window);
	sizeHints = *hints;
}

Status XGetWindowAttributes(Display* display, Window window, XWindowAttributes* attributes)
{
	WINPR_UNUSED(display);
	WINPR_UNUSED(window);
	attributeQueries++;
	*attributes = (XWindowAttributes){ .root = 1, .width = currentWidth, .height = currentHeight };
	if (changingAttributes)
		attributes->width += (int)(attributeQueries % 2);
	return attributesSuccess;
}

Bool XQueryPointer(Display* display, Window window, Window* root, Window* child, int* rx, int* ry,
                   int* cx, int* cy, unsigned int* mask)
{
	WINPR_UNUSED(display);
	WINPR_UNUSED(window);
	*root = *child = None;
	*rx = explicitPointerPosition ? pointerX : rootX;
	*ry = explicitPointerPosition ? pointerY : rootY;
	*cx = *cy = 0;
	*mask = pointerMask;
	return pointerSuccess;
}

KeySym XLookupKeysym(XKeyEvent* event, int index)
{
	WINPR_UNUSED(event);
	WINPR_UNUSED(index);
	return lookupKey;
}

BOOL freerdp_client_send_button_event(rdpClientContext* context, BOOL relative, UINT16 flags,
                                      INT32 x, INT32 y)
{
	WINPR_UNUSED(context);
	WINPR_UNUSED(relative);
	buttonSequence = ++inputSequence;
	if (flags == PTR_FLAGS_BUTTON1)
	{
		releases++;
		releaseX = x;
		releaseY = y;
	}
	return inputSuccess;
}

#define CHECK(condition)                                                          \
	do                                                                            \
	{                                                                             \
		if (!(condition))                                                         \
		{                                                                         \
			(void)fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); \
			goto out;                                                             \
		}                                                                         \
	} while (0)

/* Exercise the actual configure/RAIL logic without an X server or RDP server. */
Status XQueryTree(Display* display, Window window, Window* root, Window* parent, Window** children,
                  unsigned int* count)
{
	WINPR_UNUSED(display);
	WINPR_UNUSED(window);
	*root = None;
	*parent = windowParent;
	*children = nullptr;
	*count = 0;
	return querySuccess;
}

Bool XTranslateCoordinates(Display* display, Window src, Window dst, int sx, int sy, int* x, int* y,
                           Window* child)
{
	WINPR_UNUSED(display);
	WINPR_UNUSED(src);
	WINPR_UNUSED(dst);
	WINPR_UNUSED(sx);
	WINPR_UNUSED(sy);
	translations++;
	*x = rootX;
	*y = rootY;
	*child = None;
	return translateSuccess;
}

Pixmap XCreatePixmap(Display* display, Drawable drawable, unsigned int width, unsigned int height,
                     unsigned int depth)
{
	WINPR_UNUSED(display);
	WINPR_UNUSED(drawable);
	WINPR_UNUSED(width);
	WINPR_UNUSED(height);
	WINPR_UNUSED(depth);
	return pixmapFailure ? 0 : 1;
}

int XFreePixmap(Display* display, Pixmap pixmap)
{
	pixmapFrees++;
	WINPR_UNUSED(display);
	WINPR_UNUSED(pixmap);
	return 1;
}

#ifdef WITH_XEXT
void XShapeCombineRectangles(Display* display, Window window, int kind, int x, int y,
                             XRectangle* rectangles, int count, int operation, int ordering)
{
	WINPR_UNUSED(display);
	WINPR_UNUSED(window);
	WINPR_UNUSED(kind);
	WINPR_UNUSED(x);
	WINPR_UNUSED(y);
	WINPR_UNUSED(rectangles);
	WINPR_UNUSED(count);
	WINPR_UNUSED(operation);
	WINPR_UNUSED(ordering);
}

void XShapeCombineMask(Display* display, Window window, int kind, int x, int y, Pixmap pixmap,
                       int operation)
{
	WINPR_UNUSED(display);
	WINPR_UNUSED(window);
	WINPR_UNUSED(kind);
	WINPR_UNUSED(x);
	WINPR_UNUSED(y);
	WINPR_UNUSED(pixmap);
	WINPR_UNUSED(operation);
}
#endif

void XLockDisplay(Display* display)
{
	WINPR_UNUSED(display);
}

void XUnlockDisplay(Display* display)
{
	WINPR_UNUSED(display);
}

Bool XGetEventData(Display* display, XGenericEventCookie* cookie)
{
	WINPR_UNUSED(display);
	WINPR_UNUSED(cookie);
	return False;
}

void XFreeEventData(Display* display, XGenericEventCookie* cookie)
{
	WINPR_UNUSED(display);
	WINPR_UNUSED(cookie);
}

unsigned long XNextRequest(Display* display)
{
	WINPR_UNUSED(display);
	return 100;
}

int XSync(Display* display, Bool discard)
{
	WINPR_UNUSED(display);
	WINPR_UNUSED(discard);
	return 1;
}

int XMoveWindow(Display* display, Window window, int x, int y)
{
	xMoves++;
	WINPR_UNUSED(display);
	WINPR_UNUSED(window);
	WINPR_UNUSED(x);
	WINPR_UNUSED(y);
	return 1;
}

int XMoveResizeWindow(Display* display, Window window, int x, int y, unsigned int width,
                      unsigned int height)
{
	xMoves++;
	WINPR_UNUSED(display);
	WINPR_UNUSED(window);
	WINPR_UNUSED(x);
	WINPR_UNUSED(y);
	WINPR_UNUSED(width);
	WINPR_UNUSED(height);
	return 1;
}

static UINT sendMove(RailClientContext* rail, const RAIL_WINDOW_MOVE_ORDER* order)
{
	WINPR_UNUSED(rail);
	moves++;
	lastMove = *order;
	return moveResult;
}

static UINT sendActivate(RailClientContext* rail, const RAIL_ACTIVATE_ORDER* order)
{
	WINPR_UNUSED(rail);
	WINPR_ASSERT(order->enabled);
	WINPR_ASSERT(order->windowId == appWindow.windowId);
	activations++;
	activationSequence = ++inputSequence;
	return activateResult;
}

/* Most legacy scenarios exercise an accepted WM request. Keep its asynchronous
 * configure acknowledgement explicit; rejection is tested separately below. */
static BOOL acknowledge_window_request(xfContext* xfc)
{
	if (!appWindow.windowGeometryPending)
		return TRUE;
	const xfWindowGeometry* request = &appWindow.requestedGeometry;
	const XConfigureEvent accepted = { .send_event = True,
		                               .x = request->x,
		                               .y = request->y,
		                               .width = request->width,
		                               .height = request->height };
	return xf_rail_update_configure_geometry(xfc, &appWindow, &accepted);
}

static BOOL apply_server_update(rdpContext* context, const WINDOW_ORDER_INFO* info,
                                const WINDOW_STATE_ORDER* state)
{
	return context->update->window->WindowUpdate(context, info, state) &&
	       acknowledge_window_request((xfContext*)context);
}

static UINT complete_server_move(xfContext* xfc, const RAIL_LOCALMOVESIZE_ORDER* end)
{
	const UINT status = xfc->rail->ServerLocalMoveSize(xfc->rail, end);
	if ((status == CHANNEL_RC_OK) && !acknowledge_window_request(xfc))
		return ERROR_INTERNAL_ERROR;
	return status;
}

static BOOL adjust_local_geometry(xfContext* xfc, xfAppWindow* window, BOOL* redraw)
{
	/* Direct fixture assignments describe an independent local WM operation. */
	window->serverConfigure = FALSE;
	const BOOL result = xf_rail_adjust_position(xfc, window, redraw);
	const BOOL pending = window->windowGeometryPending;
	if (!acknowledge_window_request(xfc))
		return FALSE;
	if (pending)
		(void)xf_rail_adjust_position(xfc, window, redraw);
	if (redraw)
		*redraw |= window->surfaceStale;
	return result || pending;
}

static BOOL test_configure_snapshots(xfContext* xfc, XConfigureEvent* event)
{
	/* A queued server echo must use its snapshot, not a newer XTranslate result. */
	CHECK(xf_rail_update_configure_geometry(xfc, &appWindow, event));
	CHECK(translations == 0);
	CHECK(appWindow.x == 48 && appWindow.y == 0);
	CHECK(adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(moves == 0);

	/* A parent-relative server echo must not query a transient root position
	 * between the WM's client/frame moves. Its synthetic notification follows. */
	event->send_event = False;
	event->x = 27;
	event->y = 6;
	CHECK(!xf_rail_update_configure_geometry(xfc, &appWindow, event));
	CHECK(translations == 0 && moves == 0);
	CHECK(appWindow.x == 48 && appWindow.y == 0);

	/* Active moves must not capture the transient doubled root coordinate. */
	appWindow.local_move.state = LMS_ACTIVE;
	rootX = 2597;
	rootY = 369;
	CHECK(!xf_rail_update_configure_geometry(xfc, &appWindow, event));
	CHECK(translations == 0 && appWindow.x == 48 && appWindow.y == 0);
	event->send_event = True;
	event->x = 1295;
	event->y = 186;
	CHECK(xf_rail_update_configure_geometry(xfc, &appWindow, event));
	CHECK(translations == 0 && appWindow.x == 1295 && appWindow.y == 186);
	CHECK(appWindow.windowOffsetX == 48 && appWindow.windowOffsetY == 0);
	CHECK(!adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(moves == 0);
	/* A real child resize cannot be combined with the older root position. */
	event->send_event = False;
	event->x = 27;
	event->y = 6;
	event->width = 409;
	event->height = 382;
	CHECK(!xf_rail_update_configure_geometry(xfc, &appWindow, event));
	CHECK(appWindow.x == 1295 && appWindow.y == 186);
	CHECK(appWindow.width == 395 && appWindow.height == 375 && translations == 0);
	event->send_event = True;
	event->x = 1295;
	event->y = 186;
	CHECK(xf_rail_update_configure_geometry(xfc, &appWindow, event));
	CHECK(appWindow.width == 409 && appWindow.height == 382);
	event->width = 395;
	event->height = 375;
	CHECK(xf_rail_update_configure_geometry(xfc, &appWindow, event));
	appWindow.local_move.state = LMS_NOT_ACTIVE;

	/* A non-reparenting WM's real root-space move must not be deferred. */
	windowParent = None;
	event->send_event = False;
	event->x = 700;
	event->y = 245;
	CHECK(xf_rail_update_configure_geometry(xfc, &appWindow, event));
	CHECK(translations == 0 && appWindow.x == 700 && appWindow.y == 245);
	windowParent = 1;
	translations = 0;

	/* Wait for a complete synthetic root-space resize snapshot. */
	event->send_event = False;
	event->x = 0;
	event->width = 450;
	event->height = 400;
	appWindow.resizeMarginLeft = appWindow.resizeMarginRight = 7;
	appWindow.resizeMarginBottom = 7;
	appWindow.resizeMarginXKnown = appWindow.resizeMarginYKnown = TRUE;
	CHECK(!xf_rail_update_configure_geometry(xfc, &appWindow, event));
	CHECK(moves == 0);
	event->send_event = True;
	event->x = 700;
	event->y = 245;
	CHECK(xf_rail_update_configure_geometry(xfc, &appWindow, event));
	CHECK(translations == 0 && appWindow.x == 700 && appWindow.y == 245);
	CHECK(adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(moves == 1);
	CHECK(lastMove.left == 693 && lastMove.top == 245);
	CHECK(lastMove.right == 1157 && lastMove.bottom == 652);
	CHECK(appWindow.windowMovePending);
	CHECK(adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(moves == 1);

	/* The synthetic partner of a real resize must not enqueue the same move. */
	event->send_event = True;
	event->x = 700;
	event->y = 245;
	CHECK(xf_rail_update_configure_geometry(xfc, &appWindow, event));
	CHECK(adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(moves == 1);

	/* A failed parent query must not overwrite the last known local geometry. */
	event->send_event = False;
	querySuccess = 0;
	rootX = 9999;
	CHECK(!xf_rail_update_configure_geometry(xfc, &appWindow, event));
	CHECK(appWindow.x == 700 && appWindow.y == 245);
	querySuccess = 1;

	/* Resizing back to the last server size is still a real size change when
	 * a previous local resize has not been acknowledged by Windows yet. */
	rootX = 700;
	event->send_event = True;
	event->x = 700;
	event->y = 245;
	event->width = 395;
	event->height = 375;
	CHECK(xf_rail_update_configure_geometry(xfc, &appWindow, event));
	CHECK(appWindow.width == 395 && appWindow.height == 375);
	event->width = 450;
	event->height = 400;
	CHECK(xf_rail_update_configure_geometry(xfc, &appWindow, event));

	/* Prevent the signed-16-bit wrap observed during the runaway position loop. */
	appWindow.x = 32760;
	CHECK(!adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(moves == 1);
	appWindow.x = -32000;
	CHECK(adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(moves == 2 && lastMove.left == -32007);
	return TRUE;
out:
	return FALSE;
}

static BOOL test_server_reconciliation(xfContext* xfc, XConfigureEvent* event)
{

	/* Execute the registered server callback: successive surface sizes must all
	 * apply, including the initial title strip and its immediate expansion. */
	const WINDOW_ORDER_INFO info = { .windowId = appWindow.windowId,
		                             .fieldFlags = WINDOW_ORDER_FIELD_WND_SIZE };
	const UINT32 sizes[][2] = { { 395, 375 }, { 381, 368 }, { 122, 32 }, { 409, 382 } };
	for (size_t i = 0; i < ARRAYSIZE(sizes); i++)
	{
		const WINDOW_STATE_ORDER state = { .windowWidth = sizes[i][0],
			                               .windowHeight = sizes[i][1] };
		CHECK(apply_server_update(&xfc->common.context, &info, &state));
		CHECK(appWindow.windowWidth == sizes[i][0] && appWindow.windowHeight == sizes[i][1]);
		CHECK(appWindow.width == (int)sizes[i][0] && appWindow.height == (int)sizes[i][1]);
	}

	/* The local-move final report must use the confirmed synthetic snapshot.
	 * Keyboard completion needs no pointer query (and this test has no X server). */
	appWindow.local_move.state = LMS_ACTIVE;
	appWindow.local_move.direction = NET_WM_MOVERESIZE_MOVE_KEYBOARD;
	event->send_event = True;
	event->x = 1295;
	event->y = 186;
	event->width = 409;
	event->height = 382;
	CHECK(xf_rail_update_configure_geometry(xfc, &appWindow, event));
	CHECK(xf_rail_end_local_move(xfc, &appWindow));
	CHECK(appWindow.local_move.state == LMS_TERMINATING);
	CHECK(lastMove.left == 1288 && lastMove.top == 186);
	CHECK(appWindow.windowOffsetX == 48 && appWindow.windowOffsetY == 0);
	CHECK(translations == 0);

	/* A normalized server reply retires the pending request without requiring
	 * equal visible and outer bounds. Metadata alone does not retire it. */
	const unsigned before = moves;
	appWindow.local_move.state = LMS_NOT_ACTIVE;
	/* A no-op end without Window Information must not block a later WM request. */
	appWindow.localMoveGeometryTime = winpr_GetTickCount64NS() - 1001ULL * 1000000ULL;
	appWindow.x = 700;
	appWindow.y = 245;
	appWindow.width = 450;
	appWindow.height = 400;
	CHECK(adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(moves == before + 1);
	const WINDOW_ORDER_INFO unchanged = { .windowId = appWindow.windowId };
	const WINDOW_STATE_ORDER empty = { 0 };
	CHECK(apply_server_update(&xfc->common.context, &unchanged, &empty));
	CHECK(appWindow.windowMovePending);
	CHECK(adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(moves == before + 1);

	const WINDOW_ORDER_INFO adjusted = { .windowId = appWindow.windowId,
		                                 .fieldFlags = WINDOW_ORDER_FIELD_WND_OFFSET |
		                                               WINDOW_ORDER_FIELD_WND_SIZE };
	const WINDOW_STATE_ORDER normalized = {
		.windowOffsetX = 707, .windowOffsetY = 245, .windowWidth = 436, .windowHeight = 393
	};
	CHECK(apply_server_update(&xfc->common.context, &adjusted, &normalized));
	CHECK(!appWindow.windowMovePending);
	CHECK(appWindow.x == 707 && appWindow.width == 436 && appWindow.height == 393);
	CHECK(adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(moves == before + 1);

	/* A later gesture may request the same rectangle again. A -> B -> A must
	 * also remain valid; this is not a permanent rectangle blacklist. */
	appWindow.x = 700;
	appWindow.width = 450;
	appWindow.height = 400;
	CHECK(adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(moves == before + 2);
	appWindow.x = 701;
	CHECK(adjust_local_geometry(xfc, &appWindow, nullptr));
	appWindow.x = 700;
	CHECK(adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(moves == before + 4);

	/* Failed sends are never cached as successful or suppressed on retry. */
	appWindow.windowMovePending = FALSE;
	moveResult = ERROR_INTERNAL_ERROR;
	CHECK(!adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(!adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(!appWindow.windowMovePending && moves == before + 6);
	moveResult = CHANNEL_RC_OK;
	CHECK(adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(appWindow.windowMovePending && moves == before + 7);
	CHECK(adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(moves == before + 7);
	appWindow.lastWindowMoveTime = winpr_GetTickCount64NS() - 501ULL * 1000000ULL;
	CHECK(adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(moves == before + 8);
	return TRUE;
out:
	return FALSE;
}

static BOOL test_native_moves(xfContext* xfc, XConfigureEvent* event)
{
	RailClientContext* rail = xfc->rail;
	const WINDOW_ORDER_INFO unchanged = { .windowId = appWindow.windowId };
	const WINDOW_STATE_ORDER empty = WINPR_C_ARRAY_INIT;
	const WINDOW_ORDER_INFO adjusted = { .windowId = appWindow.windowId,
		                                 .fieldFlags = WINDOW_ORDER_FIELD_WND_OFFSET |
		                                               WINDOW_ORDER_FIELD_WND_SIZE };

	/* Missing margins are not zero, and are not inherited from an owner. */
	const unsigned checkpoint = moves;
	appWindow.resizeMarginXKnown = appWindow.resizeMarginYKnown = FALSE;
	appWindow.ownerWindowId = 0x40118;
	const WINDOW_ORDER_INFO marginX = { .windowId = appWindow.windowId,
		                                .fieldFlags = WINDOW_ORDER_FIELD_RESIZE_MARGIN_X };
	CHECK(apply_server_update(&xfc->common.context, &marginX, &empty));
	CHECK(appWindow.resizeMarginXKnown && !appWindow.resizeMarginYKnown);
	CHECK(!xf_rail_has_resize_margins(&appWindow));
	CHECK(apply_server_update(&xfc->common.context, &unchanged, &empty));
	CHECK(appWindow.resizeMarginXKnown && !appWindow.resizeMarginYKnown);
	appWindow.windowOffsetX = appWindow.x = 481;
	appWindow.windowOffsetY = appWindow.y = 114;
	appWindow.windowWidth = appWindow.width = 413;
	appWindow.windowHeight = appWindow.height = 229;
	appWindow.x = 228;
	appWindow.y = 239;
	CHECK(adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(moves == checkpoint);
	CHECK(appWindow.x == 481 && appWindow.y == 114 && appWindow.width == 413);
	const unsigned restored = xMoves;
	appWindow.y = 0;
	CHECK(adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(xMoves == restored && moves == checkpoint);
	appWindow.lastUnknownMarginReconcile = winpr_GetTickCount64NS() - 501ULL * 1000000ULL;
	appWindow.lastGeometryRequest = 0;
	CHECK(adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(appWindow.x == 481 && appWindow.y == 114 && xMoves == restored + 1);

	/* Server-initiated native mouse moves need no outer rectangle, even on
	 * a Server 2016 dialog. Motion, focus and grab events must not end them. */
	xfc->remote_app = TRUE;
	appWindow.local_move.state = LMS_STARTING;
	appWindow.local_move.direction = NET_WM_MOVERESIZE_MOVE;
	XEvent input = { 0 };
	const int incidental[] = { MotionNotify, EnterNotify, LeaveNotify,  FocusIn,
		                       FocusOut,     ButtonPress, ClientMessage };
	for (size_t i = 0; i < ARRAYSIZE(incidental); i++)
	{
		input.type = incidental[i];
		CHECK(xf_event_suppress_events(xfc, &appWindow, &input));
		CHECK(appWindow.local_move.state == LMS_STARTING && releases == 0);
	}
	input.type = ConfigureNotify;
	CHECK(!xf_event_suppress_events(xfc, &appWindow, &input));
	CHECK(appWindow.local_move.state == LMS_ACTIVE);
	event->send_event = True;
	event->x = 228;
	event->y = 239;
	event->width = 413;
	event->height = 229;
	CHECK(xf_rail_update_configure_geometry(xfc, &appWindow, event));
	input.type = MotionNotify;
	CHECK(xf_event_suppress_events(xfc, &appWindow, &input));
	CHECK(appWindow.local_move.state == LMS_ACTIVE && releases == 0);
	rootX = 300;
	rootY = 250;
	input.type = ButtonRelease;
	input.xbutton.button = Button1;
	CHECK(xf_event_suppress_events(xfc, &appWindow, &input));
	CHECK(releases == 1 && releaseX == 300 && releaseY == 250);
	CHECK(moves == checkpoint && appWindow.local_move.state == LMS_TERMINATING);
	CHECK(appWindow.windowOffsetX == 481 && appWindow.windowOffsetY == 114);
	CHECK(xf_rail_end_local_move(xfc, &appWindow));
	CHECK(releases == 1);
	CHECK(!adjust_local_geometry(xfc, &appWindow, nullptr));

	const RAIL_LOCALMOVESIZE_ORDER end = { .windowId = (UINT32)appWindow.windowId,
		                                   .moveSizeType = RAIL_WMSZ_MOVE };
	const WINDOW_STATE_ORDER movedDialog = {
		.windowOffsetX = 228, .windowOffsetY = 239, .windowWidth = 413, .windowHeight = 229
	};
	CHECK(apply_server_update(&xfc->common.context, &adjusted, &movedDialog));
	CHECK(appWindow.serverGeometryPending);
	CHECK(complete_server_move(xfc, &end) == CHANNEL_RC_OK);
	CHECK(appWindow.local_move.state == LMS_NOT_ACTIVE && !appWindow.serverGeometryPending);
	CHECK(adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(moves == checkpoint && appWindow.width == 413 && appWindow.height == 229);

	/* Regression: twenty completed native moves do not progressively shrink. */
	for (int i = 0; i < 20; i++)
	{
		appWindow.local_move.state = LMS_ACTIVE;
		event->x = 250 + i;
		event->y = 200 + i;
		CHECK(xf_rail_update_configure_geometry(xfc, &appWindow, event));
		CHECK(xf_rail_end_local_move(xfc, &appWindow));
		const WINDOW_STATE_ORDER reply = { .windowOffsetX = event->x,
			                               .windowOffsetY = event->y,
			                               .windowWidth = 413,
			                               .windowHeight = 229 };
		CHECK(apply_server_update(&xfc->common.context, &adjusted, &reply));
		CHECK(complete_server_move(xfc, &end) == CHANNEL_RC_OK);
		CHECK(adjust_local_geometry(xfc, &appWindow, nullptr));
		CHECK(appWindow.width == 413 && appWindow.height == 229 && moves == checkpoint);
	}

	/* XI2 uses the generic pointer handlers, not core X event suppression. */
	const unsigned releasesBeforeXI = releases;
	const unsigned translationsBeforeXI = translations;
	appWindow.local_move.state = LMS_ACTIVE;
	CHECK(xf_generic_MotionNotify(xfc, 10, 10, appWindow.handle, TRUE));
	CHECK(xf_generic_ButtonEvent(xfc, 10, 10, Button1, appWindow.handle, TRUE, TRUE));
	CHECK(releases == releasesBeforeXI && translations == translationsBeforeXI);
	CHECK(appWindow.local_move.state == LMS_ACTIVE);
	xfc->xi_event = TRUE;
	input.type = ButtonRelease;
	input.xbutton.button = Button1;
	CHECK(xf_event_suppress_events(xfc, &appWindow, &input));
	CHECK(releases == releasesBeforeXI && appWindow.local_move.state == LMS_ACTIVE);
	CHECK(xf_generic_ButtonEvent(xfc, 10, 10, Button1, appWindow.handle, TRUE, FALSE));
	xfc->xi_event = FALSE;
	CHECK(releases == releasesBeforeXI + 1 && moves == checkpoint);
	CHECK(appWindow.local_move.state == LMS_TERMINATING);
	RAIL_LOCALMOVESIZE_ORDER endWithoutType = end;
	endWithoutType.moveSizeType = 0;
	CHECK(rail->ServerLocalMoveSize(rail, &endWithoutType) == CHANNEL_RC_OK);
	CHECK(translations == translationsBeforeXI && appWindow.local_move.state == LMS_NOT_ACTIVE);

	/* Failed pointer reads or input sends do not falsely finish a gesture. */
	appWindow.local_move.state = LMS_STARTING;
	pointerSuccess = False;
	CHECK(!xf_rail_end_local_move(xfc, &appWindow));
	CHECK(appWindow.local_move.state == LMS_STARTING);
	pointerSuccess = True;
	inputSuccess = FALSE;
	CHECK(!xf_rail_end_local_move(xfc, &appWindow));
	CHECK(appWindow.local_move.state == LMS_STARTING);
	inputSuccess = TRUE;
	CHECK(xf_rail_end_local_move(xfc, &appWindow));
	CHECK(complete_server_move(xfc, &end) == CHANNEL_RC_OK);

	/* Unknown-margin keyboard moves and resizes stay server-driven instead of
	 * requesting an unsafe local interaction or emitting a zero-margin PDU. */
	const RAIL_LOCALMOVESIZE_ORDER start = { .windowId = (UINT32)appWindow.windowId,
		                                     .isMoveSizeStart = 1,
		                                     .moveSizeType = RAIL_WMSZ_RIGHT };
	CHECK(rail->ServerLocalMoveSize(rail, &start) == CHANNEL_RC_OK);
	CHECK(appWindow.local_move.state == LMS_NOT_ACTIVE && moves == checkpoint);
	input.type = MotionNotify;
	CHECK(!xf_event_suppress_events(xfc, &appWindow, &input));
	RAIL_LOCALMOVESIZE_ORDER keyboardStart = start;
	keyboardStart.moveSizeType = RAIL_WMSZ_KEYMOVE;
	CHECK(rail->ServerLocalMoveSize(rail, &keyboardStart) == CHANNEL_RC_OK);
	CHECK(appWindow.local_move.state == LMS_NOT_ACTIVE && moves == checkpoint);

	return TRUE;
out:
	return FALSE;
}

static BOOL test_resize_and_keyboard_completion(xfContext* xfc, XConfigureEvent* event)
{
	const unsigned checkpoint = moves;
	const WINDOW_ORDER_INFO marginY = { .windowId = appWindow.windowId,
		                                .fieldFlags = WINDOW_ORDER_FIELD_RESIZE_MARGIN_Y };
	const WINDOW_ORDER_INFO unchanged = { .windowId = appWindow.windowId };
	const WINDOW_STATE_ORDER empty = WINPR_C_ARRAY_INIT;
	const RAIL_LOCALMOVESIZE_ORDER end = { .windowId = (UINT32)appWindow.windowId,
		                                   .moveSizeType = RAIL_WMSZ_MOVE };
	XEvent input = WINPR_C_ARRAY_INIT;

	/* Explicit zero margins ARE usable, and survive later omitted fields. */
	CHECK(apply_server_update(&xfc->common.context, &marginY, &empty));
	CHECK(xf_rail_has_resize_margins(&appWindow));
	CHECK(apply_server_update(&xfc->common.context, &unchanged, &empty));
	CHECK(xf_rail_has_resize_margins(&appWindow));
	appWindow.windowMovePending = FALSE;
	appWindow.local_move.state = LMS_ACTIVE;
	appWindow.local_move.direction = NET_WM_MOVERESIZE_SIZE_RIGHT;
	appWindow.width = 530;
	appWindow.height = 269;
	CHECK(xf_rail_end_local_move(xfc, &appWindow));
	CHECK(moves == checkpoint + 1);
	CHECK(lastMove.right - lastMove.left == 530 && lastMove.bottom - lastMove.top == 269);
	CHECK(appWindow.windowWidth == 413 && appWindow.windowHeight == 229);
	CHECK(complete_server_move(xfc, &end) == CHANNEL_RC_OK);

	/* Keyboard completion is explicit, not triggered by an arbitrary key. */
	appWindow.local_move.state = LMS_ACTIVE;
	appWindow.local_move.direction = NET_WM_MOVERESIZE_MOVE_KEYBOARD;
	appWindow.windowMovePending = FALSE;
	input.type = KeyPress;
	lookupKey = XK_Left;
	CHECK(!xf_event_suppress_events(xfc, &appWindow, &input));
	CHECK(appWindow.local_move.state == LMS_ACTIVE);
	lookupKey = XK_Return;
	CHECK(!xf_event_suppress_events(xfc, &appWindow, &input));
	CHECK(appWindow.local_move.state == LMS_TERMINATING && moves == checkpoint + 2);
	appWindow.local_move.state = LMS_ACTIVE;
	lookupKey = XK_Escape;
	CHECK(!xf_event_suppress_events(xfc, &appWindow, &input));
	CHECK(appWindow.local_move.state == LMS_TERMINATING && moves == checkpoint + 2);
	CHECK(complete_server_move(xfc, &end) == CHANNEL_RC_OK);
	appWindow.width = 450;
	appWindow.height = 400;

	/* Invalid event sizes must not replace an otherwise valid snapshot. */
	event->width = 0;
	CHECK(!xf_rail_update_configure_geometry(xfc, &appWindow, event));
	CHECK(appWindow.width == 450 && appWindow.height == 400);

	return TRUE;
out:
	return FALSE;
}

static BOOL test_real_configure_snapshots(xfContext* xfc, XConfigureEvent* event)
{
	const RAIL_LOCALMOVESIZE_ORDER end = { .windowId = (UINT32)appWindow.windowId,
		                                   .moveSizeType = RAIL_WMSZ_MOVE };

	/* Real-only reparented resizes must converge without a synthetic event.
	 * Query dimensions as well as position: this queued event is older than X11. */
	const unsigned movesBeforeRealOnly = moves;
	appWindow.local_move.state = LMS_NOT_ACTIVE;
	event->send_event = False;
	event->x = event->y = 0;
	event->width = 460;
	event->height = 410;
	rootX = 620;
	rootY = 170;
	currentWidth = 500;
	currentHeight = 420;
	CHECK(!xf_rail_update_configure_geometry(xfc, &appWindow, event));
	CHECK(appWindow.configureGeometryPending);
	CHECK(appWindow.width == 450 && appWindow.height == 400);
	CHECK(xf_rail_sync_configure_geometry(xfc, &appWindow));
	CHECK(!appWindow.configureGeometryPending);
	CHECK(appWindow.x == 620 && appWindow.y == 170);
	CHECK(appWindow.width == 500 && appWindow.height == 420);
	CHECK(moves == movesBeforeRealOnly);

	/* A synthetic snapshot cancels the queued query and keeps its own tuple. */
	CHECK(!xf_rail_update_configure_geometry(xfc, &appWindow, event));
	const unsigned queriesBeforeSynthetic = attributeQueries;
	event->send_event = True;
	event->x = 650;
	event->y = 190;
	event->width = 510;
	CHECK(xf_rail_update_configure_geometry(xfc, &appWindow, event));
	CHECK(xf_rail_sync_configure_geometry(xfc, &appWindow));
	CHECK(attributeQueries == queriesBeforeSynthetic && appWindow.x == 650);
	CHECK(appWindow.width == 510 && !appWindow.configureGeometryPending);

	/* Failures and unstable reads preserve the previous complete snapshot;
	 * a later flush can retry, without spinning or accepting mixed geometry. */
	event->send_event = False;
	CHECK(!xf_rail_update_configure_geometry(xfc, &appWindow, event));
	attributesSuccess = 0;
	CHECK(!xf_rail_sync_configure_geometry(xfc, &appWindow));
	attributesSuccess = 1;
	translateSuccess = False;
	CHECK(!xf_rail_sync_configure_geometry(xfc, &appWindow));
	translateSuccess = True;
	changingAttributes = TRUE;
	const unsigned queriesBeforeUnstable = attributeQueries;
	CHECK(!xf_rail_sync_configure_geometry(xfc, &appWindow));
	CHECK(attributeQueries == queriesBeforeUnstable + 4);
	CHECK(appWindow.x == 650 && appWindow.width == 510 && appWindow.configureGeometryPending);
	changingAttributes = FALSE;
	xf_rail_flush_configure_geometry(xfc);
	CHECK(!appWindow.configureGeometryPending && appWindow.x == 620 && appWindow.width == 500);
	CHECK(moves == movesBeforeRealOnly);

	/* Native resize completion flushes pending geometry before WINDOWMOVE,
	 * even if pointer release precedes the batch flush. */
	appWindow.local_move.state = LMS_ACTIVE;
	appWindow.local_move.direction = NET_WM_MOVERESIZE_SIZE_RIGHT;
	appWindow.windowMovePending = FALSE;
	currentWidth = 550;
	CHECK(!xf_rail_update_configure_geometry(xfc, &appWindow, event));
	const unsigned beforeDeferredRelease = releases;
	explicitPointerPosition = TRUE;
	pointerX = 700;
	pointerY = 250;
	changingAttributes = TRUE;
	CHECK(xf_rail_end_local_move(xfc, &appWindow));
	CHECK(appWindow.local_move.completionPending && appWindow.local_move.state == LMS_ACTIVE);
	CHECK(releases == beforeDeferredRelease && moves == movesBeforeRealOnly);
	/* A retry uses the release position, even if the pointer has moved. */
	pointerX = 950;
	pointerY = 400;
	changingAttributes = FALSE;
	xf_rail_flush_configure_geometry(xfc);
	explicitPointerPosition = FALSE;
	CHECK(!appWindow.local_move.completionPending && releases == beforeDeferredRelease + 1);
	CHECK(releaseX == 700 && releaseY == 250);
	CHECK(moves == movesBeforeRealOnly + 1 && appWindow.width == 550);
	CHECK(!appWindow.configureGeometryPending && appWindow.local_move.state == LMS_TERMINATING);
	xf_rail_flush_configure_geometry(xfc);
	CHECK(releases == beforeDeferredRelease + 1 && moves == movesBeforeRealOnly + 1);
	CHECK(lastMove.right - lastMove.left == 550);
	CHECK(complete_server_move(xfc, &end) == CHANNEL_RC_OK);
	const WINDOW_ORDER_INFO completed = { .windowId = appWindow.windowId,
		                                  .fieldFlags = WINDOW_ORDER_FIELD_WND_SIZE };
	const WINDOW_STATE_ORDER finalSize = { .windowWidth = 550, .windowHeight = appWindow.height };
	CHECK(apply_server_update(&xfc->common.context, &completed, &finalSize));
	/* At a normal event-batch boundary, a focused idle window reports the
	 * single checked current snapshot; another flush does not resend it. */
	xfc->focused = TRUE;
	rootX = 625;
	currentWidth = 560;
	CHECK(!xf_rail_update_configure_geometry(xfc, &appWindow, event));
	xf_rail_flush_configure_geometry(xfc);
	CHECK(appWindow.x == 625 && appWindow.width == 560 && !appWindow.configureGeometryPending);
	CHECK(moves == movesBeforeRealOnly + 2);
	xf_rail_flush_configure_geometry(xfc);
	CHECK(moves == movesBeforeRealOnly + 2);
	xfc->focused = FALSE;
	/* Unmapping cancels a queued query rather than probing a vanished window. */
	appWindow.configureGeometryPending = TRUE;
	appWindow.is_mapped = FALSE;
	const unsigned queriesBeforeUnmap = attributeQueries;
	xf_rail_flush_configure_geometry(xfc);
	CHECK(!appWindow.configureGeometryPending && attributeQueries == queriesBeforeUnmap);
	return TRUE;
out:
	return FALSE;
}

static BOOL test_move_end_ordering(xfContext* xfc)
{
	const WINDOW_ORDER_INFO info = { .windowId = appWindow.windowId,
		                             .fieldFlags = WINDOW_ORDER_FIELD_WND_OFFSET |
		                                           WINDOW_ORDER_FIELD_WND_SIZE };
	const int directions[] = { NET_WM_MOVERESIZE_MOVE, NET_WM_MOVERESIZE_SIZE_RIGHT,
		                       NET_WM_MOVERESIZE_MOVE_KEYBOARD, NET_WM_MOVERESIZE_MOVE };
	for (size_t direction = 0; direction < ARRAYSIZE(directions); direction++)
	{
		for (unsigned beforeEnd = 0; beforeEnd < 2; beforeEnd++)
		{
			const unsigned sentBefore = moves;
			appWindow.is_mapped = TRUE;
			appWindow.rail_state = WINDOW_SHOW;
			appWindow.dwStyle = 0;
			appWindow.localMoveGeometryTime = 0;
			appWindow.windowMovePending = FALSE;
			/* Mouse moves must work even when a dialog omits its margins. */
			appWindow.resizeMarginXKnown = appWindow.resizeMarginYKnown = direction != 0;
			appWindow.resizeMarginLeft = appWindow.resizeMarginRight = 7;
			appWindow.resizeMarginTop = 0;
			appWindow.resizeMarginBottom = 7;
			appWindow.local_move.state = LMS_ACTIVE;
			appWindow.local_move.direction = directions[direction];
			appWindow.local_move.positionGeneration = appWindow.serverPositionGeneration;
			appWindow.local_move.sizeGeneration = appWindow.serverSizeGeneration;
			const WINDOW_STATE_ORDER initial = {
				.windowOffsetX = 145, .windowOffsetY = 129, .windowWidth = 413, .windowHeight = 229
			};
			CHECK(apply_server_update(&xfc->common.context, &info, &initial));
			const XConfigureEvent local = {
				.send_event = True, .x = -40, .y = 118, .width = 430, .height = 240
			};
			CHECK(xf_rail_update_configure_geometry(xfc, &appWindow, &local));
			CHECK(xf_rail_end_local_move(xfc, &appWindow));
			const unsigned completedMoves = moves;
			CHECK(completedMoves == sentBefore + (directions[direction] != NET_WM_MOVERESIZE_MOVE));
			/* The final visible bounds may differ from the local rectangle. */
			const WINDOW_STATE_ORDER final = {
				.windowOffsetX = -35, .windowOffsetY = 118, .windowWidth = 423, .windowHeight = 233
			};
			if (beforeEnd)
				CHECK(apply_server_update(&xfc->common.context, &info, &final));
			const unsigned appliedBeforeEnd = xMoves;
			const RAIL_LOCALMOVESIZE_ORDER end = { .windowId = (UINT32)appWindow.windowId,
				                                   .moveSizeType = RAIL_WMSZ_MOVE,
				                                   .posX = -42,
				                                   .posY = 118 };
			CHECK(complete_server_move(xfc, &end) == CHANNEL_RC_OK);
			/* End-before-update must not queue the old cached X11 position. */
			CHECK(xMoves == appliedBeforeEnd);
			CHECK(appWindow.x == (beforeEnd ? -35 : -40));
			if (!beforeEnd)
			{
				CHECK(!adjust_local_geometry(xfc, &appWindow, nullptr));
				CHECK(xMoves == appliedBeforeEnd && moves == completedMoves);
				CHECK(apply_server_update(&xfc->common.context, &info, &final));
			}
			CHECK(appWindow.x == -35 && appWindow.y == 118);
			CHECK(appWindow.width == 423 && appWindow.height == 233);
			CHECK(adjust_local_geometry(xfc, &appWindow, nullptr));
			CHECK(moves == completedMoves && !appWindow.serverGeometryPending);
		}
	}
	/* Focusing a local restore must not send its old visible bounds while
	 * the server still reports the minimized rectangle. */
	const unsigned beforeRestore = moves;
	appWindow.dwStyle = WS_MINIMIZE;
	appWindow.windowOffsetX = -32000;
	appWindow.windowOffsetY = -32000;
	CHECK(!adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(moves == beforeRestore);
	appWindow.dwStyle = 0;
	/* The show/style order may arrive before the restored geometry order. */
	appWindow.stateGeometryPending = TRUE;
	CHECK(!adjust_local_geometry(xfc, &appWindow, nullptr));
	const WINDOW_STATE_ORDER restored = {
		.windowOffsetX = -35, .windowOffsetY = 118, .windowWidth = 423, .windowHeight = 233
	};
	CHECK(apply_server_update(&xfc->common.context, &info, &restored));
	CHECK(!appWindow.stateGeometryPending);
	CHECK(adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(moves == beforeRestore);
	return TRUE;
out:
	return FALSE;
}

static BOOL test_cancel_pending_completion(xfContext* xfc)
{
	const unsigned before = releases;
	const unsigned beforeMoves = moves;
	const int events[] = { UnmapNotify, DestroyNotify };
	for (size_t i = 0; i < ARRAYSIZE(events); i++)
	{
		appWindow.local_move.state = LMS_ACTIVE;
		appWindow.local_move.completionPending = TRUE;
		appWindow.configureGeometryPending = TRUE;
		appWindow.visibilityShapeDeferred = TRUE;
		/* Child structure notifications must not abort the parent's gesture. */
		XEvent event = { .xunmap = { .type = events[i], .window = appWindow.handle + 1 } };
		CHECK(xf_event_suppress_events(xfc, &appWindow, &event));
		CHECK(appWindow.local_move.state == LMS_ACTIVE && appWindow.local_move.completionPending);
		event.xunmap.window = appWindow.handle;
		CHECK(!xf_event_suppress_events(xfc, &appWindow, &event));
		CHECK(appWindow.local_move.state == LMS_NOT_ACTIVE);
		CHECK(!appWindow.local_move.completionPending && !appWindow.configureGeometryPending);
		CHECK(!appWindow.visibilityShapeDeferred);
		xf_rail_flush_configure_geometry(xfc);
		CHECK(releases == before && moves == beforeMoves);
	}
	return TRUE;
out:
	return FALSE;
}

static BOOL test_fixed_size_hints(xfContext* xfc)
{
	appWindow.windowStyleKnown = TRUE;
	appWindow.dwStyle = WS_CAPTION | WS_SYSMENU;
	appWindow.windowWidth = 413;
	appWindow.windowHeight = 229;
	xf_SetWindowMinMaxInfo(xfc, &appWindow, 0, 0, 0, 0, 136, 39, 1670, 932);
	CHECK(!xf_rail_window_can_resize(&appWindow));
	CHECK(sizeHints.min_width == 413 && sizeHints.max_width == 413);
	CHECK(sizeHints.min_height == 229 && sizeHints.max_height == 229);
	appWindow.windowWidth = 450;
	appWindow.windowHeight = 250;
	xf_UpdateWindowSizeHints(xfc, &appWindow);
	CHECK(sizeHints.min_width == 450 && sizeHints.max_width == 450);
	appWindow.dwStyle |= WS_SIZEBOX;
	xf_UpdateWindowSizeHints(xfc, &appWindow);
	CHECK(xf_rail_window_can_resize(&appWindow));
	CHECK(sizeHints.min_width == 136 && sizeHints.max_width == 1670);
	CHECK(sizeHints.min_height == 39 && sizeHints.max_height == 932);
	appWindow.dwStyle &= ~WS_SIZEBOX;
	appWindow.local_move.state = LMS_NOT_ACTIVE;
	appWindow.localMoveGeometryTime = 0;
	appWindow.windowMovePending = FALSE;
	appWindow.windowOffsetX = 20;
	appWindow.windowOffsetY = 30;
	appWindow.x = 80;
	appWindow.y = 90;
	appWindow.width = 600;
	appWindow.height = 500;
	appWindow.resizeMarginXKnown = appWindow.resizeMarginYKnown = TRUE;
	appWindow.resizeMarginLeft = appWindow.resizeMarginRight = 0;
	appWindow.resizeMarginTop = appWindow.resizeMarginBottom = 0;
	CHECK(adjust_local_geometry(xfc, &appWindow, nullptr));
	CHECK(appWindow.x == 80 && appWindow.y == 90);
	CHECK(appWindow.width == 450 && appWindow.height == 250);
	CHECK(lastMove.left == 80 && lastMove.top == 90);
	CHECK(lastMove.right - lastMove.left == 450 && lastMove.bottom - lastMove.top == 250);
	appWindow.windowStyleKnown = FALSE;
	CHECK(xf_rail_window_can_resize(&appWindow));
	return TRUE;
out:
	return FALSE;
}

static BOOL test_pointer_activation(xfContext* xfc)
{
	xfc->remote_app = TRUE;
	xfc->rail->ClientActivate = sendActivate;
	xfc->button_map[0].button = Button1;
	xfc->button_map[0].flags = PTR_FLAGS_BUTTON1;
	appWindow.is_mapped = TRUE;
	appWindow.is_transient = FALSE;
	appWindow.dwStyle = WS_CAPTION | WS_SYSMENU;
	appWindow.dwExStyle = 0;
	const WINDOW_ORDER_INFO info = { .fieldFlags = WINDOW_ORDER_TYPE_DESKTOP |
		                                           WINDOW_ORDER_FIELD_DESKTOP_ACTIVE_WND };
	MONITORED_DESKTOP_ORDER desktop = { .activeWindowId = (UINT32)appWindow.windowId };
	CHECK(xfc->common.context.update->window->MonitoredDesktop(&xfc->common.context, &info,
	                                                           &desktop));
	CHECK(xf_generic_ButtonEvent(xfc, 10, 10, Button1, appWindow.handle, TRUE, TRUE));
	CHECK(activations == 0);
	desktop.activeWindowId++;
	CHECK(xfc->common.context.update->window->MonitoredDesktop(&xfc->common.context, &info,
	                                                           &desktop));
	CHECK(xf_generic_ButtonEvent(xfc, 10, 10, Button1, appWindow.handle, TRUE, TRUE));
	CHECK(activations == 1 && activationSequence < channelFlushSequence &&
	      channelFlushSequence < buttonSequence);
	CHECK(xf_generic_ButtonEvent(xfc, 10, 10, Button1, appWindow.handle, TRUE, FALSE));
	CHECK(activations == 1);
	/* No optimistic acknowledgement: a rejected/unanswered switch can retry. */
	CHECK(xf_generic_ButtonEvent(xfc, 10, 10, Button1, appWindow.handle, TRUE, TRUE));
	CHECK(activations == 2);
	desktop.activeWindowId = (UINT32)appWindow.windowId;
	CHECK(xfc->common.context.update->window->MonitoredDesktop(&xfc->common.context, &info,
	                                                           &desktop));
	CHECK(xf_generic_ButtonEvent(xfc, 10, 10, Button1, appWindow.handle, TRUE, TRUE));
	CHECK(activations == 2);
	xfc->railActiveWindowId = 0;
	appWindow.is_transient = TRUE;
	CHECK(xf_generic_ButtonEvent(xfc, 10, 10, Button1, appWindow.handle, TRUE, TRUE));
	CHECK(activations == 2);
	appWindow.is_transient = FALSE;
	appWindow.dwExStyle = WS_EX_NOACTIVATE;
	CHECK(xf_generic_ButtonEvent(xfc, 10, 10, Button1, appWindow.handle, TRUE, TRUE));
	CHECK(activations == 2);
	appWindow.dwExStyle = 0;
	appWindow.ownerWindowId = 0x100;
	appWindow.dwStyle |= WS_POPUP;
	CHECK(xf_generic_ButtonEvent(xfc, 10, 10, Button1, appWindow.handle, TRUE, TRUE));
	CHECK(activations == 3); /* A real owned dialog can be activated. */
	activateResult = ERROR_INTERNAL_ERROR;
	const unsigned beforeFailure = buttonSequence;
	CHECK(!xf_generic_ButtonEvent(xfc, 10, 10, Button1, appWindow.handle, TRUE, TRUE));
	CHECK(buttonSequence == beforeFailure);
	return TRUE;
out:
	return FALSE;
}

static xfContext* repaintContext;
static unsigned repaintRequests;
static BOOL repaintWhileLocked;

static void* inspect_repaint_lock(RdpgfxClientContext* gfx, UINT16 surfaceId)
{
	WINPR_UNUSED(gfx);
	WINPR_UNUSED(surfaceId);
	repaintRequests++;
#if defined(WITH_VERBOSE_WINPR_ASSERT)
	repaintWhileLocked |= repaintContext->isRailWindowsLocked;
#endif
	return nullptr;
}

static BOOL test_deferred_repaint(xfContext* xfc)
{
	BOOL redraw = FALSE;
	appWindow.local_move.state = LMS_NOT_ACTIVE;
	appWindow.localMoveGeometryTime = 0;
	appWindow.stateGeometryPending = FALSE;
	appWindow.dwStyle = WS_CAPTION | WS_SYSMENU;
	appWindow.windowStyleKnown = TRUE;
	appWindow.is_mapped = TRUE;
	appWindow.surfaceId = 1;
	appWindow.x = appWindow.windowOffsetX = 20;
	appWindow.y = appWindow.windowOffsetY = 30;
	appWindow.windowWidth = 450;
	appWindow.windowHeight = 250;
	appWindow.width = 600;
	appWindow.height = 500;
	appWindow.pixmapWidth = 600;
	appWindow.pixmapHeight = 500;
	CHECK(adjust_local_geometry(xfc, &appWindow, &redraw));
	CHECK(redraw && appWindow.surfaceStale);
	CHECK(appWindow.width == 450 && appWindow.height == 250);

	appWindow.local_move.state = LMS_ACTIVE;
	appWindow.serverGeometryPending = TRUE;
	appWindow.windowWidth = 470;
	CHECK(!xf_EndLocalMoveSize(xfc, &appWindow));
	CHECK(appWindow.windowGeometryPending && appWindow.width == 450);
	CHECK(acknowledge_window_request(xfc));
	CHECK(appWindow.local_move.state == LMS_NOT_ACTIVE && appWindow.width == 470);

	/* Expose must release the window before looking up the GFX surface. */
	RdpgfxClientContext gfx = { .GetSurfaceData = inspect_repaint_lock };
	InitializeCriticalSection(&gfx.mux);
	rdpGdi* gdi = xfc->common.context.gdi;
	RdpgfxClientContext* previous = gdi->gfx;
	gdi->gfx = &gfx;
	repaintContext = xfc;
	xfc->exposeRequested = TRUE;
	xfc->exposedWindow = appWindow.handle;
	xfc->exposedArea.w = appWindow.width;
	xfc->exposedArea.h = appWindow.height;
	freerdp instance = { .context = &xfc->common.context };
	const BOOL repainted = xf_event_update_screen(&instance);
	CHECK(repainted && repaintRequests == 1 && !repaintWhileLocked);
	/* A fixed-size correction allocates/repaints only after the asynchronous
	 * WM response. Both batch and server-completion paths release the lock. */
	appWindow.surfaceStale = FALSE;
	appWindow.configureGeometryPending = TRUE;
	appWindow.decorations = TRUE;
	currentWidth = 600;
	currentHeight = 500;
	rootX = appWindow.windowOffsetX;
	rootY = appWindow.windowOffsetY;
	xf_rail_flush_configure_geometry(xfc);
	CHECK(appWindow.windowGeometryPending && repaintRequests == 2);
	CHECK(acknowledge_window_request(xfc));
	xf_rail_flush_configure_geometry(xfc);
	const unsigned afterBatch = repaintRequests;
	appWindow.surfaceStale = FALSE;
	appWindow.local_move.state = LMS_ACTIVE;
	appWindow.serverGeometryPending = TRUE;
	appWindow.windowWidth = 490;
	appWindow.serverSizeGeneration++;
	const RAIL_LOCALMOVESIZE_ORDER end = { .windowId = (UINT32)appWindow.windowId,
		                                   .posX = 20,
		                                   .posY = 30 };
	const UINT ended = xfc->rail->ServerLocalMoveSize(xfc->rail, &end);
	CHECK(appWindow.windowGeometryPending);
	CHECK(acknowledge_window_request(xfc));
	xf_rail_flush_configure_geometry(xfc);
	gdi->gfx = previous;
	DeleteCriticalSection(&gfx.mux);
	CHECK(afterBatch == 3 && ended == CHANNEL_RC_OK);
	CHECK(repaintRequests == 4 && !repaintWhileLocked);
	return TRUE;
out:
	return FALSE;
}

static BOOL test_connection_move_owner(xfContext* xfc)
{
	BOOL inserted = FALSE;
	xfAppWindow other = { .xfc = xfc,
		                  .windowId = 0x300d1,
		                  .handle = 0x100,
		                  .is_mapped = TRUE,
		                  .dwStyle = WS_CAPTION | WS_SYSMENU };
	freerdp instance = { .context = &xfc->common.context };
	const RAIL_LOCALMOVESIZE_ORDER end = { .windowId = (UINT32)appWindow.windowId };
	appWindow.surfaceId = UINT32_MAX;
	appWindow.is_mapped = TRUE;
	appWindow.windowStyleKnown = TRUE;
	appWindow.dwStyle = WS_CAPTION | WS_SYSMENU | WS_SIZEBOX;
	appWindow.resizeMarginXKnown = appWindow.resizeMarginYKnown = TRUE;
	appWindow.resizeMarginLeft = appWindow.resizeMarginRight = 7;
	appWindow.resizeMarginTop = 0;
	appWindow.resizeMarginBottom = 7;
	appWindow.local_move =
	    (xfLocalMove){ .state = LMS_ACTIVE, .direction = NET_WM_MOVERESIZE_MOVE };
	appWindow.configureGeometryPending = FALSE;
	appWindow.serverGeometryPending = FALSE;
	appWindow.x = appWindow.windowOffsetX = rootX = 80;
	appWindow.y = appWindow.windowOffsetY = rootY = 90;
	appWindow.width = currentWidth = 400;
	appWindow.windowWidth = 400;
	appWindow.height = currentHeight = 300;
	appWindow.windowHeight = 300;
	xfc->railLocalMoveWindowId = appWindow.windowId;
	pointerMask = Button1Mask;
	CHECK(HashTable_Insert(xfc->railWindows, &other.windowId, &other));
	inserted = TRUE;
	const unsigned inputsBefore = inputSequence;
	const unsigned before = releases;
	const unsigned movesBefore = moves;
	CHECK(xf_generic_MotionNotify(xfc, 10, 10, other.handle, TRUE));
	CHECK(xf_generic_ButtonEvent(xfc, 10, 10, Button1, other.handle, TRUE, TRUE));
	const XEvent focus = { .xfocus = {
		                       .type = FocusIn, .window = other.handle, .mode = NotifyNormal } };
	CHECK(xf_event_process(&instance, &focus));
	CHECK(inputSequence == inputsBefore && releases == before);
	XEvent ungrab = { .xcrossing = { .type = EnterNotify,
		                             .window = appWindow.handle,
		                             .mode = NotifyUngrab,
		                             .x_root = 700,
		                             .y_root = 250 } };
	CHECK(xf_event_process(&instance, &ungrab));
	CHECK(appWindow.local_move.state == LMS_ACTIVE && releases == before);
	pointerMask = 0;
	CHECK(xf_event_process(&instance, &ungrab));
	CHECK(appWindow.local_move.state == LMS_TERMINATING && releases == before + 1);
	CHECK(releaseX == 700 && releaseY == 250 && moves == movesBefore);
	const XEvent up = { .xbutton = { .type = ButtonRelease,
		                             .window = other.handle,
		                             .button = Button1,
		                             .x_root = 999,
		                             .y_root = 999 } };
	CHECK(xf_event_process(&instance, &up));
	CHECK(releases == before + 1);
	CHECK(complete_server_move(xfc, &end) == CHANNEL_RC_OK);
	CHECK(!xfc->railLocalMoveWindowId);

	/* A release on another window retains its event coordinates while a
	 * changing geometry snapshot is retried, including a tiled mouse move. */
	appWindow.local_move = (xfLocalMove){ .state = LMS_ACTIVE,
		                                  .direction = NET_WM_MOVERESIZE_MOVE,
		                                  .window_width = 400,
		                                  .window_height = 300 };
	appWindow.windowMovePending = FALSE;
	xfc->railLocalMoveWindowId = appWindow.windowId;
	currentWidth = 500;
	changingAttributes = TRUE;
	XEvent deferred = up;
	deferred.xbutton.x_root = 740;
	deferred.xbutton.y_root = 310;
	CHECK(xf_event_process(&instance, &deferred));
	CHECK(releases == before + 1 && appWindow.local_move.completionPending);
	changingAttributes = FALSE;
	rootX = 900;
	rootY = 450;
	xf_rail_flush_configure_geometry(xfc);
	CHECK(releases == before + 2 && releaseX == 740 && releaseY == 310);
	CHECK(moves == movesBefore + 1 && lastMove.right - lastMove.left == 514);
	CHECK(complete_server_move(xfc, &end) == CHANNEL_RC_OK);

	/* The WM never resized locally, but Windows has newer bounds. Do not
	 * overwrite those bounds with the stale local size at completion. */
	appWindow.x = rootX = 80;
	appWindow.y = rootY = 90;
	appWindow.width = currentWidth = 400;
	appWindow.height = currentHeight = 300;
	appWindow.windowOffsetX = 80;
	appWindow.windowOffsetY = 90;
	appWindow.windowWidth = 700;
	appWindow.windowHeight = 500;
	appWindow.serverGeometryPending = TRUE;
	appWindow.local_move = (xfLocalMove){ .state = LMS_STARTING,
		                                  .direction = NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT,
		                                  .window_x = 80,
		                                  .window_y = 90,
		                                  .window_width = 400,
		                                  .window_height = 300 };
	xfc->railLocalMoveWindowId = appWindow.windowId;
	CHECK(xf_event_check_local_move(xfc));
	CHECK(appWindow.local_move.reconcileOnEnd && moves == movesBefore + 1);
	CHECK(complete_server_move(xfc, &end) == CHANNEL_RC_OK);
	CHECK(appWindow.width == 700 && appWindow.height == 500);
	CHECK(!xfc->railLocalMoveWindowId);

	/* XI2's common button path resolves the connection owner too. */
	appWindow.local_move =
	    (xfLocalMove){ .state = LMS_ACTIVE, .direction = NET_WM_MOVERESIZE_MOVE };
	currentWidth = 700;
	currentHeight = 500;
	xfc->railLocalMoveWindowId = appWindow.windowId;
	CHECK(xf_generic_ButtonEvent(xfc, 10, 10, Button1, other.handle, TRUE, FALSE));
	CHECK(releases == before + 4 && appWindow.local_move.state == LMS_TERMINATING);
	CHECK(complete_server_move(xfc, &end) == CHANNEL_RC_OK);

	/* A queued motion predates a WM-consumed release. Infer completion from
	 * the current pointer, not the older coordinates in that motion event. */
	appWindow.local_move =
	    (xfLocalMove){ .state = LMS_ACTIVE, .direction = NET_WM_MOVERESIZE_MOVE };
	xfc->railLocalMoveWindowId = appWindow.windowId;
	const XEvent motion = {
		.xmotion = { .type = MotionNotify, .window = other.handle, .x_root = 1, .y_root = 2 }
	};
	CHECK(xf_event_process(&instance, &motion));
	CHECK(releases == before + 5 && releaseX == rootX && releaseY == rootY);
	CHECK(complete_server_move(xfc, &end) == CHANNEL_RC_OK);

	/* A changed size alone never authorizes guessing margins or resizing a
	 * fixed window. Keep the existing mouse-move-only completion in both cases. */
	for (size_t fixed = 0; fixed < 2; fixed++)
	{
		appWindow.resizeMarginXKnown = fixed != 0;
		appWindow.dwStyle = WS_CAPTION | WS_SYSMENU | (fixed ? 0 : WS_SIZEBOX);
		appWindow.local_move = (xfLocalMove){ .state = LMS_ACTIVE,
			                                  .direction = NET_WM_MOVERESIZE_MOVE,
			                                  .window_width = 400,
			                                  .window_height = 300 };
		xfc->railLocalMoveWindowId = appWindow.windowId;
		CHECK(xf_event_process(&instance, &up));
		CHECK(moves == movesBefore + 1);
		CHECK(complete_server_move(xfc, &end) == CHANNEL_RC_OK);
	}
	HashTable_ValueObject(xfc->railWindows)->fnObjectFree = nullptr;
	HashTable_Remove(xfc->railWindows, &other.windowId);
	return TRUE;
out:
	changingAttributes = FALSE;
	pointerMask = 0;
	if (inserted)
	{
		HashTable_ValueObject(xfc->railWindows)->fnObjectFree = nullptr;
		HashTable_Remove(xfc->railWindows, &other.windowId);
	}
	return FALSE;
}

static BOOL test_resize_style_transitions(xfContext* xfc)
{
	const UINT32 normal = WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_SIZEBOX;
	const UINT32 stripped = WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	appWindow.resizeCapabilityKnown = FALSE;
	appWindow.rail_state = WINDOW_SHOW;
	appWindow.showState = WINDOW_SHOW;
	xf_rail_update_window_style(&appWindow, normal, 0);
	xf_SetWindowMinMaxInfo(xfc, &appWindow, 0, 0, 0, 0, 136, 39, 1940, 1220);
	xf_rail_update_window_style(&appWindow, stripped, 0);
	xf_UpdateWindowSizeHints(xfc, &appWindow);
	CHECK(xf_rail_window_can_resize(&appWindow));
	CHECK(sizeHints.min_width == 136 && sizeHints.max_width == 1940);
	CHECK(sizeHints.min_height == 39 && sizeHints.max_height == 1220);
	/* Popup is a window category, not evidence that its frame survived. */
	xf_rail_update_window_style(&appWindow, normal | WS_POPUP, 0);
	xf_rail_update_window_style(&appWindow, stripped | WS_POPUP, 0);
	CHECK(xf_rail_window_can_resize(&appWindow));
	/* A complete style update can legitimately remove resize permission. */
	xf_rail_update_window_style(&appWindow, normal & ~WS_SIZEBOX, 0);
	xf_rail_update_window_style(&appWindow, stripped, 0);
	xf_UpdateWindowSizeHints(xfc, &appWindow);
	CHECK(!xf_rail_window_can_resize(&appWindow));
	CHECK(sizeHints.min_width == (int)appWindow.windowWidth);
	CHECK(sizeHints.max_width == (int)appWindow.windowWidth);
	xf_rail_update_window_style(&appWindow, normal, 0);
	xf_UpdateWindowSizeHints(xfc, &appWindow);
	CHECK(sizeHints.min_width == 136 && sizeHints.max_width == 1940);

	/* Windows can restore during a mouse move. Those known server bounds
	 * need no second resize request; subsequent genuinely local tiling does. */
	const unsigned before = moves;
	const unsigned beforeRelease = releases;
	appWindow.local_move = (xfLocalMove){ .state = LMS_ACTIVE,
		                                  .direction = NET_WM_MOVERESIZE_MOVE,
		                                  .window_width = 1854,
		                                  .window_height = 1168 };
	appWindow.width = 863;
	appWindow.windowWidth = 863;
	appWindow.height = 884;
	appWindow.windowHeight = 884;
	appWindow.serverGeometryPending = TRUE;
	appWindow.configureGeometryPending = FALSE;
	CHECK(xf_rail_end_local_move(xfc, &appWindow));
	CHECK(moves == before && releases == beforeRelease + 1);
	appWindow.local_move = (xfLocalMove){ .state = LMS_ACTIVE,
		                                  .direction = NET_WM_MOVERESIZE_MOVE,
		                                  .window_width = 863,
		                                  .window_height = 884 };
	appWindow.width = 927;
	appWindow.height = 1168;
	appWindow.windowMovePending = FALSE;
	CHECK(xf_rail_end_local_move(xfc, &appWindow));
	CHECK(moves == before + 1 && releases == beforeRelease + 2);
	return TRUE;
out:
	return FALSE;
}

static BOOL test_requested_observed_geometry(xfContext* xfc)
{
	appWindow.local_move.state = LMS_NOT_ACTIVE;
	appWindow.localMoveGeometryTime = 0;
	appWindow.stateGeometryPending = FALSE;
	appWindow.windowGeometryPending = FALSE;
	appWindow.serverConfigure = FALSE;
	appWindow.dwStyle = WS_VISIBLE | WS_CAPTION | WS_SIZEBOX;
	xf_rail_update_window_style(&appWindow, appWindow.dwStyle, 0);
	appWindow.resizeMarginXKnown = appWindow.resizeMarginYKnown = TRUE;
	appWindow.resizeMarginLeft = appWindow.resizeMarginRight = 0;
	appWindow.resizeMarginTop = appWindow.resizeMarginBottom = 0;
	appWindow.windowMovePending = FALSE;
	appWindow.x = rootX = 120;
	appWindow.y = rootY = 90;
	appWindow.width = currentWidth = 400;
	appWindow.height = currentHeight = 300;
	const unsigned beforeMoves = moves;
	const unsigned beforeRequests = xMoves;
	CHECK(!xf_MoveWindow(xfc, &appWindow, 220, 140, 800, 600));
	CHECK(appWindow.windowGeometryPending && appWindow.serverConfigure);
	CHECK(appWindow.x == 120 && appWindow.width == 400);
	const XConfigureEvent stale = {
		.send_event = True, .serial = 99, .x = 100, .y = 80, .width = 350, .height = 250
	};
	CHECK(!xf_rail_update_configure_geometry(xfc, &appWindow, &stale));
	CHECK(appWindow.windowGeometryPending && appWindow.x == 120 && appWindow.width == 400);
	CHECK(!xf_rail_sync_configure_geometry(xfc, &appWindow));
	CHECK(appWindow.windowGeometryPending && appWindow.configureGeometryPending);
	CHECK(!xf_MoveWindow(xfc, &appWindow, 220, 140, 800, 600));
	CHECK(xMoves == beforeRequests + 1);
	/* The WM clamps the target: report its accepted tuple once, otherwise
	 * Windows would route input using the rejected server position. */
	rootX = 210;
	rootY = 130;
	currentWidth = 780;
	currentHeight = 580;
	CHECK(xf_rail_sync_configure_geometry(xfc, &appWindow));
	CHECK(appWindow.windowGeometryPending && appWindow.configureGeometryPending);
	CHECK(!xf_rail_adjust_position(xfc, &appWindow, nullptr));
	CHECK(moves == beforeMoves);
	/* A different intermediate snapshot must not undo the target. Report
	 * only the still-clamped current tuple after the WM response deadline. */
	appWindow.lastGeometryRequest = 1;
	CHECK(xf_rail_sync_configure_geometry(xfc, &appWindow));
	CHECK(!appWindow.windowGeometryPending && !appWindow.serverConfigure);
	CHECK(appWindow.x == 210 && appWindow.width == 780);
	CHECK(xf_rail_adjust_position(xfc, &appWindow, nullptr));
	CHECK(xf_rail_adjust_position(xfc, &appWindow, nullptr));
	CHECK(moves == beforeMoves + 1);
	/* A subsequent independent local configure must still be reported. */
	const XConfigureEvent local = {
		.send_event = True, .x = 230, .y = 150, .width = 790, .height = 590
	};
	CHECK(xf_rail_update_configure_geometry(xfc, &appWindow, &local));
	CHECK(!appWindow.serverConfigure && xf_rail_adjust_position(xfc, &appWindow, nullptr));
	CHECK(moves == beforeMoves + 2);
	/* A denied/no-op request expires without inventing observed success. */
	CHECK(!xf_MoveWindow(xfc, &appWindow, 800, 700, 1200, 900));
	appWindow.lastGeometryRequest = 1;
	rootX = 230;
	rootY = 150;
	currentWidth = 790;
	currentHeight = 590;
	CHECK(xf_rail_sync_configure_geometry(xfc, &appWindow));
	CHECK(!appWindow.windowGeometryPending && appWindow.x == 230 && appWindow.width == 790);
	CHECK(xf_rail_adjust_position(xfc, &appWindow, nullptr));
	CHECK(moves == beforeMoves + 2);
	/* Keep the previous backing store if replacement allocation fails,
	 * retain the observed bounds and retry allocation at a later flush. */
	const Pixmap oldPixmap = appWindow.pixmap;
	const UINT32 oldWidth = appWindow.pixmapWidth;
	const unsigned oldFrees = pixmapFrees;
	pixmapFailure = TRUE;
	XConfigureEvent enlarged = local;
	enlarged.width = 900;
	CHECK(!xf_rail_update_configure_geometry(xfc, &appWindow, &enlarged));
	CHECK(appWindow.width == 900 && appWindow.configureGeometryPending);
	CHECK(appWindow.pixmap == oldPixmap && appWindow.pixmapWidth == oldWidth);
	CHECK(pixmapFrees == oldFrees);
	pixmapFailure = FALSE;
	currentWidth = 900;
	CHECK(xf_rail_sync_configure_geometry(xfc, &appWindow));
	CHECK(appWindow.pixmapWidth == 900 && !appWindow.configureGeometryPending);
	/* Temporary and final startup sizes are never filtered by arrival time.
	 * Delay WM acknowledgement until the final server target is queued. */
	const WINDOW_ORDER_INFO info = { .windowId = appWindow.windowId,
		                             .fieldFlags = WINDOW_ORDER_FIELD_WND_OFFSET |
		                                           WINDOW_ORDER_FIELD_WND_SIZE };
	WINDOW_STATE_ORDER state = {
		.windowOffsetX = 80, .windowOffsetY = 70, .windowWidth = 200, .windowHeight = 100
	};
	CHECK(xfc->common.context.update->window->WindowUpdate(&xfc->common.context, &info, &state));
	state.windowWidth = 1000;
	state.windowHeight = 700;
	CHECK(xfc->common.context.update->window->WindowUpdate(&xfc->common.context, &info, &state));
	CHECK(appWindow.windowWidth == 1000 && appWindow.requestedGeometry.width == 1000);
	CHECK(appWindow.width == 900);
	CHECK(acknowledge_window_request(xfc));
	CHECK(appWindow.width == 1000 && appWindow.height == 700);
	return TRUE;
out:
	pixmapFailure = FALSE;
	return FALSE;
}

static BOOL test_move_end_position(xfContext* xfc)
{
	const RAIL_LOCALMOVESIZE_ORDER end = { .windowId = (UINT32)appWindow.windowId,
		                                   .moveSizeType = RAIL_WMSZ_MOVE,
		                                   .posX = -40,
		                                   .posY = 180 };
	const unsigned beforeMoves = moves;
	appWindow.local_move.state = LMS_NOT_ACTIVE;
	appWindow.serverMoveActive = TRUE;
	appWindow.local_move.positionGeneration = appWindow.serverPositionGeneration;
	appWindow.local_move.sizeGeneration = appWindow.serverSizeGeneration;
	const int width = appWindow.width;
	const int height = appWindow.height;
	CHECK(xfc->rail->ServerLocalMoveSize(xfc->rail, &end) == CHANNEL_RC_OK);
	CHECK(appWindow.windowGeometryPending && appWindow.x != -40);
	CHECK(appWindow.requestedGeometry.x == -40 && appWindow.requestedGeometry.y == 180);
	CHECK(appWindow.requestedGeometry.width == width &&
	      appWindow.requestedGeometry.height == height);
	CHECK(acknowledge_window_request(xfc));
	CHECK(appWindow.x == -40 && appWindow.y == 180 && appWindow.width == width);
	const unsigned requests = xMoves;
	CHECK(xfc->rail->ServerLocalMoveSize(xfc->rail, &end) == CHANNEL_RC_OK);
	CHECK(xMoves == requests && moves == beforeMoves);
	/* A newer Window Information position wins over a delayed end PDU. */
	appWindow.serverMoveActive = TRUE;
	appWindow.local_move.positionGeneration = appWindow.serverPositionGeneration;
	const WINDOW_ORDER_INFO info = { .windowId = appWindow.windowId,
		                             .fieldFlags = WINDOW_ORDER_FIELD_WND_OFFSET };
	const WINDOW_STATE_ORDER state = { .windowOffsetX = 60, .windowOffsetY = 80 };
	CHECK(apply_server_update(&xfc->common.context, &info, &state));
	CHECK(xfc->rail->ServerLocalMoveSize(xfc->rail, &end) == CHANNEL_RC_OK);
	CHECK(appWindow.x == 60 && appWindow.y == 80 && moves == beforeMoves);
	return TRUE;
out:
	return FALSE;
}

int main(int argc, char* argv[])
{
	WINPR_UNUSED(argc);
	WINPR_UNUSED(argv);
	int result = 1;
	BOOL railInitialized = FALSE;
	xfContext xfc = WINPR_C_ARRAY_INIT;
	Screen screen = WINPR_C_ARRAY_INIT;
	rdpWindowUpdate windows = WINPR_C_ARRAY_INIT;
	rdpUpdate update = { .window = &windows };
	RailClientContext rail = { .ClientWindowMove = sendMove };
	rdpGdi gdi = { .gfx = (RdpgfxClientContext*)1 };
	xfc.log = WLog_Get("com.freerdp.client.x11");
	xfc.screen = &screen;
	xfc.depth = 24;
	xfc.UseXThreads = TRUE;
	xfc.common.context.update = &update;
	xfc.common.context.gdi = &gdi;
	xfc.common.context.settings = freerdp_settings_new(0);
	CHECK(xfc.common.context.settings);
	CHECK(xf_rail_init(&xfc, &rail));
	railInitialized = TRUE;
	appWindow.windowId = 0x300d0;
	appWindow.xfc = &xfc;
	appWindow.surfaceId = UINT32_MAX;
	CHECK(HashTable_Insert(xfc.railWindows, &appWindow.windowId, &appWindow));
	appWindow.is_mapped = TRUE;
	appWindow.windowOffsetX = 48;
	appWindow.windowWidth = 395;
	appWindow.windowHeight = 375;
	XConfigureEvent event = { .send_event = True, .x = 48, .width = 395, .height = 375 };

	/* Keep the fixture and event across scenarios to exercise consecutive
	 * gestures and pending server replies in their original order. */
	CHECK(test_configure_snapshots(&xfc, &event));
	CHECK(test_server_reconciliation(&xfc, &event));
	CHECK(test_native_moves(&xfc, &event));
	CHECK(test_resize_and_keyboard_completion(&xfc, &event));
	CHECK(test_real_configure_snapshots(&xfc, &event));
	CHECK(test_move_end_ordering(&xfc));
	CHECK(test_cancel_pending_completion(&xfc));
	CHECK(test_fixed_size_hints(&xfc));
	CHECK(test_pointer_activation(&xfc));
	CHECK(test_deferred_repaint(&xfc));
	CHECK(test_connection_move_owner(&xfc));
	CHECK(test_resize_style_transitions(&xfc));
	CHECK(test_requested_observed_geometry(&xfc));
	CHECK(test_move_end_position(&xfc));
	result = 0;
out:
	if (railInitialized)
	{
		/* The window is owned by the test, not by the RAIL window table. */
		HashTable_ValueObject(xfc.railWindows)->fnObjectFree = nullptr;
		xf_rail_uninit(&xfc, &rail);
	}
	appWindow.xfc = nullptr;
	rail.custom = nullptr;
	freerdp_settings_free(xfc.common.context.settings);
	return result;
}
