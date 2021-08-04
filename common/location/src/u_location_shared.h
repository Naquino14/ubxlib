/*
 * Copyright 2020 u-blox Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _U_LOCATION_SHARED_H_
#define _U_LOCATION_SHARED_H_

/* No #includes allowed here */

/** @file
 * @brief This header file defines functions that do not form part,
 * of the location API but are shared internally for use with the
 * network API, e.g. so that it can clear-up asynchronous location
 * requests when a network is taken down.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------
 * COMPILE-TIME MACROS
 * -------------------------------------------------------------- */

/* ----------------------------------------------------------------
 * TYPES
 * -------------------------------------------------------------- */

/** FIFO entry to keep track of asynchronous location requests.
 */
typedef struct uLocationSharedFifoEntry_t {
    int32_t networkHandle;
    void (*pCallback) (int32_t networkHandle,
                       int32_t errorCode,
                       const uLocation_t *pLocation);
    struct uLocationSharedFifoEntry_t *pNext;
} uLocationSharedFifoEntry_t;

/* ----------------------------------------------------------------
 * SHARED VARIABLES
 * -------------------------------------------------------------- */

/** Mutex to protect the FIFO.
 */
extern uPortMutexHandle_t gULocationMutex;

/* ----------------------------------------------------------------
 * FUNCTIONS
 * -------------------------------------------------------------- */

/** Initialise the internally shared location API: should
 * be called by the network API when the it initialises itself.
 * gULocationMutex should NOT be locked before this
 * is called (since this creates the mutex).
 *
 * @return  zero on success or negative error code.
 */
int32_t uLocationSharedInit();

/** De-initialise the internally shared location API: should
 * be called by the network API when the it de-initialises itself.
 * gULocationMutex should NOT be locked before this
 * is called (since this deletes the mutex).
 */
void uLocationSharedDeinit();

/** Add a new location request to the FIFO.
 * IMPORTANT: gULocationMutex should be locked before this
 * is called.
 *
 * @param networkHandle the handle of the network making the request.
 * @param type          the request type.
 * @param pCallback     the callback associated with the request.
 * @return              zero on success or negative error code.
 */
int32_t uLocationSharedRequestPush(int32_t networkHandle,
                                   uLocationType_t type,
                                   void (*pCallback) (int32_t networkHandle,
                                                      int32_t errorCode,
                                                      const uLocation_t *pLocation));

/** Pop the oldest location request of the given type from the FIFO.
 * IMPORTANT: gULocationMutex should be locked before this
 * is called.
 *
 * @param type the request type.
 * @return     the entry pointer: it is removed from the list and
 *             hence it is up to the calling task to free the pointer
 *             when done; NULL is returned if the FIFO is empty.
 */
uLocationSharedFifoEntry_t *pULocationSharedRequestPop(uLocationType_t type);

#ifdef __cplusplus
}
#endif

#endif // _U_LOCATION_SHARED_H_

// End of file
