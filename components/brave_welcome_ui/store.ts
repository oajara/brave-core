/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

import { createStore, applyMiddleware } from 'redux'
import thunk from 'redux-thunk'

// Utils
import reducers from './reducers'

export default createStore(
    reducers,
    applyMiddleware(thunk)
)
