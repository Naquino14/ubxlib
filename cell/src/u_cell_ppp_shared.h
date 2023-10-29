/*
 * Copyright 2019-2023 u-blox
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _U_CELL_PPP_SHARED_H_
#define _U_CELL_PPP_SHARED_H_

/* Only header files representing a direct and unavoidable
 * dependency between the API of this module and the API
 * of another module should be included here; otherwise
 * please keep #includes to your .c files. */

#include "u_port_ppp.h"

/** @file
 * @brief This header file defines functions that expose the PPP
 * transport for cellular.  They are not intended for direct customer
 * use, they are shared internally with the port layer which then
 * integrates with the bottom-end of the IP stack of a platform.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------
 * COMPILE-TIME MACROS
 * -------------------------------------------------------------- */

#ifndef U_CELL_PPP_DIAL_TIMEOUT_SECONDS
/** The time in seconds to wait for a PPP dial-up to succeed.
 */
# define U_CELL_PPP_DIAL_TIMEOUT_SECONDS 240
#endif

#ifndef U_CELL_PPP_HANG_UP_TIMEOUT_SECONDS
/** How long to wait for PPP to disconnect, that is to return
 * "NO CARRIER" after the hang-up sequence "~+++" has been sent.
 */
# define U_CELL_PPP_HANG_UP_TIMEOUT_SECONDS 30
#endif

/* ----------------------------------------------------------------
 * TYPES
 * -------------------------------------------------------------- */

/* ----------------------------------------------------------------
 * FUNCTIONS
 * -------------------------------------------------------------- */

/** Open the PPP interface of a cellular module; only works with
 * modules where CMUX is supported (so, for example, does not work
 * on LARA-R6).  The cellular network connection should already have
 * been brought up using uCellNetConnect() or uCellNetActivate(); all
 * this does is open the PPP data interface.  If the PPP interface is
 * already open this function will do nothing and return success;
 * please call uCellPppClose() first if you would like to change
 * the buffering arrangements, the callback or its parameter.
 *
 * Note: this will invoke multiplexer mode in the cellular device
 * and hence will only work on interfaces that support multiplexer
 * mode (for example the USB interface of a cellular device does not
 * support multiplexer mode).  Also, since multiplexer mode is a
 * frame-oriented protocol it will be broken if there a character
 * is lost on the interface and hence, on a UART interface, it is
 * HIGHLY RECOMMENDED that the UART flow control lines are
 * connected.
 *
 * Note: this function will allocate memory that is not released,
 * for thread-safety reasons, until the cellular device is closed.
 * If you need the heap memory back before then, see uCellPppFree().
 *
 * Implementation note: follows the function signature of
 * #uPortPppConnectCallback_t.
 *
 * @param cellHandle                    the handle of the cellular instance.
 * @param[in] pReceiveCallback          the data reception callback; may be
 *                                      NULL if only data transmission is
 *                                      required.
 * @param[in,out] pReceiveCallbackParam a parameter that will be passed to
 *                                      pReceiveCallback as its last parameter;
 *                                      may be NULL, ignored if pReceiveCallback
 *                                      is NULL.
 * @param[in] pReceiveData              a pointer to a buffer for received
 *                                      data; may be NULL, in which case, if
 *                                      pReceiveCallback is non-NULL, this code
 *                                      will provide a receive buffer.
 * @param receiveDataSize               the amount of space at pReceiveData in
 *                                      bytes or, if pReceiveData is NULL,
 *                                      the receive buffer size that should
 *                                      be allocated by this function;
 *                                      #U_PORT_PPP_RECEIVE_BUFFER_BYTES is
 *                                      a sensible value.
 * @param[in] pKeepGoingCallback        a callback function that governs how
 *                                      long to wait for the PPP connection to
 *                                      open.  This function is called once a
 *                                      second while waiting for the "CONNECT"
 *                                      response; the PPP open attempt will
 *                                      only continue while it returns true.
 *                                      This allows the caller to terminate
 *                                      the connection attempt at their
 *                                      convenience. May be NULL, in which case
 *                                      the connection attempt will eventually
 *                                      time out on failure.
 * @return                              zero on success, else negative error
 *                                      code.
 */
int32_t uCellPppOpen(uDeviceHandle_t cellHandle,
                     uPortPppReceiveCallback_t *pReceiveCallback,
                     void *pReceiveCallbackParam,
                     char *pReceiveData, size_t receiveDataSize,
                     bool (*pKeepGoingCallback) (uDeviceHandle_t cellHandle));

/** Close the PPP interface of a cellular module.  This does not
 * deactivate the cellular connection, the caller must do that
 * afterwards with a call to uCellNetDisconnect() or
 * uCellNetDeactivate().  When this function has returned the
 * pReceiveCallback function passed to uCellPppOpen() will no longer be
 * called and any pReceiveData buffer passed to uCellPppOpen()
 * will no longer be written-to.  If no PPP connection is open this
 * function will do nothing and return success.
 *
 * Note: in the case of SARA-R5, and potentially other modules,
 * it is vital that the PPP session is closed cleanly by the
 * application before this function is called; not doing so may
 * lead to the PPP entity being unresponsive in subsequent calls
 * to uCellPppOpen().  Should that occur, the sure way out is
 * to reboot the module, e.g. with uCellPwrReboot(), before
 * calling uCellPppOpen() again.
 *
 * Implementation note: follows the function signature of
 * #uPortPppDisconnectCallback_t.
 *
 * @param cellHandle            the handle of the cellular instance.
 * @param pppTerminateRequired  set this to true if the PPP connection
 *                              should be terminated first or leave
 *                              as false if the PPP connection
 *                              has already been terminated by
 *                              the peer; be sure to get this right
 *                              for the SARA-R5 case.
 * @return                      zero on success, else negative error
 *                              code.
 */
int32_t uCellPppClose(uDeviceHandle_t cellHandle,
                      bool pppTerminateRequired);

/** Transmit data over the PPP interface of the cellular module.
 * This may be integrated into a higher layer, e.g. the PPP
 * interface at the bottom of an IP stack, to permit it to send
 * PPP frames over a cellular transport.  uCellPppOpen() must have
 * been called for transmission to succeed.
 *
 * Implementation note: follows the function signature of
 * #uPortPppTransmitCallback_t.
 *
 * @param cellHandle  the handle of the cellular instance.
 * @param[in] pData   a pointer to the data to transmit; can only
 *                    be NULL if dataSize is zero.
 * @param dataSize    the number of bytes of data at pData.
 * @return            on success the number bytes transmitted,
 *                    which may be less than dataSize, else
 *                    negative error code.
 */
int32_t uCellPppTransmit(uDeviceHandle_t cellHandle,
                         const char *pData, size_t dataSize);

/** uCellPppClose() does not free memory in order to ensure
 * thread-safety; memory is only free'ed when the cellular instance
 * is closed.  However, if you can't wait, you really need that
 * memory back, and you are absolutely sure that there is no chance
 * of an asynchronous receive occurring, you may call this function
 * to regain heap.  Note that this only does the memory-freeing part,
 * not the closing down part, i.e. you must have called
 * uCellPppClose() and, to really ensure thread-safety, also called
 * uCellNetDisconnect() or uCellNetDeactivate(), for it to have
 * any effect.
 *
 * @param cellHandle  the handle of the cellular instance.
 */
void uCellPppFree(uDeviceHandle_t cellHandle);

#ifdef __cplusplus
}
#endif

#endif // _U_CELL_PPP_SHARED_H_

// End of file
